/**
 * @file encode.c
 * @brief 编码器模块实现文件，基于硬件定时器 (GPTIMER) 读取 AB 相编码器脉冲
 */

#include "encode.h"

#ifndef ENCODE_HOST_TEST
#include "ti_msp_dl_config.h"
#endif

/** 16位定时器最大计数值掩码 */
#define ENCODE_COUNTER_MAX (65535U)

#ifndef ENCODE_HOST_TEST
/** 记录左右通道上一次读取时的编码器计数值 */
static uint16_t g_encode_last[2];

/**
 * @brief  获取编码器通道对应的硬件 GPTIMER 寄存器指针
 * @param  channel 编码器通道
 * @return GPTIMER_Regs 指针
 */
static GPTIMER_Regs *Encode_Get_Timer(Encode_Channel_t channel)
{
    return (channel == ENCODE_RIGHT) ? AB2_INST : AB1_INST;
}
#endif

/**
 * @brief  根据前后两次计数值计算增量 (无符号16位减法在溢出/下溢时依靠二补码性质自动完成溢出纠正)
 */
int16_t Encode_Calc_Delta(uint16_t now, uint16_t last)
{
    return (int16_t)(now - last);
}

#ifndef ENCODE_HOST_TEST
/**
 * @brief  初始化编码器模块，清空左右通道历史计数
 */
void Encode_Init(void)
{
    Encode_Clear(ENCODE_LEFT);
    Encode_Clear(ENCODE_RIGHT);
}

/**
 * @brief  清空并重置指定通道编码器计数器
 */
void Encode_Clear(Encode_Channel_t channel)
{
    GPTIMER_Regs *timer = Encode_Get_Timer(channel);

    DL_TimerG_stopCounter(timer);       // 暂停计数器
    DL_TimerG_setTimerCount(timer, 0U);  // 计数值清零
    g_encode_last[channel] = 0U;        // 重置上次采样记录
    DL_TimerG_startCounter(timer);      // 重新启动计数器
}

/**
 * @brief  读取指定通道编码器当前硬件计数值
 */
uint16_t Encode_Get_Count(Encode_Channel_t channel)
{
    return (uint16_t)(DL_TimerG_getTimerCount(Encode_Get_Timer(channel)) & ENCODE_COUNTER_MAX);
}

/**
 * @brief  获取并更新指定通道自上次读取以来的脉冲增量
 */
int16_t Encode_Get_Delta(Encode_Channel_t channel)
{
    uint16_t now = Encode_Get_Count(channel);
    int16_t delta = Encode_Calc_Delta(now, g_encode_last[channel]);

    g_encode_last[channel] = now; // 更新上一次计数值记录
    return delta;
}
#endif

