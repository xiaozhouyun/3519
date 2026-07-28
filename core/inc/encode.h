#ifndef ENCODE_H
#define ENCODE_H

/**
 * @file encode.h
 * @brief 编码器模块头文件，提供双通道正交编码器计数与增量解算接口
 */

#include <stdint.h>

/**
 * @brief 编码器通道枚举
 */
typedef enum {
    ENCODE_LEFT  = 0,  /**< 左轮编码器通道 (对应硬件 AB1 计数器) */
    ENCODE_RIGHT = 1   /**< 右轮编码器通道 (对应硬件 AB2 计数器) */
} Encode_Channel_t;

/** 全局左右轮编码器增量值 */
extern int16_t g_encoder_left_delta;
extern int16_t g_encoder_right_delta;

/** 全局左右轮编码器总脉冲累计值 */
extern int32_t g_encoder_left_total;
extern int32_t g_encoder_right_total;



/**
 * @brief  初始化左右编码器模块
 */
void Encode_Init(void);

/**
 * @brief  清空并重置指定通道编码器计数器
 * @param  channel 编码器通道选择 (@ref Encode_Channel_t)
 */
void Encode_Clear(Encode_Channel_t channel);

/**
 * @brief  读取指定通道编码器当前硬件计数值
 * @param  channel 编码器通道选择 (@ref Encode_Channel_t)
 * @return 16位未符号硬件计数值 (0~65535)
 */
uint16_t Encode_Get_Count(Encode_Channel_t channel);

/**
 * @brief  获取并更新指定通道自上次读取以来的脉冲增量 (Delta)
 * @param  channel 编码器通道选择 (@ref Encode_Channel_t)
 * @return 16位有符号脉冲增量值 (正数表示正转，负数表示反转)
 */
int16_t Encode_Get_Delta(Encode_Channel_t channel);

/**
 * @brief  根据前后两次计数值计算带溢出/回绕处理的增量 (纯数学计算接口)
 * @param  now  当前计数值
 * @param  last 上一次计数值
 * @return 16位有符号脉冲增量
 */
int16_t Encode_Calc_Delta(uint16_t now, uint16_t last);

#endif

