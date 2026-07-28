/*********************************************************************************************************************
 * MSPM0G3519 ICM45686 六轴传感器驱动与姿态解算模块实现文件
 * 
 * 文件          icm45686.c
 * 平台          MSPM0G3519 (ti_msp_dl_config.h)
 * 说明          本文件包含 ICM45686 六轴 IMU 的 SPI 通信读写、初始化自检、
 *               Mahony 姿态融合滤波算法、四元数求解及 Roll/Pitch/Yaw 姿态欧拉角更新。
 ********************************************************************************************************************/

#include "icm45686.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ===================================================================================================================
 * ICM45686 寄存器映射与算法参数定义
 * =================================================================================================================== */
#define ICM45686_REG_ACCEL_DATA_X1_UI   (0x00U)  /**< 加速度计 X 轴数据高字节起始寄存器 (0x00 ~ 0x0B 连续 12 字节装载六轴数据) */
#define ICM45686_REG_PWR_MGMT0           (0x10U)  /**< 电源管理寄存器 0 (使能加速度计和陀螺仪工作模式) */
#define ICM45686_REG_ACCEL_CONFIG0       (0x1BU)  /**< 加速度计配置寄存器 0 (设置满量程 Range 与输出数据率 ODR) */
#define ICM45686_REG_GYRO_CONFIG0        (0x1CU)  /**< 陀螺仪配置寄存器 0 (设置满量程 Range 与输出数据率 ODR) */
#define ICM45686_REG_WHO_AM_I            (0x72U)  /**< 芯片标识 WHO_AM_I 寄存器地址 */
#define ICM45686_SPI_READ                (0x80U)  /**< SPI 读操作掩码位 (Highest Bit = 1 代表读) */
#define ICM45686_TIMEOUT                 (100000U)/**< SPI 超时等待循环计数阈值 */
#define ICM45686_ACC_LSB_PER_G           (4096.0f)/**< 加速度计灵敏度比例因子 (根据量程设置，LSB/g) */
#define ICM45686_GYRO_LSB_PER_DPS        (16.4f)  /**< 陀螺仪灵敏度比例因子 (根据量程设置，LSB/(deg/s)) */
#define ICM45686_MAHONY_KP               (1.0f)   /**< Mahony 姿态互补滤波比例增益系数 Kp */

/* ===================================================================================================================
 * 硬件片选 (CS) 引脚控制宏
 * =================================================================================================================== */
#define ICM45686_CS_LOW()  DL_GPIO_clearPins(imuInt_PORT, imuInt_CS_PIN)  /**< 拉低 CS 引脚，开启 SPI 通信 */
#define ICM45686_CS_HIGH() DL_GPIO_setPins(imuInt_PORT, imuInt_CS_PIN)   /**< 拉高 CS 引脚，结束 SPI 通信 */

/* ===================================================================================================================
 * 全局姿态与原始数据变量定义
 * =================================================================================================================== */
int16_t icm45686_acc_x;  /**< X 轴加速度计原始测量值 */
int16_t icm45686_acc_y;  /**< Y 轴加速度计原始测量值 */
int16_t icm45686_acc_z;  /**< Z 轴加速度计原始测量值 */
int16_t icm45686_gyro_x; /**< X 轴陀螺仪原始测量值 */
int16_t icm45686_gyro_y; /**< Y 轴陀螺仪原始测量值 */
int16_t icm45686_gyro_z; /**< Z 轴陀螺仪原始测量值 */
float icm45686_roll;     /**< 横滚角 Roll (单位: 度 deg) */
float icm45686_pitch;    /**< 俯仰角 Pitch (单位: 度 deg) */
float icm45686_yaw;      /**< 偏航角 Yaw (单位: 度 deg) */

/* 姿态融合单位四元数 [q0, q1, q2, q3] */
static float q0 = 1.0f;
static float q1;
static float q2;
static float q3;

/* ===================================================================================================================
 * 底层 SPI 通信辅助实现函数
 * =================================================================================================================== */

/**
 * @brief  等待 SPI 总线发送 FIFO 清空且总线进入空闲状态
 * @return uint8_t 0: 等待成功(总线空闲), 1: 等待超时
 */
static uint8_t icm45686_spi_wait_idle(void)
{
    uint32_t timeout = ICM45686_TIMEOUT;

    while ((!DL_SPI_isTXFIFOEmpty(IMU660RC_INST) || DL_SPI_isBusy(IMU660RC_INST)) && --timeout)
    {
    }
    return (timeout != 0U) ? 0U : 1U;
}

/**
 * @brief  清空 SPI 接收 FIFO 缓冲区中残留的数据字节
 */
static void icm45686_spi_flush_rx(void)
{
    while (!DL_SPI_isRXFIFOEmpty(IMU660RC_INST))
    {
        (void)DL_SPI_receiveData8(IMU660RC_INST);
    }
}

/**
 * @brief  向 ICM45686 写入单个 8 位寄存器
 * @param  reg    目标寄存器地址
 * @param  value  待写入的数据字节
 * @return uint8_t 0: 写入成功, 1: 写入失败 (超时)
 */
static uint8_t icm45686_write_register(uint8_t reg, uint8_t value)
{
    uint32_t timeout = ICM45686_TIMEOUT;

    ICM45686_CS_LOW();
    icm45686_spi_flush_rx();
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout)
    {
    }
    if (timeout == 0U)
    {
        ICM45686_CS_HIGH();
        return 1U;
    }
    DL_SPI_transmitData8(IMU660RC_INST, reg);
    timeout = ICM45686_TIMEOUT;
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout)
    {
    }
    if (timeout == 0U)
    {
        ICM45686_CS_HIGH();
        return 1U;
    }
    DL_SPI_transmitData8(IMU660RC_INST, value);
    if (icm45686_spi_wait_idle() != 0U)
    {
        ICM45686_CS_HIGH();
        return 1U;
    }
    icm45686_spi_flush_rx();
    ICM45686_CS_HIGH();
    return 0U;
}

/**
 * @brief  连续从 ICM45686 读取多个寄存器字节数据
 * @param  reg   起始寄存器地址
 * @param  data  接收数据缓冲区指针
 * @param  len   读取的连续字节长度
 * @return uint8_t 0: 读取成功, 1: 读取失败 (超时或 FIFO 异常)
 */
static uint8_t icm45686_read_registers(uint8_t reg, uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint32_t timeout = ICM45686_TIMEOUT;

    ICM45686_CS_LOW();
    icm45686_spi_flush_rx();
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout)
    {
    }
    if (timeout == 0U)
    {
        ICM45686_CS_HIGH();
        return 1U;
    }
    DL_SPI_transmitData8(IMU660RC_INST, reg | ICM45686_SPI_READ);
    for (i = 0U; i < len; i++)
    {
        timeout = ICM45686_TIMEOUT;
        while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout)
        {
        }
        if (timeout == 0U)
        {
            ICM45686_CS_HIGH();
            return 1U;
        }
        DL_SPI_transmitData8(IMU660RC_INST, 0x00U);
    }
    if (icm45686_spi_wait_idle() != 0U)
    {
        ICM45686_CS_HIGH();
        return 1U;
    }
    (void)DL_SPI_receiveData8(IMU660RC_INST); // 抛弃读地址发阶段收到的空字节
    for (i = 0U; i < len; i++)
    {
        if (DL_SPI_isRXFIFOEmpty(IMU660RC_INST))
        {
            ICM45686_CS_HIGH();
            return 1U;
        }
        data[i] = (uint8_t)DL_SPI_receiveData8(IMU660RC_INST);
    }
    ICM45686_CS_HIGH();
    return 0U;
}

/* ===================================================================================================================
 * 设备自检与初始化接口
 * =================================================================================================================== */

/**
 * @brief  读取 ICM45686 芯片设备 ID 字节
 * @return uint8_t 设备 ID (正常应为 0xE9)
 */
uint8_t icm45686_read_id(void)
{
    uint8_t id = 0U;

    (void)icm45686_read_registers(ICM45686_REG_WHO_AM_I, &id, 1U);
    return id;
}

/**
 * @brief  初始化 ICM45686 六轴 IMU
 * @return uint8_t 0: 初始化成功, 1: 初始化失败 (ID 不匹配或配置写失败)
 */
uint8_t icm45686_init(void)
{
    ICM45686_CS_HIGH();
    delay_cycles(CPUCLK_FREQ / 1000U * 2U);
    if (icm45686_read_id() != ICM45686_WHO_AM_I_VALUE)
    {
        return 1U;
    }
    // 配置加速度计、陀螺仪数据刷新率/量程及电源模式
    if (icm45686_write_register(ICM45686_REG_ACCEL_CONFIG0, 0x29U) != 0U ||
        icm45686_write_register(ICM45686_REG_GYRO_CONFIG0, 0x19U) != 0U ||
        icm45686_write_register(ICM45686_REG_PWR_MGMT0, 0x0FU) != 0U)
    {
        return 1U;
    }
    delay_cycles(CPUCLK_FREQ / 1000U * 2U);
    return 0U;
}

/* ===================================================================================================================
 * 姿态解算算法与周期更新
 * =================================================================================================================== */

/**
 * @brief  Mahony 姿态互补滤波核心更新算法
 * @param  gx  X 轴角速度 (单位: rad/s)
 * @param  gy  Y 轴角速度 (单位: rad/s)
 * @param  gz  Z 轴角速度 (单位: rad/s)
 * @param  ax  X 轴加速度测量值
 * @param  ay  Y 轴加速度测量值
 * @param  az  Z 轴加速度测量值
 * @param  dt  算法更新周期 (单位: s)
 */
static void icm45686_mahony_update(float gx, float gy, float gz, float ax, float ay, float az, float dt)
{
    float norm;
    float half_vx;
    float half_vy;
    float half_vz;
    float half_ex;
    float half_ey;
    float half_ez;
    float half_q0;
    float half_q1;
    float half_q2;
    float half_q3;

    // 1. 加速度计三轴矢量归一化
    norm = fast_sqrtf(ax * ax + ay * ay + az * az);
    if (norm > 0.0001f)
    {
        ax /= norm;
        ay /= norm;
        az /= norm;

        // 2. 根据四元数计算推导出的重力方向估计矢量半量
        half_vx = q1 * q3 - q0 * q2;
        half_vy = q0 * q1 + q2 * q3;
        half_vz = q0 * q0 - 0.5f + q3 * q3;

        // 3. 测量重力矢量与估算重力矢量的叉积计算方向偏差半量
        half_ex = ay * half_vz - az * half_vy;
        half_ey = az * half_vx - ax * half_vz;
        half_ez = ax * half_vy - ay * half_vx;

        // 4. 利用比例增益 Kp 补偿陀螺仪角速度测量零偏
        gx += ICM45686_MAHONY_KP * half_ex;
        gy += ICM45686_MAHONY_KP * half_ey;
        gz += ICM45686_MAHONY_KP * half_ez;
    }

    // 5. 四元数微分方程一阶解算
    half_q0 = 0.5f * q0;
    half_q1 = 0.5f * q1;
    half_q2 = 0.5f * q2;
    half_q3 = 0.5f * q3;
    q0 += (-half_q1 * gx - half_q2 * gy - half_q3 * gz) * dt;
    q1 += (half_q0 * gx + half_q2 * gz - half_q3 * gy) * dt;
    q2 += (half_q0 * gy - half_q1 * gz + half_q3 * gx) * dt;
    q3 += (half_q0 * gz + half_q1 * gy - half_q2 * gx) * dt;

    // 6. 姿态四元数归一化
    norm = fast_sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    if (norm > 0.0001f)
    {
        q0 /= norm;
        q1 /= norm;
        q2 /= norm;
        q3 /= norm;
    }
}

/**
 * @brief  读取传感器数据并更新欧拉角 (Roll, Pitch, Yaw)
 * @param  dt 更新计算周期时间 (单位: 秒 s)
 */
void icm45686_update(float dt)
{
    uint8_t data[12];
    float gx;
    float gy;
    float gz;
    float ax;
    float ay;
    float az;

    // 周期有效性检查及 12 字节原始数据批量读取
    if (dt <= 0.0f || icm45686_read_registers(ICM45686_REG_ACCEL_DATA_X1_UI, data, sizeof(data)) != 0U)
    {
        return;
    }

    // 解析两字节 16 位有符号加速度计与陀螺仪原始数值
    icm45686_acc_x = (int16_t)(((uint16_t)data[0] << 8) | data[1]);
    icm45686_acc_y = (int16_t)(((uint16_t)data[2] << 8) | data[3]);
    icm45686_acc_z = (int16_t)(((uint16_t)data[4] << 8) | data[5]);
    icm45686_gyro_x = (int16_t)(((uint16_t)data[6] << 8) | data[7]);
    icm45686_gyro_y = (int16_t)(((uint16_t)data[8] << 8) | data[9]);
    icm45686_gyro_z = (int16_t)(((uint16_t)data[10] << 8) | data[11]);

    // 转换为标准单位 (g 与 rad/s)
    ax = (float)icm45686_acc_x / ICM45686_ACC_LSB_PER_G;
    ay = (float)icm45686_acc_y / ICM45686_ACC_LSB_PER_G;
    az = (float)icm45686_acc_z / ICM45686_ACC_LSB_PER_G;
    gx = (float)icm45686_gyro_x / ICM45686_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gy = (float)icm45686_gyro_y / ICM45686_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gz = (float)icm45686_gyro_z / ICM45686_GYRO_LSB_PER_DPS * (M_PI / 180.0f);

    // 调用 Mahony 滤波更新四元数
    icm45686_mahony_update(gx, gy, gz, ax, ay, az, dt);

    // 四元数转 Roll, Pitch, Yaw 姿态欧拉角 (转换为角度制 deg)
    icm45686_roll = fast_atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * (180.0f / M_PI);
    icm45686_pitch = fast_asinf(2.0f * (q0 * q2 - q3 * q1)) * (180.0f / M_PI);
    icm45686_yaw = fast_atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * (180.0f / M_PI);
}
