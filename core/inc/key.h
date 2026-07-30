#ifndef KEY_H
#define KEY_H

#include <stdint.h>

#define KEY_EVENT_KEY0       (1U << 0)
#define KEY_EVENT_KEY1       (1U << 1)
#define KEY_EVENT_KEY2       (1U << 2)
#define KEY_EVENT_KEY3       (1U << 3)
#define KEY_EVENT_USER       (1U << 4)

#define KEY_DOUBLE_CLICK_MS  (300U)
#define KEY_DEBOUNCE_MS      (30U)

/**
 * @brief 启用按键所在 Group1 的 NVIC 中断。
 */
void Key_Init(void);

#endif /* KEY_H */
