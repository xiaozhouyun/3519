/**
 * @file Grayscale.c
 * @brief MSPM0G3519 感为八通道灰度循迹传感器串行输出驱动实现
 */

#include "../inc/Grayscale.h"
#include "blue.h"
#include "main.h"

#define GRAYSCALE_SERIAL_DELAY_CYCLES   (CPUCLK_FREQ / 200000U)

static uint8_t Grayscale_Serial_Read(void)
{
    uint8_t data = 0U;
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        DL_GPIO_clearPins(graySerial_PORT, graySerial_CLK_PIN);
        DL_Common_delayCycles(GRAYSCALE_SERIAL_DELAY_CYCLES);
        if (DL_GPIO_readPins(graySerial_PORT, graySerial_DAT_PIN) != 0U) {
            data |= (uint8_t)(1U << i);
        }
        DL_GPIO_setPins(graySerial_PORT, graySerial_CLK_PIN);
        DL_Common_delayCycles(GRAYSCALE_SERIAL_DELAY_CYCLES);
    }

    return data;
}

void Grayscale_Update(Grayscale_Sensor_t *sensor)
{
    uint8_t digital;
    uint8_t i;
    uint8_t black_count = 0U;

    digital = Grayscale_Serial_Read();

    sensor->digital = 0U;
    for (i = 0U; i < 8U; i++) {
        if ((digital & (1U << i)) != 0U) {
            sensor->digital |= (uint8_t)(1U << (7U - i));
        }
    }
    sensor->is_ok = 1U;

    // 统计扫描到黑线 (位值为 0) 的通道数量
    for (i = 0U; i < 8U; i++) {
        if ((sensor->digital & (1U << i)) == 0U) {
            black_count++;
        }
    }

    // 当扫到超过 6 条黑线 (≥ 7 条) 且启动已满 5 秒以上时，清零运行标志位并停止小车
    if (black_count > 5U) {
        if (g_system_timer_sec >= 15U) {
            g_bt_running_flag = 0U;
            BT_Stop();
        }
    }
}

uint8_t Grayscale_Get_Digital(Grayscale_Sensor_t *sensor)
{
    if (sensor->is_ok == 0U) {
        return 0U;
    }
    return sensor->digital;
}
