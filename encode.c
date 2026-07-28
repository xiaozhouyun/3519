#include "encode.h"

#ifndef ENCODE_HOST_TEST
#include "ti_msp_dl_config.h"
#endif

#define ENCODE_COUNTER_MAX (65535U)

#ifndef ENCODE_HOST_TEST
static uint16_t g_encode_last[2];

static GPTIMER_Regs *Encode_Get_Timer(Encode_Channel_t channel)
{
    return (channel == ENCODE_RIGHT) ? AB2_INST : AB1_INST;
}
#endif

int16_t Encode_Calc_Delta(uint16_t now, uint16_t last)
{
    return (int16_t)(now - last);
}

#ifndef ENCODE_HOST_TEST
void Encode_Init(void)
{
    Encode_Clear(ENCODE_LEFT);
    Encode_Clear(ENCODE_RIGHT);
}

void Encode_Clear(Encode_Channel_t channel)
{
    GPTIMER_Regs *timer = Encode_Get_Timer(channel);

    DL_TimerG_stopCounter(timer);
    DL_TimerG_setTimerCount(timer, 0U);
    g_encode_last[channel] = 0U;
    DL_TimerG_startCounter(timer);
}

uint16_t Encode_Get_Count(Encode_Channel_t channel)
{
    return (uint16_t)(DL_TimerG_getTimerCount(Encode_Get_Timer(channel)) & ENCODE_COUNTER_MAX);
}

int16_t Encode_Get_Delta(Encode_Channel_t channel)
{
    uint16_t now = Encode_Get_Count(channel);
    int16_t delta = Encode_Calc_Delta(now, g_encode_last[channel]);

    g_encode_last[channel] = now;
    return delta;
}
#endif
