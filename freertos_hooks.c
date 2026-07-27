/*
 * FreeRTOS hook functions for MSPM0G3519 (ARMCLANG)
 * Provides required callbacks for static allocation and stack overflow checking.
 */
#include "FreeRTOS.h"
#include "task.h"

/* configCHECK_FOR_STACK_OVERFLOW == 2, must provide this hook */
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    /* Spin forever on stack overflow - inspect in debugger */
    while (1) {}
}
#endif

/* configSUPPORT_STATIC_ALLOCATION == 1, must provide idle task memory */
#if (configSUPPORT_STATIC_ALLOCATION == 1)
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   configSTACK_DEPTH_TYPE *pulIdleTaskStackSize)
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configIDLE_TASK_STACK_DEPTH];

    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize   = configIDLE_TASK_STACK_DEPTH;
}

    #if (configUSE_TIMERS == 1)
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE *pulTimerTaskStackSize)
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}
    #endif /* configUSE_TIMERS */
#endif /* configSUPPORT_STATIC_ALLOCATION */
