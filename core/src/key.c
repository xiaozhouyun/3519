/**
 * @file key.c
 * @brief 五个按键的中断事件记录。
 */

#include "key.h"
#include "main.h"
#include "blue.h"
#include "ti_msp_dl_config.h"
#include "FreeRTOS.h"
#include "task.h"

volatile uint8_t g_key_event = 0U;
volatile uint8_t g_key_double_click_pending = 0U;

static TickType_t s_key_last_edge_tick[4];
static TickType_t s_key_first_click_tick[4];

static uint8_t Key_Is_Double_Click(uint8_t key_event, uint8_t key_index,
                                   TickType_t now)
{
    if ((TickType_t)(now - s_key_last_edge_tick[key_index]) <
        pdMS_TO_TICKS(KEY_DEBOUNCE_MS)) {
        return 0U;
    }

    s_key_last_edge_tick[key_index] = now;
    if (((g_key_double_click_pending & key_event) != 0U) &&
        ((TickType_t)(now - s_key_first_click_tick[key_index]) <=
         pdMS_TO_TICKS(KEY_DOUBLE_CLICK_MS))) {
        g_key_double_click_pending &= (uint8_t)(~key_event);
        return 1U;
    }

    g_key_double_click_pending |= key_event;
    s_key_first_click_tick[key_index] = now;
    return 0U;
}

static void Key_Handle_Question(uint8_t key_event, uint8_t key_index,
                                uint8_t question, TickType_t now)
{
    g_key_event |= key_event;
    if ((Key_Is_Double_Click(key_event, key_index, now) != 0U) &&
        (FollowLine_Select_Question(question) != false)) {
        BT_Start();
    }
}

void Key_Init(void)
{
    g_key_event = 0U;
    g_key_double_click_pending = 0U;
    for (uint8_t i = 0U; i < 4U; i++) {
        s_key_last_edge_tick[i] = (TickType_t)(-pdMS_TO_TICKS(KEY_DEBOUNCE_MS));
        s_key_first_click_tick[i] = 0U;
    }
    NVIC_ClearPendingIRQ(key_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(key_GPIOB_INT_IRQN);
    NVIC_ClearPendingIRQ(key_GPIOC_INT_IRQN);
    NVIC_EnableIRQ(key_GPIOC_INT_IRQN);
}

void GROUP1_IRQHandler(void)
{
    TickType_t now = xTaskGetTickCountFromISR();
    uint32_t status_b = DL_GPIO_getEnabledInterruptStatus(GPIOB,
                    key_key0_PIN | key_key1_PIN | key_user_key_PIN);
    uint32_t status_c = DL_GPIO_getEnabledInterruptStatus(GPIOC,
                    key_key2_PIN | key_key3_PIN);

    if ((status_b & key_key0_PIN) != 0U) {
        DL_GPIO_clearInterruptStatus(GPIOB, key_key0_PIN);
        Key_Handle_Question(KEY_EVENT_KEY0, 0U, 2U, now);
    }
    if ((status_b & key_key1_PIN) != 0U) {
        DL_GPIO_clearInterruptStatus(GPIOB, key_key1_PIN);
        Key_Handle_Question(KEY_EVENT_KEY1, 1U, 3U, now);
    }
    if ((status_c & key_key2_PIN) != 0U) {
        DL_GPIO_clearInterruptStatus(GPIOC, key_key2_PIN);
        Key_Handle_Question(KEY_EVENT_KEY2, 2U, 4U, now);
    }
    if ((status_c & key_key3_PIN) != 0U) {
        DL_GPIO_clearInterruptStatus(GPIOC, key_key3_PIN);
        Key_Handle_Question(KEY_EVENT_KEY3, 3U, 5U, now);
    }
    if ((status_b & key_user_key_PIN) != 0U) {
        DL_GPIO_clearInterruptStatus(GPIOB, key_user_key_PIN);
        g_key_event |= KEY_EVENT_USER;
    }
}
