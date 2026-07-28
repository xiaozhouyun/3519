/**
 * @file drv8873.c
 * @brief DRV8873 电机驱动实现文件
 * 
 * 主要实现:
 * 1. 速度逻辑范围 [-1000, 1000] 到 PWM 比较值的转换 (边沿对齐模式 Period - Duty)
 * 2. 方向 (PH/Phase) 引脚电平控制
 * 3. 基于 IPROPI 镜像电流公式 (Iout = Vipropi * K / Rsense) 的 ADC 采样与 mA 换算
 */

#include "drv8873.h"


#include "ti_msp_dl_config.h"



/** ADC 转换等待超时循环计数阈值 */
#define DRV8873_ADC_TIMEOUT (100000U)



/**
 * @brief  将逻辑速度指令 [-1000, 1000] 一步解算为方向与定时器 CC 比较值
 */
DRV8873_Control_t DRV8873_Speed_To_Control(int16_t speed)
{
    DRV8873_Control_t ctrl;

    // 1. 限幅 [-DRV8873_SPEED_MAX, DRV8873_SPEED_MAX]
    if (speed > DRV8873_SPEED_MAX) {
        speed = DRV8873_SPEED_MAX;
    } else if (speed < -DRV8873_SPEED_MAX) {
        speed = -DRV8873_SPEED_MAX;
    }

    // 2. 解算电机方向 (正数为 FORWARD，负数为 REVERSE)
    ctrl.dir = (speed >= 0) ? DRV8873_DIR_FORWARD : DRV8873_DIR_REVERSE;

    // 3. 计算绝对值速度与 PWM 占空比 (0 ~ DRV8873_PWM_PERIOD)
    uint16_t abs_speed = (speed < 0) ? (uint16_t)(-speed) : (uint16_t)speed;
    uint16_t duty = (uint16_t)(((uint32_t)abs_speed * DRV8873_PWM_PERIOD) / DRV8873_SPEED_MAX);

    // 4. 根据边沿对齐模式计算 CC 比较值 (Compare = Period - Duty)
    if (duty >= DRV8873_PWM_PERIOD) {
        ctrl.compare = 0U;
    } else {
        ctrl.compare = (uint16_t)( duty);
    }


    return ctrl;
}

/**
 * @brief  将 ADC 原始采样转换换算为实际输出电流 (单位: mA)
 *         换算公式: I(mA) = (adc_raw * Vref_mV / 4095) * K / Rsense_ohm
 */
uint32_t DRV8873_Adc_To_Current_mA(uint16_t adc_raw)
{
    uint64_t numerator = (uint64_t)adc_raw * DRV8873_ADC_REF_MV * DRV8873_IPROPI_K;
    uint32_t denominator = DRV8873_ADC_FULL_SCALE * DRV8873_SENSE_RESISTOR_OHM;
    return (uint32_t)(numerator / denominator);
}

#ifndef DRV8873_HOST_TEST
/**
 * @brief  写入硬件配置，同时更新电机方向 GPIO 引脚与 PWM 定时器比较值
 * @param  channel 电机通道 (通道2 PB18/C1, 通道1 PA22/C0)
 * @param  dir     运行方向 (1为正转，0为反转)
 * @param  compare 定时器 CC 比较值（0-1000）
 */
static void DRV8873_Write_Hardware(DRV8873_Channel_t channel, DRV8873_Direction_t dir, uint16_t compare)
{
    if (channel == DRV8873_CH2) {
        if (dir == DRV8873_DIR_FORWARD) {
            DL_GPIO_setPins(DRV8873HPWPT_PH2_PORT, DRV8873HPWPT_PH2_PIN);
        } else {
            DL_GPIO_clearPins(DRV8873HPWPT_PH2_PORT, DRV8873HPWPT_PH2_PIN);
        }
        DL_TimerG_setCaptureCompareValue(DRV8873_INST, compare, GPIO_DRV8873_C1_IDX);
    } else {
        if (dir == DRV8873_DIR_FORWARD) {
            DL_GPIO_clearPins(DRV8873HPWPT_PH1_PORT, DRV8873HPWPT_PH1_PIN);
        } else {
            DL_GPIO_setPins(DRV8873HPWPT_PH1_PORT, DRV8873HPWPT_PH1_PIN);
        }
        DL_TimerG_setCaptureCompareValue(DRV8873_INST, compare, GPIO_DRV8873_C0_IDX);
    }
}

/**
 * @brief  DRV8873 驱动模块硬件初始化
 */
void DRV8873_Init(void)
{
    /* 设置 ADC1 采样时间 0 为 50 个 clock 周期，保证电流采样稳定性 */
    DL_ADC12_setSampleTime0(ADC1_INST, 50);

    DRV8873_Stop_All();                     // 上电默认关闭电机
    DL_TimerG_startCounter(DRV8873_INST);   // 启动定时器 PWM 计数
}

/**
 * @brief  停止指定通道电机 (比较值设为周期值，即 0 占空比)
 */
void DRV8873_Stop(DRV8873_Channel_t channel)
{
    DRV8873_Write_Hardware(channel, DRV8873_DIR_FORWARD, 0);
}

/**
 * @brief  停止所有通道电机
 */
void DRV8873_Stop_All(void)
{
    DRV8873_Stop(DRV8873_CH1);
    DRV8873_Stop(DRV8873_CH2);
}

/**
 * @brief  设置指定通道电机的运行速度与方向 （这里速度没有转换 暂时理解成pwm值0-1000）
 */
void DRV8873_Set_Speed(DRV8873_Channel_t channel, int16_t speed)
{
    DRV8873_Control_t ctrl = DRV8873_Speed_To_Control(speed);

    DRV8873_Write_Hardware(channel, ctrl.dir, ctrl.compare);
}



/**
 * @brief  触发 ADC1 采样并读取指定电机通道的 IPROPI 采样原始值
 */
uint16_t DRV8873_Read_Current_Raw(DRV8873_Channel_t channel)
{
    uint32_t timeout = DRV8873_ADC_TIMEOUT;
    uint32_t flag = (channel == DRV8873_CH2) ? DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED
                                             : DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED;
    DL_ADC12_MEM_IDX mem = (channel == DRV8873_CH2) ? ADC1_ADCMEM_DRC8873_ADC1
                                                    : ADC1_ADCMEM_DRC8873_ADC0;

    // 清除标志位并使能转换
    DL_ADC12_clearInterruptStatus(ADC1_INST,
        DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED | DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
    DL_ADC12_enableConversions(ADC1_INST);
    DL_ADC12_startConversion(ADC1_INST);

    // 等待 ADC 转换完成标志或超时
    while (!DL_ADC12_getRawInterruptStatus(ADC1_INST, flag) && --timeout) {
    }

    return DL_ADC12_getMemResult(ADC1_INST, mem);
}

/**
 * @brief  读取指定通道电机的实测电流 (mA)
 */
uint32_t DRV8873_Read_Current_mA(DRV8873_Channel_t channel)
{
    return DRV8873_Adc_To_Current_mA(DRV8873_Read_Current_Raw(channel));
}
#endif

