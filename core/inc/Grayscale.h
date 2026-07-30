/*********************************************************************************************************************
* MSPM0G3519 八通道灰度循迹传感器驱动 (感为串行输出版)
*
* 硬件连接：PA1 接 CLK 输出，PA0 接 DAT 输入，5V 供电并与主控共地。
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



// 8通道灰度传感器数据结构体
typedef struct {
    uint8_t  digital;               // 8 通道黑白二值化开关状态 (Bit0~Bit7, 1:白地面, 0:黑线)
    uint8_t  is_ok;                 // 校准就绪标志 (1-已就绪, 0-未就绪)
} Grayscale_Sensor_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  轮询读取 8 通道串行数字量并更新数据
 * @param  sensor  灰度传感器结构体指针
 */
void Grayscale_Update(Grayscale_Sensor_t *sensor);

/**
 * @brief  获取 8 位黑白开关二值化按位结果
 * @param  sensor  灰度传感器结构体指针
 * @return uint8_t 8位状态 (Bit0~Bit7 代表 8 个通道，1:白地面, 0:黑线)
 */
uint8_t Grayscale_Get_Digital(Grayscale_Sensor_t *sensor);



#ifdef __cplusplus
}
#endif

#endif
