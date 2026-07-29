/*********************************************************************************************************************
* MSPM0G3519 IMU660RC 六轴传感器驱动 (TI Official DriverLib 移植版)
* 
* 文件          zf_device_imu660rc.c
* 平台          MSPM0G3519 (ti_msp_dl_config.h)
* 说明          本文件包含 IMU660RC 寄存器读写、SPI 通讯同步、FP16 解压算法、
*               四元数与欧拉角转换、PB24(INT2) 引脚中断服务函数及初始化流程。
********************************************************************************************************************/

#include "../inc/zf_device_imu660rc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// 毫秒级与微秒级精准延时 (基于 DriverLib 系统时钟 CPUCLK_FREQ)
#define delay_ms(ms)            delay_cycles(CPUCLK_FREQ / 1000 * (ms))
#define system_delay_ms(ms)     delay_ms(ms)

// 软件 GPIO 片选引脚 (使用头文件中定义的 IMU660RC_CS_PORT / IMU660RC_CS_PIN) 控制宏
#define IMU660RC_CS(x)          ((x) ? DL_GPIO_setPins(IMU660RC_CS_PORT, IMU660RC_CS_PIN) : DL_GPIO_clearPins(IMU660RC_CS_PORT, IMU660RC_CS_PIN))

static uint8 imu660rc_quarternion_rate;                                         // 姿态解算刷新率全局配置

float imu660rc_transition_factor[2];                                           // 转换系数 [0]:加速度, [1]:陀螺仪
int16 imu660rc_gyro_x = 0,  imu660rc_gyro_y = 0,    imu660rc_gyro_z = 0;    // 陀螺仪原始数据
int16 imu660rc_acc_x  = 0,  imu660rc_acc_y  = 0,    imu660rc_acc_z  = 0;    // 加速度计原始数据
float imu660rc_roll   = 0,  imu660rc_pitch  = 0,    imu660rc_yaw    = 0;    // 解算欧拉角 (横滚角 Roll, 俯仰角 Pitch, 偏航角 Yaw)
float imu660rc_quarternion[4];                                              // 姿态融合四元数

// ===================================================================================================================
// 底层 SPI 接口与同步等待函数 (适配 MSPM0 DriverLib SPI1 外设)
// ===================================================================================================================

/**
 * @brief  等待 SPI1 总线发送完毕且总线进入空闲状态 (防数据碰撞)
 */
static inline void imu660rc_spi_wait_idle(void)
{
    uint32_t timeout = 100000U;
    while ((!DL_SPI_isTXFIFOEmpty(IMU660RC_INST) || DL_SPI_isBusy(IMU660RC_INST)) && --timeout);
}

/**
 * @brief  清空 SPI1 接收 FIFO 缓冲区 (防旧残留数据干扰)
 */
static inline void imu660rc_spi_flush_rx(void)
{
    while (!DL_SPI_isRXFIFOEmpty(IMU660RC_INST))
    {
        (void)DL_SPI_receiveData8(IMU660RC_INST);
    }
}

/**
 * @brief  写单个 8 位 IMU660RC 寄存器
 * @param  reg   寄存器目标地址
 * @param  data  待写入的数据字节
 */
static void imu660rc_write_register(uint8 reg, uint8 data)
{
    IMU660RC_CS(0); // 拉低 CS 启动 SPI 传输
    delay_cycles(10);

    imu660rc_spi_flush_rx();

    while (DL_SPI_isTXFIFOFull(IMU660RC_INST));
    DL_SPI_transmitData8(IMU660RC_INST, reg | IMU660RC_SPI_W);

    while (DL_SPI_isTXFIFOFull(IMU660RC_INST));
    DL_SPI_transmitData8(IMU660RC_INST, data);

    imu660rc_spi_wait_idle();
    imu660rc_spi_flush_rx();

    delay_cycles(10);
    IMU660RC_CS(1); // 拉高 CS 完成 SPI 传输
}

/**
 * @brief  读单个 8 位 IMU660RC 寄存器
 * @param  reg   寄存器目标地址
 * @return uint8 读取到的寄存器字节内容
 */
static uint8 imu660rc_read_register(uint8 reg)
{
    uint8 val;
    IMU660RC_CS(0); // 拉低 CS 启动 SPI 传输
    delay_cycles(10);

    imu660rc_spi_flush_rx();

    while (DL_SPI_isTXFIFOFull(IMU660RC_INST));
    DL_SPI_transmitData8(IMU660RC_INST, reg | IMU660RC_SPI_R);

    while (DL_SPI_isTXFIFOFull(IMU660RC_INST));
    DL_SPI_transmitData8(IMU660RC_INST, 0x00);

    imu660rc_spi_wait_idle();

    (void)DL_SPI_receiveData8(IMU660RC_INST); // 抛弃地址发送阶段返回的数据
    val = (uint8)DL_SPI_receiveData8(IMU660RC_INST); // 读取并返回数据阶段的数据

    delay_cycles(10);
    IMU660RC_CS(1); // 拉高 CS 完成 SPI 传输
    return val;
}

/**
 * @brief  连续读取多个 IMU660RC 寄存器
 * @param  reg   起始寄存器地址
 * @param  data  目标接收缓冲区指针
 * @param  len   连续读取字节数
 */
static void imu660rc_read_registers(uint8 reg, uint8 *data, uint32 len)
{
    uint32 i;
    IMU660RC_CS(0); // 拉低 CS 启动 SPI 传输
    delay_cycles(10);

    imu660rc_spi_flush_rx();

    while (DL_SPI_isTXFIFOFull(IMU660RC_INST));
    DL_SPI_transmitData8(IMU660RC_INST, reg | IMU660RC_SPI_R);

    for (i = 0; i < len; i++)
    {
        while (DL_SPI_isTXFIFOFull(IMU660RC_INST));
        DL_SPI_transmitData8(IMU660RC_INST, 0x00);
    }

    imu660rc_spi_wait_idle();

    (void)DL_SPI_receiveData8(IMU660RC_INST); // 抛弃地址发送阶段接收到的无效字节
    for (i = 0; i < len; i++)
    {
        data[i] = (uint8)DL_SPI_receiveData8(IMU660RC_INST);
    }

    delay_cycles(10);
    IMU660RC_CS(1); // 拉高 CS 完成 SPI 传输
}

// ===================================================================================================================
// 数学运算与姿态解算辅助函数 (FP16浮点解包、四元数归一化与欧拉角转换)
// ===================================================================================================================

/**
 * @brief  将半精度浮点数 (FP16) 转换为标准单精度 32 位 IEEE 754 浮点数位格式
 * @param  h  16位半精度浮点数据
 * @return uint32 转换后的32位浮点二进制位
 */
static uint32 fp16_to_float(uint16 h)
{
    uint16 h_exp = (h & 0x7c00u);
    uint32 f_sgn = ((uint32)h & 0x8000u) << 16;
    switch (h_exp)
    {
        case 0x0000u:   // 0 或非规格化数
        {
            uint16 h_sig = (h & 0x03ffu);
            if (h_sig == 0)
            {
                return f_sgn;
            }
            h_sig <<= 1;
            while ((h_sig & 0x0400u) == 0)
            {
                h_sig <<= 1;
                h_exp++;
            }
            uint32 f_exp = ((uint32)(127 - 15 - h_exp)) << 23;
            uint32 f_sig = ((uint32)(h_sig & 0x03ffu)) << 13;
            return f_sgn + f_exp + f_sig;
        }
        case 0x7c00u:   // 无穷大或 NaN
        {
            return f_sgn + 0x7f800000u + (((uint32)(h & 0x03ffu)) << 13);
        }
        default:        // 规格化浮点数
        {
            return f_sgn + (((uint32)(h & 0x7fffu) + 0x1c000u) << 13);
        }
    }
}

/**
 * @brief  四元数归一化处理
 * @param  quat  归一化后的四元数输出数组 [q0, q1, q2, q3]
 * @param  fp16  传感器 SRAM 读出的 4 个 16位 FP16 四元数原始数据
 */
static void quarternion_normalize(float quat[4], uint16 *fp16)
{
    float n = 0;
    float temp[4];

    *(uint32 *)(&temp[0]) = fp16_to_float(fp16[0]);
    *(uint32 *)(&temp[1]) = fp16_to_float(fp16[1]);
    *(uint32 *)(&temp[2]) = fp16_to_float(fp16[2]);
    *(uint32 *)(&temp[3]) = fp16_to_float(fp16[3]);

    n = temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2] + temp[3] * temp[3];
    n = sqrtf(n);

    if (n > 0.001f) // 防止除零溢出
    {
        n = temp[3] < 0.0f ? -n : n;

        quat[0] = temp[1] / n;
        quat[1] = temp[2] / n;
        quat[2] = temp[0] / n;
        quat[3] = temp[3] / n;
    }
}

/**
 * @brief  由单位四元数计算欧拉角 (Roll, Pitch, Yaw)
 * @param  quat   输入的归一化四元数
 * @param  roll   横滚角输出指针 (单位: 度 deg)
 * @param  pitch  俯仰角输出指针 (单位: 度 deg)
 * @param  yaw    偏航角输出指针 (单位: 度 deg, 范围: 0 ~ 360 度)
 */
static void quarternion_to_euler(float quat[4], float *roll, float *pitch, float *yaw)
{
    float euler[3];

    float sqx = quat[0] * quat[0];
    float sqy = quat[1] * quat[1];
    float sqz = quat[2] * quat[2];

    // 旋转矩阵反算弧度制姿态角
    euler[0] =  atan2f(2.0f * (quat[1] * quat[3] + quat[0] * quat[2]), 1.0f - 2.0f * (sqy + sqx));
    euler[1] = -asinf(2.0f * (quat[0] * quat[3] - quat[1] * quat[2]));
    euler[2] =  atan2f(2.0f * (quat[0] * quat[1] + quat[2] * quat[3]), 1.0f - 2.0f * (sqx + sqz));

    // 弧度转化为角度
    euler[0] = 180.0f * (euler[0]) / M_PI;
    euler[1] = 180.0f * (euler[1]) / M_PI;
    euler[2] = 180.0f * (euler[2]) / M_PI;

    // 偏航角归一化至 0 ~ 360 度
    euler[2] = 0.0f > euler[2] ? euler[2] + 360.0f : euler[2];

    *roll   = euler[0];
    *pitch  = euler[1];
    *yaw    = euler[2];
}

/**
 * @brief  切换 IMU660RC 的 Memory Bank 内存页面
 * @param  bank 目标 Bank
 */
static void imu660rc_set_mem_bank(imu660rc_mem_bank_enum bank)
{
    imu660rc_write_register(IMU660RC_FUNC_CFG_ACCESS, bank);
}

/**
 * @brief  IMU660RC 通讯自检
 * @return uint8 0-自检通过, 1-自检失败
 */
static uint8 imu660rc_self_check(void)
{
    uint8 dat = 0, return_state = 0;
    uint16 timeout_count = 0;
    do
    {
        if (IMU660RC_TIMEOUT_COUNT < timeout_count++)
        {
            return_state = 1;
            break;
        }
        dat = imu660rc_read_register(IMU660RC_CHIP_ID);
        system_delay_ms(1);
    } while (0x70 != dat && 0x6C != dat && 0x6B != dat); // 支持 0x70(LSM6DSO16IS), 0x6C(LSM6DSO), 0x6B(ISM330)
    return return_state;
}

// ===================================================================================================================
// 应用层数据获取接口
// ===================================================================================================================

/**
 * @brief  读取加速度计 3 轴原始数据 (仅在未使能四元数融合时生效)
 */
void imu660rc_get_acc(void)
{
    int16 dat[3];
    if (IMU660RC_QUARTERNION_DISABLE == imu660rc_quarternion_rate)
    {
        imu660rc_read_registers(IMU660RC_OUTX_L_A, (uint8 *)dat, 6);
        imu660rc_acc_x = dat[0];
        imu660rc_acc_y = dat[1];
        imu660rc_acc_z = dat[2];
    }
}

/**
 * @brief  读取陀螺仪 3 轴原始数据 (仅在未使能四元数融合时生效)
 */
void imu660rc_get_gyro(void)
{
    int16 dat[3];
    if (IMU660RC_QUARTERNION_DISABLE == imu660rc_quarternion_rate)
    {
        imu660rc_read_registers(IMU660RC_OUTX_L_G, (uint8 *)dat, 6);
        imu660rc_gyro_x = dat[0];
        imu660rc_gyro_y = dat[1];
        imu660rc_gyro_z = dat[2];
    }
}

/**
 * @brief  获取姿态解算四元数与欧拉角 (使能四元数时，从嵌入式 SRAM 读取硬件融合解算数据)
 */
void imu660rc_get_quarternion(void)
{
    uint8   i;
    uint16  buff[4];
    uint8   *buff1_ptr;
    int16   *buff2_ptr;

    if (IMU660RC_QUARTERNION_DISABLE != imu660rc_quarternion_rate)
    {
        buff1_ptr = (uint8 *)buff;

        // 切换至嵌入式功能内存 Bank 并提取融合四元数数据
        imu660rc_set_mem_bank(IMU660RC_EMBED_MEM_BANK);
        imu660rc_write_register(IMU660RC_PAGE_RW, 0x20);
        imu660rc_write_register(IMU660RC_PAGE_SEL, 0x31);

        for (i = 0; 8 > i; i++)
        {
            imu660rc_write_register(0x08, 0x4C + i);
            buff1_ptr[i] = imu660rc_read_register(0x09);
        }

        imu660rc_write_register(IMU660RC_PAGE_RW, 0x0);
        imu660rc_set_mem_bank(IMU660RC_MAIN_MEM_BANK);

        // 四元数归一化与欧拉角计算
        quarternion_normalize(imu660rc_quarternion, buff);
        quarternion_to_euler(imu660rc_quarternion, &imu660rc_roll, &imu660rc_pitch, &imu660rc_yaw);

#if (1 == IMU660RC_QUARTERNION_GET_ACC)
        // 同时同步加速度原始数据
        buff2_ptr = (int16 *)buff;
        imu660rc_read_registers(IMU660RC_OUTX_L_A, (uint8 *)buff2_ptr, 6);
        imu660rc_acc_x = buff2_ptr[0];
        imu660rc_acc_y = buff2_ptr[1];
        imu660rc_acc_z = buff2_ptr[2];
#endif
#if (1 == IMU660RC_QUARTERNION_GET_GYRO)
        // 同时同步陀螺仪原始数据
        buff2_ptr = (int16 *)buff;
        imu660rc_read_registers(IMU660RC_OUTX_L_G, (uint8 *)buff2_ptr, 6);
        imu660rc_gyro_x = buff2_ptr[0];
        imu660rc_gyro_y = buff2_ptr[1];
        imu660rc_gyro_z = buff2_ptr[2];
#endif
    }
}

// ===================================================================================================================
// PB24 (INT2) 外部中断服务入口 (对应 MSPM0 GROUP1 中断向量)
// ===================================================================================================================

/**
 * @brief  MSPM0 组 1 GPIO 中断服务函数 (包含 GPIOB 端口的中断触发)
 *         当 IMU660RC 解算完成并从 INT2 (PB24) 输出上升沿脉冲时，自动触发本 ISR 进行姿态更新
 */
// void GROUP1_IRQHandler(void)
// {
//     // 获取 GPIOB 端口引脚 24 (imuInt_int2_PIN) 的中断使能状态
//     uint32_t gpioStat = DL_GPIO_getEnabledInterruptStatus(IMU660RC_INT2_PORT, IMU660RC_INT2_PIN);
//     if (gpioStat & IMU660RC_INT2_PIN)
//     {
//         // 清除 PB24 悬挂中断标志
//         DL_GPIO_clearInterruptStatus(IMU660RC_INT2_PORT, IMU660RC_INT2_PIN);
//         // 执行四元数与姿态角更新
//         imu660rc_get_quarternion();
//     }
// }

// ===================================================================================================================
// 核心初始化入口
// ===================================================================================================================

/**
 * @brief  初始化 IMU660RC 六轴传感器及 INT2 硬件中断
 * @param  quarternion_rate 姿态融合四元数更新速率 (若设为 DISABLE 则不使能 INT2 中断)
 * @return uint8 0: 初始化成功, 1: 初始化失败
 */
uint8 imu660rc_init(imu660rc_quarternion_rate_config quarternion_rate)
{
    uint8 return_state = 0;

    imu660rc_quarternion_rate = quarternion_rate;

    // 配置 CS (PB13) 引脚为普通 GPIO 推挽输出
#ifdef imuInt_CS_IOMUX
    DL_GPIO_initDigitalOutput(imuInt_CS_IOMUX);
#elif defined(GPIO_IMU660RC_CS0_IOMUX)
    DL_GPIO_initDigitalOutput(GPIO_IMU660RC_CS0_IOMUX);
#else
    DL_GPIO_initDigitalOutput(IOMUX_PINCM30);
#endif
    DL_GPIO_setPins(IMU660RC_CS_PORT, IMU660RC_CS_PIN);
    DL_GPIO_enableOutput(IMU660RC_CS_PORT, IMU660RC_CS_PIN);

    system_delay_ms(10);
    // 上电首次读 SPI，促使传感器由 I2C 模式切换为 SPI 模式
    (void)imu660rc_read_register(IMU660RC_CHIP_ID);
    system_delay_ms(5);

    do
    {
        // 1. 检查通讯自检
        if (imu660rc_self_check())
        {
            return_state = 1;
            break;
        }

        // 2. 复位控制与功能开启
        imu660rc_write_register(IMU660RC_FUNC_CFG_ACCESS, 0x04);
        system_delay_ms(30);

        imu660rc_write_register(IMU660RC_CTRL3, 0x44);

        // 3. 配置加速度计量程与比例因子
        switch (IMU660RC_ACC_SAMPLE_DEFAULT)
        {
            default:
            {
                return_state = 1;
            }break;
            case IMU660RC_ACC_SAMPLE_SGN_2G:
            {
                imu660rc_write_register(IMU660RC_CTRL8, 0x00);
                imu660rc_transition_factor[0] = 16393.44f;
            }break;
            case IMU660RC_ACC_SAMPLE_SGN_4G:
            {
                imu660rc_write_register(IMU660RC_CTRL8, 0x01);
                imu660rc_transition_factor[0] = 8196.72f;
            }break;
            case IMU660RC_ACC_SAMPLE_SGN_8G:
            {
                imu660rc_write_register(IMU660RC_CTRL8, 0x02);
                imu660rc_transition_factor[0] = 4098.36f;
            }break;
            case IMU660RC_ACC_SAMPLE_SGN_16G:
            {
                imu660rc_write_register(IMU660RC_CTRL8, 0x03);
                imu660rc_transition_factor[0] = 2049.18f;
            }break;
        }
        if (1 == return_state)
        {
            break;
        }

        // 4. 配置陀螺仪量程与比例因子
        switch (IMU660RC_GYRO_SAMPLE_DEFAULT)
        {
            default:
            {
                return_state = 1;
            }break;
            case IMU660RC_GYRO_SAMPLE_SGN_125DPS:
            {
                imu660rc_write_register(IMU660RC_CTRL6, 0x00);
                imu660rc_transition_factor[1] = 228.5714f;
            }break;
            case IMU660RC_GYRO_SAMPLE_SGN_250DPS:
            {
                imu660rc_write_register(IMU660RC_CTRL6, 0x01);
                imu660rc_transition_factor[1] = 114.2857f;
            }break;
            case IMU660RC_GYRO_SAMPLE_SGN_500DPS:
            {
                imu660rc_write_register(IMU660RC_CTRL6, 0x02);
                imu660rc_transition_factor[1] = 57.1428f;
            }break;
            case IMU660RC_GYRO_SAMPLE_SGN_1000DPS:
            {
                imu660rc_write_register(IMU660RC_CTRL6, 0x03);
                imu660rc_transition_factor[1] = 28.5714f;
            }break;
            case IMU660RC_GYRO_SAMPLE_SGN_2000DPS:
            {
                imu660rc_write_register(IMU660RC_CTRL6, 0x04);
                imu660rc_transition_factor[1] = 14.2857f;
            }break;
            case IMU660RC_GYRO_SAMPLE_SGN_4000DPS:
            {
                imu660rc_write_register(IMU660RC_CTRL6, 0x0C);
                imu660rc_transition_factor[1] = 7.14285f;
            }break;
        }
        if (1 == return_state)
        {
            break;
        }

        // 5. 滤波器与工作模式设置
        imu660rc_write_register(IMU660RC_CTRL1, 0x15);
        imu660rc_write_register(IMU660RC_CTRL2, 0x18);
        imu660rc_write_register(IMU660RC_CTRL7, 0x01);
        imu660rc_write_register(IMU660RC_CTRL9, 0x08);

        // 6. 配置姿态融解引擎与 INT2 中断脚输出
        if (IMU660RC_QUARTERNION_DISABLE != quarternion_rate)
        {
            imu660rc_write_register(IMU660RC_FIFO_CRTL1, 0x01);
            imu660rc_write_register(IMU660RC_FIFO_CRTL4, 0x06);
            imu660rc_write_register(IMU660RC_INT2_CTRL, 0x80);                 // 配置硬件数据就绪输出至 INT2 物理引脚
            imu660rc_write_register(IMU660RC_CTRL4, 0x08);

            imu660rc_write_register(IMU660RC_CTRL1, 0x10 | (quarternion_rate + 3));
            imu660rc_write_register(IMU660RC_CTRL2, 0x10 | (quarternion_rate + 3));

            imu660rc_set_mem_bank(IMU660RC_EMBED_MEM_BANK);

            imu660rc_write_register(IMU660RC_EMB_FUNC_FIFO_EN_A, 0x02);
            imu660rc_write_register(IMU660RC_SFLP_ODR, 0x43 | (quarternion_rate << 3));
            imu660rc_write_register(IMU660RC_EMB_FUNC_EN_A, 0x02);
            imu660rc_write_register(IMU660RC_PAGE_RW, 0x00);
            imu660rc_set_mem_bank(IMU660RC_MAIN_MEM_BANK);

            // 7. 配置 MSPM0 单片机 PB24 引脚上升沿中断与 NVIC
            DL_GPIO_setUpperPinsPolarity(IMU660RC_INT2_PORT, DL_GPIO_PIN_24_EDGE_RISE);
            DL_GPIO_clearInterruptStatus(IMU660RC_INT2_PORT, IMU660RC_INT2_PIN);
            DL_GPIO_enableInterrupt(IMU660RC_INT2_PORT, IMU660RC_INT2_PIN);
            NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
            NVIC_EnableIRQ(GPIOB_INT_IRQn);
        }
    } while (0);

    return return_state;
}
