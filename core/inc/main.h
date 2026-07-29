#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

/**
 * @brief 全局系统计时器 (秒)，由 main.c 100Hz 任务更新
 */
extern volatile uint32_t g_system_timer_sec;

#endif /* MAIN_H */
