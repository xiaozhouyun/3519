/*********************************************************************************************************************
* MSPM0G3519 八通道灰度循迹传感器驱动 (TI Official DriverLib 移植版)
* 
* 硬件引脚连接说明 (基于 ti_msp_dl_config.h / empty.syscfg 配置):
* -------------------------------------------------------------------------------------------------------------------
* 信号线               单片机引脚 (MSPM0G3519)                      说明
* 通道切换 A0 (Bit0)    PA7 (xunjiGPIO_PIN_1_PIN, GPIOA)             多路开关地址0
* 通道切换 A1 (Bit1)    PA8 (xunjiGPIO_PIN_2_PIN, GPIOA)             多路开关地址1
* 通道切换 A2 (Bit2)    PA9 (xunjiGPIO_PIN_3_PIN, GPIOA)             多路开关地址2
* 模拟量 ADC 输入       PA27 (ADC0_xunji_INST, ADC0 MemIdx0)         灰度传感器模拟信号采样
* -------------------------------------------------------------------------------------------------------------------
********************************************************************************************************************/

#ifndef _GRAYSCALE_H_
#define _GRAYSCALE_H_

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef _ZF_TYPES_DEFINED_
#define _ZF_TYPES_DEFINED_
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
#endif

// 默认黑白校准预设阈值 (预设白底约3500, 黑线约500)
#define GRAYSCALE_DEFAULT_WHITE     3500
#define GRAYSCALE_DEFAULT_BLACK     500

// 8通道灰度传感器数据结构体
typedef struct {
    uint16_t analog_val[8];         // 8 通道原始 ADC 模拟值 (0 ~ 4095)
    uint16_t normal_val[8];         // 8 通道归一化模拟值 (0 ~ 4095)
    uint16_t calibrated_white[8];   // 白地面校准门限值
    uint16_t calibrated_black[8];   // 黑线校准门限值
    uint16_t gray_white[8];         // 灰度白门限 (2/3点)
    uint16_t gray_black[8];         // 灰度黑门限 (1/3点)
    double   normal_factor[8];      // 归一化缩放比例系数
    uint8_t  digital;               // 8 通道黑白二值化开关状态 (Bit0~Bit7, 1:白地面, 0:黑线)
    uint8_t  is_ok;                 // 校准就绪标志 (1-已就绪, 0-未就绪)
} Grayscale_Sensor_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  传感器结构体初次默认初始化 (预设默认黑白阈值，上电即可直接使用)
 * @param  sensor  灰度传感器结构体指针
 */
void Grayscale_Init_First(Grayscale_Sensor_t *sensor);

/**
 * @brief  带自定义黑白校准参数的灰度传感器初始化
 * @param  sensor            灰度传感器结构体指针
 * @param  calibrated_white  8通道白地面校准数组 (长度8)
 * @param  calibrated_black  8通道黑线校准数组 (长度8)
 */
void Grayscale_Init(Grayscale_Sensor_t *sensor, const uint16_t *calibrated_white, const uint16_t *calibrated_black);

/**
 * @brief  快捷统一设置所有 8 个通道的白地面与黑线目标阈值
 * @param  sensor  灰度传感器结构体指针
 * @param  white   白地面目标 ADC 门限 (如 3200)
 * @param  black   黑线目标 ADC 门限 (如 600)
 */
void Grayscale_Set_Global_Thresholds(Grayscale_Sensor_t *sensor, uint16_t white, uint16_t black);

/**
 * @brief  轮询采样 8 通道模拟量并更新二值化数字状态与归一化数据
 * @param  sensor  灰度传感器结构体指针
 */
void Grayscale_Update(Grayscale_Sensor_t *sensor);

/**
 * @brief  获取 8 位黑白开关二值化按位结果
 * @param  sensor  灰度传感器结构体指针
 * @return uint8_t 8位状态 (Bit0~Bit7 代表 8 个通道，1:白地面, 0:黑线)
 */
uint8_t Grayscale_Get_Digital(Grayscale_Sensor_t *sensor);

/**
 * @brief  获取 8 通道归一化模拟数据 (0 ~ 4095)
 * @param  sensor  灰度传感器结构体指针
 * @param  result  目标接收缓冲区 (uint16_t 数组，长度 8)
 * @return uint8_t 1:成功, 0:未就绪
 */
uint8_t Grayscale_Get_Normalized(Grayscale_Sensor_t *sensor, uint16_t *result);

/**
 * @brief  获取 8 通道原始 ADC 模拟数据 (0 ~ 4095)
 * @param  sensor  灰度传感器结构体指针
 * @param  result  目标接收缓冲区 (uint16_t 数组，长度 8)
 * @return uint8_t 1:成功, 0:未就绪
 */
uint8_t Grayscale_Get_Analog(Grayscale_Sensor_t *sensor, uint16_t *result);

#ifdef __cplusplus
}
#endif

#endif