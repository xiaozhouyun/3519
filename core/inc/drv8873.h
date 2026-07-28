#ifndef DRV8873_H
#define DRV8873_H

/**
 * @file drv8873.h
 * @brief DRV8873 双通道 H 桥电机驱动芯片驱动头文件 (PH/EN 控制模式 + IPROPI 电流采样)
 * 
 * 硬件引脚映射说明 (基于 SysConfig 配置):
 * - CH1 (电机通道1): PWM(PB20/TIMG12_CCP0) + 方向PH1(PA22) + 电流采样ADC1_MEM0(PA15)
 * - CH2 (电机通道2): PWM(PA25/TIMG12_CCP1) + 方向PH2(PB18) + 电流采样ADC1_MEM1(PB17)
 */

#include <stdint.h>

/** 电机速度控制最大逻辑限制范围 [-1000, 1000] */
#define DRV8873_SPEED_MAX             (1000)

/** PWM 定时器周期计数值 (对应 SysConfig 中的 LoadValue) */
#define DRV8873_PWM_PERIOD            (1000U)

/** ADC 参考电压 (单位: mV) */
#define DRV8873_ADC_REF_MV            (3300U)

/** 12位 ADC 满量程读数 (2^12 - 1) */
#define DRV8873_ADC_FULL_SCALE        (4095U)

/** DRV8873 IPROPI 镜像电流比例系数 (典型值 1100 A/A) */
#define DRV8873_IPROPI_K              (1100U)

/** IPROPI 引脚对地采样电阻阻值 (单位: 欧姆 3kzΩ) */
#define DRV8873_SENSE_RESISTOR_OHM    (3000U)

/**
 * @brief DRV8873 电机驱动通道枚举
 */
typedef enum {
    DRV8873_CH1 = 0,  /**< 通道 1 (左电机) */
    DRV8873_CH2 = 1   /**< 通道 2 (右电机) */
} DRV8873_Channel_t;

/**
 * @brief 电机运行方向枚举
 */
typedef enum {
    DRV8873_DIR_REVERSE = 0,  /**< 反转 (PH 引脚拉低) */
    DRV8873_DIR_FORWARD = 1   /**< 正转 (PH 引脚拉高) */
} DRV8873_Direction_t;

/**
 * @brief 电机控制解算输出结构体 (包含方向与 PWM 比较寄存器值)
 */
typedef struct {
    DRV8873_Direction_t dir;  /**< 电机方向 */
    uint16_t compare;         /**< 定时器 CC 比较值 */
} DRV8873_Control_t;

/**
 * @brief  初始化 DRV8873 电机驱动模块 (停止所有电机并启动 PWM 计数器)
 */
void DRV8873_Init(void);

/**
 * @brief  停止指定通道电机的运转 (PWM占空比清零)
 * @param  channel 电机通道 (@ref DRV8873_Channel_t)
 */
void DRV8873_Stop(DRV8873_Channel_t channel);

/**
 * @brief  停止所有通道电机的运转
 */
void DRV8873_Stop_All(void);

/**
 * @brief  设置指定通道电机的运行速度与方向
 * @param  channel 电机通道 (@ref DRV8873_Channel_t)
 * @param  speed   目标速度限制在 [-1000, 1000]，正数为正转，负数为反转，0为停止
 */
void DRV8873_Set_Speed(DRV8873_Channel_t channel, int16_t speed);

/**
 * @brief  读取指定通道 DRV8873 采样引脚的 ADC 原始读数 (0~4095)
 * @param  channel 电机通道 (@ref DRV8873_Channel_t)
 * @return 12位 ADC 原始采样值
 */
uint16_t DRV8873_Read_Current_Raw(DRV8873_Channel_t channel);

/**
 * @brief  读取指定通道电机当前的实际输出电流 (单位: mA)
 * @param  channel 电机通道 (@ref DRV8873_Channel_t)
 * @return 电流测量值 (mA)
 */
uint32_t DRV8873_Read_Current_mA(DRV8873_Channel_t channel);

/**
 * @brief  将逻辑速度指令 [-1000, 1000] 一步解算为方向与定时器比较值
 * @param  speed 逻辑速度指令
 * @return 解算后的电机控制结构体 (@ref DRV8873_Control_t)
 */
DRV8873_Control_t DRV8873_Speed_To_Control(int16_t speed);

/**
 * @brief  将 ADC 原始采样值换算为电流毫安数 (mA) (纯计算接口)
 * @param  adc_raw 12位 ADC 原始采样值
 * @return 换算后的毫安电流 (mA)
 */
uint32_t DRV8873_Adc_To_Current_mA(uint16_t adc_raw);

#endif


