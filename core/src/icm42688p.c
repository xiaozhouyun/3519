#include "icm42688p.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* =========================================================================
 * ICM-42688-P 寄存器宏定义 (Bank 0)
 * ========================================================================= */
#define ICM42688P_REG_ACCEL_DATA_X1 (0x1FU)  /* 加速度计 X轴高字节数据寄存器地址 */
#define ICM42688P_REG_GYRO_DATA_X1  (0x25U)  /* 陀螺仪 X轴高字节数据寄存器地址 */
#define ICM42688P_REG_WHO_AM_I      (0x75U)  /* 设备 ID 寄存器地址 (应返回 0x44) */
#define ICM42688P_REG_PWR_MGMT0     (0x4EU)  /* 电源管理寄存器 0 (模式切换) */
#define ICM42688P_REG_GYRO_CONFIG0  (0x4FU)  /* 陀螺仪配置寄存器 0 (量程与 ODR) */
#define ICM42688P_REG_ACCEL_CONFIG0 (0x50U)  /* 加速度计配置寄存器 0 (量程与 ODR) */

/* =========================================================================
 * SPI 读写标志与传感器参数宏定义
 * ========================================================================= */
#define ICM42688P_SPI_READ        (0x80U)    /* SPI 读操作最高有效位置 1 */
#define ICM42688P_TIMEOUT         (100000U)  /* SPI 读写超时等待循环计数 */
#define ICM42688P_ACC_LSB_PER_G   (2048.0f)  /* 加速度计灵敏度: ±16g 量程下为 2048 LSB/g */
#define ICM42688P_GYRO_LSB_PER_DPS (16.4f)   /* 陀螺仪灵敏度: ±2000 dps 量程下为 16.4 LSB/(°/s) */
#define ICM42688P_MAHONY_KP       (1.0f)     /* Mahony 互补滤波比例增益 KP */

/* 保持与 IMU660RC 相同的 SPI 实例及 CS 引脚宏控制 */
#define ICM42688P_CS_LOW()  DL_GPIO_clearPins(imuInt_PORT, imuInt_CS_PIN)  /* 片选拉低: 选中芯片 */
#define ICM42688P_CS_HIGH() DL_GPIO_setPins(imuInt_PORT, imuInt_CS_PIN)   /* 片选拉高: 释放芯片 */

/* =========================================================================
 * 全局变量定义
 * ========================================================================= */
int16_t icm42688p_acc_x;   /* 加速度计 X 轴原始数据 (ADC Counts) */
int16_t icm42688p_acc_y;   /* 加速度计 Y 轴原始数据 (ADC Counts) */
int16_t icm42688p_acc_z;   /* 加速度计 Z 轴原始数据 (ADC Counts) */
int16_t icm42688p_gyro_x;  /* 陀螺仪 X 轴原始数据 (ADC Counts) */
int16_t icm42688p_gyro_y;  /* 陀螺仪 Y 轴原始数据 (ADC Counts) */
int16_t icm42688p_gyro_z;  /* 陀螺仪 Z 轴原始数据 (ADC Counts) */

float icm42688p_roll;   /* 横滚角 Roll (单位: 度 °) */
float icm42688p_pitch;  /* 俯仰角 Pitch (单位: 度 °) */
float icm42688p_yaw;    /* 偏航角 Yaw (单位: 度 °) */

/* 姿态解算四元数 (初始状态 q0=1 表示无旋转) */
static float q0 = 1.0f;
static float q1;
static float q2;
static float q3;

/* =========================================================================
 * 底层 SPI 通信与私有辅助函数
 * ========================================================================= */

/**
 * @brief 等待 SPI 发送 FIFO 空闲且总线不忙
 * @return 0: 正常完成; 1: 超时失败
 */
static uint8_t icm42688p_spi_wait_idle(void)
{
    uint32_t timeout = ICM42688P_TIMEOUT;

    while ((!DL_SPI_isTXFIFOEmpty(IMU660RC_INST) || DL_SPI_isBusy(IMU660RC_INST)) && --timeout) {
    }
    return (timeout != 0U) ? 0U : 1U;
}

/**
 * @brief 清空 SPI 接收 FIFO 中的无用数据
 */
static void icm42688p_spi_flush_rx(void)
{
    while (!DL_SPI_isRXFIFOEmpty(IMU660RC_INST)) {
        (void)DL_SPI_receiveData8(IMU660RC_INST);
    }
}

/**
 * @brief 向 ICM42688P 指定寄存器写入单字节数据
 * @param reg 目标寄存器地址
 * @param value 要写入的单字节数据
 * @return 0: 成功; 1: 失败/超时
 */
static uint8_t icm42688p_write_register(uint8_t reg, uint8_t value)
{
    uint32_t timeout = ICM42688P_TIMEOUT;

    ICM42688P_CS_LOW();
    icm42688p_spi_flush_rx();

    /* 等待 TX FIFO 有空闲空间 */
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
    }
    if (timeout == 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }

    /* 发送寄存器地址 */
    DL_SPI_transmitData8(IMU660RC_INST, reg);
    timeout = ICM42688P_TIMEOUT;
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
    }
    if (timeout == 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }

    /* 发送要写入的数据 */
    DL_SPI_transmitData8(IMU660RC_INST, value);

    /* 等待传输完成 */
    if (icm42688p_spi_wait_idle() != 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }

    icm42688p_spi_flush_rx();
    ICM42688P_CS_HIGH();
    return 0U;
}

/**
 * @brief 从 ICM42688P 连续读取多个寄存器的字节数据
 * @param reg 起始寄存器地址
 * @param data 读取到的数据缓冲区指针
 * @param len 要读取的字节数
 * @return 0: 成功; 1: 失败/超时
 */
static uint8_t icm42688p_read_registers(uint8_t reg, uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint32_t timeout = ICM42688P_TIMEOUT;

    ICM42688P_CS_LOW();
    icm42688p_spi_flush_rx();

    /* 发送带有读标志 (0x80) 的寄存器地址 */
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
    }
    if (timeout == 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }
    DL_SPI_transmitData8(IMU660RC_INST, reg | ICM42688P_SPI_READ);

    /* 发送哑数据 (0x00) 以驱动 SPI 时钟产生接收数据 */
    for (i = 0U; i < len; i++) {
        timeout = ICM42688P_TIMEOUT;
        while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
        }
        if (timeout == 0U) {
            ICM42688P_CS_HIGH();
            return 1U;
        }
        DL_SPI_transmitData8(IMU660RC_INST, 0x00U);
    }

    /* 等待传输结束 */
    if (icm42688p_spi_wait_idle() != 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }

    /* 丢弃第一个由发送地址时产生的无用接收字节 */
    (void)DL_SPI_receiveData8(IMU660RC_INST);

    /* 依次从 RX FIFO 中读取有效字节 */
    for (i = 0U; i < len; i++) {
        if (DL_SPI_isRXFIFOEmpty(IMU660RC_INST)) {
            ICM42688P_CS_HIGH();
            return 1U;
        }
        data[i] = (uint8_t)DL_SPI_receiveData8(IMU660RC_INST);
    }

    ICM42688P_CS_HIGH();
    return 0U;
}

/* =========================================================================
 * 传感器驱动公有接口
 * ========================================================================= */

/**
 * @brief 读取 ICM42688P 的 WHO_AM_I 设备识别码
 * @return 设备的 WHO_AM_I 寄存器值 (正常应为 0x44)
 */
uint8_t icm42688p_read_id(void)
{
    uint8_t id = 0U;

    (void)icm42688p_read_registers(ICM42688P_REG_WHO_AM_I, &id, 1U);
    return id;
}

/**
 * @brief 初始化 ICM42688P 传感器
 * @details 检查 WHO_AM_I，配置加速度计与陀螺仪的采样率和量程，并开启低噪声模式电源
 * @return 0: 初始化成功; 1: 初始化失败 (ID 不匹配或写寄存器失败)
 */
uint8_t icm42688p_init(void)
{
    ICM42688P_CS_HIGH();
    delay_cycles(CPUCLK_FREQ / 1000U * 2U);  /* 延时等待芯片上电稳定 */

    /* 验证设备 ID */
    if (icm42688p_read_id() != ICM42688P_WHO_AM_I_VALUE) {
        return 1U;
    }

    /* 配置过程:
     * 1. PWR_MGMT0 (0x4E) = 0x00: 先关闭传感器进入待机模式
     * 2. GYRO_CONFIG0 (0x4F) = 0x06: 陀螺仪量程 ±2000 dps, ODR = 1kHz
     * 3. ACCEL_CONFIG0 (0x50) = 0x06: 加速度计量程 ±16g, ODR = 1kHz
     * 4. PWR_MGMT0 (0x4E) = 0x0F: 开启加速度计与陀螺仪低噪声 (LN) 模式
     */
    if (icm42688p_write_register(ICM42688P_REG_PWR_MGMT0, 0x00U) != 0U ||
        icm42688p_write_register(ICM42688P_REG_GYRO_CONFIG0, 0x06U) != 0U ||
        icm42688p_write_register(ICM42688P_REG_ACCEL_CONFIG0, 0x06U) != 0U ||
        icm42688p_write_register(ICM42688P_REG_PWR_MGMT0, 0x0FU) != 0U) {
        return 1U;
    }
    delay_cycles(CPUCLK_FREQ / 20U);  /* 开启后延时等待传感器输出稳定 */
    return 0U;
}

/* =========================================================================
 * 姿态解算算法 (Mahony 互补滤波)
 * ========================================================================= */

/**
 * @brief Mahony 互补滤波姿态更新函数
 * @param gx 陀螺仪 X 轴角速度 (单位: rad/s)
 * @param gy 陀螺仪 Y 轴角速度 (单位: rad/s)
 * @param gz 陀螺仪 Z 轴角速度 (单位: rad/s)
 * @param ax 加速度计 X 轴重力加速度分量
 * @param ay 加速度计 Y 轴重力加速度分量
 * @param az 加速度计 Z 轴重力加速度分量
 * @param dt 采样更新周期 (单位: s)
 */
static void icm42688p_mahony_update(float gx, float gy, float gz, float ax, float ay, float az, float dt)
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

    /* 对加速度向量进行归一化处理 */
    norm = fast_sqrtf(ax * ax + ay * ay + az * az);
    if (norm > 0.0001f) {
        ax /= norm;
        ay /= norm;
        az /= norm;

        /* 计算根据当前估计姿态(四元数)预测出的重力向量方向 (0.5 * V) */
        half_vx = q1 * q3 - q0 * q2;
        half_vy = q0 * q1 + q2 * q3;
        half_vz = q0 * q0 - 0.5f + q3 * q3;

        /* 测量重力向量与预测重力向量的叉积，得到姿态误差向量 (0.5 * Error) */
        half_ex = ay * half_vz - az * half_vy;
        half_ey = az * half_vx - ax * half_vz;
        half_ez = ax * half_vy - ay * half_vx;

        /* 使用比例控制 (KP) 补偿陀螺仪角速度测量偏差 */
        gx += ICM42688P_MAHONY_KP * half_ex;
        gy += ICM42688P_MAHONY_KP * half_ey;
        gz += ICM42688P_MAHONY_KP * half_ez;
    }

    /* 一阶欧拉法通过微分方程更新四元数 */
    half_q0 = 0.5f * q0;
    half_q1 = 0.5f * q1;
    half_q2 = 0.5f * q2;
    half_q3 = 0.5f * q3;
    q0 += (-half_q1 * gx - half_q2 * gy - half_q3 * gz) * dt;
    q1 += (half_q0 * gx + half_q2 * gz - half_q3 * gy) * dt;
    q2 += (half_q0 * gy - half_q1 * gz + half_q3 * gx) * dt;
    q3 += (half_q0 * gz + half_q1 * gy - half_q2 * gx) * dt;

    /* 重新归一化四元数，保持单位四元数特性 */
    norm = fast_sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    if (norm > 0.0001f) {
        q0 /= norm;
        q1 /= norm;
        q2 /= norm;
        q3 /= norm;
    }
}

/**
 * @brief 读取传感器数据并完成姿态解算的周期更新函数
 * @param dt 距上次更新的间隔时间 (单位: s)
 */
void icm42688p_update(float dt)
{
    uint8_t data[12];
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;

    /* 参数有效性校验与 12 字节连续读取 (加速度 6 字节 + 陀螺仪 6 字节) */
    if (dt <= 0.0f || icm42688p_read_registers(ICM42688P_REG_ACCEL_DATA_X1, data, sizeof(data)) != 0U) {
        return;
    }

    /* 拼合高低字节，提取三轴加速度和三轴陀螺仪原始 ADC 计数值 */
    icm42688p_acc_x = (int16_t)(((uint16_t)data[0] << 8) | data[1]);
    icm42688p_acc_y = (int16_t)(((uint16_t)data[2] << 8) | data[3]);
    icm42688p_acc_z = (int16_t)(((uint16_t)data[4] << 8) | data[5]);
    icm42688p_gyro_x = (int16_t)(((uint16_t)data[6] << 8) | data[7]);
    icm42688p_gyro_y = (int16_t)(((uint16_t)data[8] << 8) | data[9]);
    icm42688p_gyro_z = (int16_t)(((uint16_t)data[10] << 8) | data[11]);

    /* 将原始数据转换为物理单位：
     * - 加速度: g
     * - 陀螺仪: rad/s (弧度每秒)
     */
    ax = (float)icm42688p_acc_x / ICM42688P_ACC_LSB_PER_G;
    ay = (float)icm42688p_acc_y / ICM42688P_ACC_LSB_PER_G;
    az = (float)icm42688p_acc_z / ICM42688P_ACC_LSB_PER_G;
    gx = (float)icm42688p_gyro_x / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gy = (float)icm42688p_gyro_y / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gz = (float)icm42688p_gyro_z / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);

    /* 运行 Mahony 互补滤波计算姿态四元数 */
    icm42688p_mahony_update(gx, gy, gz, ax, ay, az, dt);

    /* 将四元数转化为欧拉角 (角度制 °) */
    /* 1. 横滚角 Roll (绕 X 轴旋转) */
    icm42688p_roll = fast_atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * (180.0f / M_PI);

    /* 2. 俯仰角 Pitch (绕 Y 轴旋转，限幅以防 atan2/asin 溢出) */
    {
        float sin_pitch = 2.0f * (q0 * q2 - q3 * q1);

        if (sin_pitch > 1.0f) {
            sin_pitch = 1.0f;
        } else if (sin_pitch < -1.0f) {
            sin_pitch = -1.0f;
        }
        icm42688p_pitch = fast_atan2f(sin_pitch, fast_sqrtf(1.0f - sin_pitch * sin_pitch)) * (180.0f / M_PI);
    }

    /* 3. 偏航角 Yaw (绕 Z 轴旋转) */
    icm42688p_yaw = fast_atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * (180.0f / M_PI);
}
