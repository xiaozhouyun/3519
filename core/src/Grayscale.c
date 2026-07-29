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

void Grayscale_Init_First(Grayscale_Sensor_t *sensor)
{
    uint16_t default_white[8];
    uint16_t default_black[8];
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        default_white[i] = GRAYSCALE_DEFAULT_WHITE;
        default_black[i] = GRAYSCALE_DEFAULT_BLACK;
    }

    Grayscale_Init(sensor, default_white, default_black);
}

void Grayscale_Init(Grayscale_Sensor_t *sensor, const uint16_t *calibrated_white,
    const uint16_t *calibrated_black)
{
    uint8_t i;

    memset(sensor, 0, sizeof(Grayscale_Sensor_t));

    for (i = 0U; i < 8U; i++) {
        uint16_t white = calibrated_white[i];
        uint16_t black = calibrated_black[i];

        if (black >= white) {
            uint16_t temp = white;
            white = black;
            black = temp;
        }

        sensor->calibrated_white[i] = white;
        sensor->calibrated_black[i] = black;
        sensor->gray_white[i] = (uint16_t)((white * 2U + black) / 3U);
        sensor->gray_black[i] = (uint16_t)((white + black * 2U) / 3U);
        sensor->normal_factor[i] = (white > black) ? (4095.0 / (double)(white - black)) : 0.0;
    }
}

void Grayscale_Set_Global_Thresholds(Grayscale_Sensor_t *sensor, uint16_t white, uint16_t black)
{
    uint16_t white_arr[8];
    uint16_t black_arr[8];
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        white_arr[i] = white;
        black_arr[i] = black;
    }

    Grayscale_Init(sensor, white_arr, black_arr);
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
    if (black_count > 6U) {
        if (g_system_timer_sec >= 5U) {
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

uint8_t Grayscale_Get_Normalized(Grayscale_Sensor_t *sensor, uint16_t *result)
{
    if (sensor->is_ok == 0U) {
        return 0U;
    }
    memcpy(result, sensor->normal_val, sizeof(sensor->normal_val));
    return 1U;
}

uint8_t Grayscale_Get_Analog(Grayscale_Sensor_t *sensor, uint16_t *result)
{
    if (sensor->is_ok == 0U) {
        return 0U;
    }
    memcpy(result, sensor->analog_val, sizeof(sensor->analog_val));
    return 1U;
}
