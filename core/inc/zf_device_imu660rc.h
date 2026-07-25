/*********************************************************************************************************************
* MSPM0G3519 IMU660RC 六轴陀螺仪加速度计驱动 (TI Official DriverLib 移植版)
* 
* 硬件引脚连接说明 (基于 ti_msp_dl_config.h / empty.syscfg 配置):
* -------------------------------------------------------------------------------------------------------------------
* IMU660RC 模块引脚      单片机引脚 (MSPM0G3519)                      说明
* SCL / SPC             PB16 (GPIO_IMU660RC_SCLK, SPI1 SCLK)        SPI 时钟线
* SDA / SDI / MOSI      PB15 (GPIO_IMU660RC_PICO, SPI1 PICO)        SPI 主出从入
* SA0 / SDO / MISO      PB14 (GPIO_IMU660RC_POCI, SPI1 POCI)        SPI 主入从出
* CS                    PB13 (imuInt_CS_PIN, GPIOB)                  SPI 软件控制片选信号 (GPIO)
* INT2                  PB24 (imuInt_int2_PIN, GPIOB)                数据就绪/四元数更新中断脚 (引脚外部中断)
* VCC                   3.3V
* GND                   电源地
* -------------------------------------------------------------------------------------------------------------------
********************************************************************************************************************/

#ifndef _zf_device_imu660rc_h_
#define _zf_device_imu660rc_h_

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#ifndef _ZF_TYPES_DEFINED_
#define _ZF_TYPES_DEFINED_
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
#endif

//==================================================== 硬件与中断引脚配置 ====================================================
#define IMU660RC_INT2_PORT              ( imuInt_PORT )                         // INT2 挂载端口：GPIOB
#define IMU660RC_INT2_PIN               ( imuInt_int2_PIN )                     // INT2 挂载引脚：DL_GPIO_PIN_24 (PB24)

// 兼容用户重新定义的 PB13 CS 片选宏 (优先使用用户定义的 imuInt_CS_PIN)
#ifdef imuInt_CS_PIN
#define IMU660RC_CS_PORT                ( imuInt_PORT )
#define IMU660RC_CS_PIN                 ( imuInt_CS_PIN )
#else
#define IMU660RC_CS_PORT                ( GPIOB )
#define IMU660RC_CS_PIN                 ( DL_GPIO_PIN_13 )
#endif

#define IMU660RC_QUARTERNION_GET_GYRO   ( 1 )                                   // 1-姿态更新时同时读取角速度原始数据 0-不读取
#define IMU660RC_QUARTERNION_GET_ACC    ( 1 )                                   // 1-姿态更新时同时读取加速度原始数据 0-不读取
#define IMU660RC_ACC_SAMPLE_DEFAULT     ( IMU660RC_ACC_SAMPLE_SGN_8G )          // 默认加速度计采样量程 (±8G)
#define IMU660RC_GYRO_SAMPLE_DEFAULT    ( IMU660RC_GYRO_SAMPLE_SGN_2000DPS )    // 默认陀螺仪采样量程 (±2000DPS)

// 内存 Bank 切换枚举定义
typedef enum
{
    IMU660RC_MAIN_MEM_BANK  = 0x00,                                             // 主内存 Bank
    IMU660RC_HUB_MEM_BANK   = 0x40,                                             // Hub 内存 Bank
    IMU660RC_EMBED_MEM_BANK = 0x80,                                             // 嵌入式功能 Bank (四元数解算引擎)
} imu660rc_mem_bank_enum;

// 加速度计采样量程枚举
typedef enum
{
    IMU660RC_ACC_SAMPLE_SGN_2G ,                                                // 加速度量程 ±2G
    IMU660RC_ACC_SAMPLE_SGN_4G ,                                                // 加速度量程 ±4G  
    IMU660RC_ACC_SAMPLE_SGN_8G ,                                                // 加速度量程 ±8G  
    IMU660RC_ACC_SAMPLE_SGN_16G,                                                // 加速度量程 ±16G 
} imu660rc_acc_sample_config;

// 陀螺仪采样量程枚举
typedef enum
{
    IMU660RC_GYRO_SAMPLE_SGN_125DPS ,                                           // 陀螺仪量程 ±125DPS (度/秒)
    IMU660RC_GYRO_SAMPLE_SGN_250DPS ,                                           // 陀螺仪量程 ±250DPS  
    IMU660RC_GYRO_SAMPLE_SGN_500DPS ,                                           // 陀螺仪量程 ±500DPS  
    IMU660RC_GYRO_SAMPLE_SGN_1000DPS,                                           // 陀螺仪量程 ±1000DPS 
    IMU660RC_GYRO_SAMPLE_SGN_2000DPS,                                           // 陀螺仪量程 ±2000DPS 
    IMU660RC_GYRO_SAMPLE_SGN_4000DPS,                                           // 陀螺仪量程 ±4000DPS 
} imu660rc_gyro_sample_config;

// 四元数融合姿态刷新率配置枚举
typedef enum
{
    IMU660RC_QUARTERNION_15HZ,                                                  // 姿态解算刷新率 15 Hz
    IMU660RC_QUARTERNION_30HZ,                                                  // 姿态解算刷新率 30 Hz
    IMU660RC_QUARTERNION_60HZ,                                                  // 姿态解算刷新率 60 Hz
    IMU660RC_QUARTERNION_120HZ,                                                 // 姿态解算刷新率 120 Hz
    IMU660RC_QUARTERNION_240HZ,                                                 // 姿态解算刷新率 240 Hz
    IMU660RC_QUARTERNION_480HZ,                                                 // 姿态解算刷新率 480 Hz
    IMU660RC_QUARTERNION_DISABLE,                                               // 禁用硬件四元数输出 (仅手动读取原始数据)
} imu660rc_quarternion_rate_config;

//==================================================== IMU660RC 通讯协议与寄存器地址 ====================================================
#define IMU660RC_DEV_ADDR           ( 0x6B )                                    // I2C 设备地址 (SA0拉高0x6B, 拉低0x6A)
#define IMU660RC_SPI_W              ( 0x00 )                                    // SPI 写掩码 (Bit7 = 0)
#define IMU660RC_SPI_R              ( 0x80 )                                    // SPI 读掩码 (Bit7 = 1)
#define IMU660RC_TIMEOUT_COUNT      ( 0x00FF )                                  // 通讯超时判断计数阈值

// 核心控制与数据寄存器地址定义
#define IMU660RC_FUNC_CFG_ACCESS    ( 0x01 )                                    // 内存 Bank 切换控制寄存器
#define IMU660RC_FIFO_CRTL1         ( 0x07 )                                    // FIFO 控制寄存器 1
#define IMU660RC_FIFO_CRTL2         ( 0x08 )                                    // FIFO 控制寄存器 2
#define IMU660RC_FIFO_CRTL3         ( 0x09 )                                    // FIFO 控制寄存器 3
#define IMU660RC_FIFO_CRTL4         ( 0x0A )                                    // FIFO 控制寄存器 4
#define IMU660RC_INT2_CTRL          ( 0x0E )                                    // INT2 中断引脚控制寄存器
#define IMU660RC_CHIP_ID            ( 0x0F )                                    // WHO_AM_I 芯片 ID 寄存器 (默认值 0x70)
#define IMU660RC_CTRL1              ( 0x10 )                                    // 加速度计控制寄存器 1
#define IMU660RC_CTRL2              ( 0x11 )                                    // 陀螺仪控制寄存器 2
#define IMU660RC_CTRL3              ( 0x12 )                                    // 控制寄存器 3 (软件复位/BDU设置)
#define IMU660RC_CTRL4              ( 0x13 )                                    // 控制寄存器 4
#define IMU660RC_CTRL5              ( 0x14 )                                    // 控制寄存器 5
#define IMU660RC_CTRL6              ( 0x15 )                                    // 控制寄存器 6 (陀螺仪量程与低通滤波器)
#define IMU660RC_CTRL7              ( 0x16 )                                    // 控制寄存器 7
#define IMU660RC_CTRL8              ( 0x17 )                                    // 控制寄存器 8 (加速度计量程设置)
#define IMU660RC_CTRL9              ( 0x18 )                                    // 控制寄存器 9
#define IMU660RC_CTRL10             ( 0x19 )                                    // 控制寄存器 10
#define IMU660RC_CTRL_STATUS        ( 0x1A )                                    // 控制状态寄存器
#define IMU660RC_STATUS_REG         ( 0x1E )                                    // 数据就绪状态寄存器
#define IMU660RC_OUT_TEMP_L         ( 0x20 )                                    // 温度低字节
#define IMU660RC_OUT_TEMP_H         ( 0x21 )                                    // 温度高字节
#define IMU660RC_OUTX_L_G           ( 0x22 )                                    // 陀螺仪 X 轴低字节
#define IMU660RC_OUTX_H_G           ( 0x23 )                                    // 陀螺仪 X 轴高字节
#define IMU660RC_OUTY_L_G           ( 0x24 )                                    // 陀螺仪 Y 轴低字节
#define IMU660RC_OUTY_H_G           ( 0x25 )                                    // 陀螺仪 Y 轴高字节
#define IMU660RC_OUTZ_L_G           ( 0x26 )                                    // 陀螺仪 Z 轴低字节
#define IMU660RC_OUTZ_H_G           ( 0x27 )                                    // 陀螺仪 Z 轴高字节
#define IMU660RC_OUTX_L_A           ( 0x28 )                                    // 加速度计 X 轴低字节
#define IMU660RC_OUTX_H_A           ( 0x29 )                                    // 加速度计 X 轴高字节
#define IMU660RC_OUTY_L_A           ( 0x2A )                                    // 加速度计 Y 轴低字节
#define IMU660RC_OUTY_H_A           ( 0x2B )                                    // 加速度计 Y 轴高字节
#define IMU660RC_OUTZ_L_A           ( 0x2C )                                    // 加速度计 Z 轴低字节
#define IMU660RC_OUTZ_H_A           ( 0x2D )                                    // 加速度计 Z 轴高字节

#define IMU660RC_PAGE_SEL           ( 0x02 )
#define IMU660RC_EMB_FUNC_EN_A      ( 0x04 )
#define IMU660RC_PAGE_RW            ( 0x17 )    
#define IMU660RC_EMB_FUNC_FIFO_EN_A ( 0x44 )  
#define IMU660RC_SFLP_ODR           ( 0x5E )
#define IMU660RC_EMB_FUNC_CFG       ( 0x63 )

//==================================================== 全局变量声明 ====================================================
extern float imu660rc_transition_factor[2];                                   // 物理单位转换比例因子 [0]:加速度, [1]:陀螺仪
extern int16 imu660rc_gyro_x,   imu660rc_gyro_y,    imu660rc_gyro_z;            // 陀螺仪三轴原始测量值
extern int16 imu660rc_acc_x ,   imu660rc_acc_y ,    imu660rc_acc_z;             // 加速度计三轴原始测量值
extern float imu660rc_roll  ,   imu660rc_pitch ,    imu660rc_yaw;               // 姿态解算得到的欧拉角 (单位: 角度 deg)
extern float imu660rc_quarternion[4];                                           // 姿态融合四元数 [q0, q1, q2, q3]

//==================================================== 函数接口声明 ====================================================

/**
 * @brief  获取加速度计原始数据 (仅在禁用四元数模式下单独调用)
 */
void    imu660rc_get_acc            (void);

/**
 * @brief  获取陀螺仪原始数据 (仅在禁用四元数模式下单独调用)
 */
void    imu660rc_get_gyro           (void);

/**
 * @brief  获取并更新硬件融合四元数及欧拉角 (INT2中断自动调用或手动刷新)
 */
void    imu660rc_get_quarternion    (void);

/**
 * @brief  将加速度计原始数据转换为物理加速度 (单位: g, 1g ≈ 9.80 m/s^2)
 */
#define imu660rc_acc_transition(acc_value)      ((float)(acc_value) / imu660rc_transition_factor[0])

/**
 * @brief  将陀螺仪原始数据转换为角速度 (单位: deg/s 角度每秒)
 */
#define imu660rc_gyro_transition(gyro_value)    ((float)(gyro_value) / imu660rc_transition_factor[1])

/**
 * @brief  初始化 IMU660RC 六轴传感器 (初始化 SPI、自检、量程及 INT2 中断)
 * @param  quarternion_rate  四元数姿态解算刷新率配置
 * @return uint8  0: 初始化成功, 1: 初始化失败(自检失败或配置错误)
 */
uint8   imu660rc_init               (imu660rc_quarternion_rate_config quarternion_rate);

#endif