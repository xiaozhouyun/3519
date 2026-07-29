/**
 * @file Grayscale.c
 * @brief MSPM0G3519 感为八通道灰度循迹传感器 I2C 驱动实现
 */

#include "../inc/Grayscale.h"

#define GRAYSCALE_I2C_ADDR             0x4EU
#define GRAYSCALE_CMD_DIGITAL          0xDDU
#define GRAYSCALE_CMD_ANALOG_ALL       0xB0U
#define GRAYSCALE_I2C_TIMEOUT          100000U
uint32_t status;
/* 保持旧上层接口约定：物理 1 路映射到索引 7，物理 8 路映射到索引 0。 */
static uint8_t Grayscale_Map_Channel(uint8_t channel)
{
    return (uint8_t)(7U - channel);
}

static uint8_t Grayscale_I2C_Wait_Idle(void)
{
    uint32_t timeout = GRAYSCALE_I2C_TIMEOUT;

    while (timeout-- > 0U) {
         status = DL_I2C_getControllerStatus(GRAYSCALE_INST);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
            DL_I2C_resetControllerTransfer(GRAYSCALE_INST);
            return 0U;
        }
        if ((status & DL_I2C_CONTROLLER_STATUS_IDLE) != 0U) {
            return 1U;
        }
    }
    return 0U;
}

static uint8_t Grayscale_I2C_Write(const uint8_t *data, uint8_t length)
{
    uint32_t timeout = GRAYSCALE_I2C_TIMEOUT;

    if ((length == 0U) || (Grayscale_I2C_Wait_Idle() == 0U)) {
        return 0U;
    }

    DL_I2C_flushControllerTXFIFO(GRAYSCALE_INST);
    if (DL_I2C_fillControllerTXFIFO(GRAYSCALE_INST, data, length) != (uint32_t)length) {
        return 0U;
    }

    DL_I2C_startControllerTransfer(GRAYSCALE_INST, GRAYSCALE_I2C_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_TX, length);

    while (timeout-- > 0U) {
        uint32_t status = DL_I2C_getControllerStatus(GRAYSCALE_INST);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
            DL_I2C_resetControllerTransfer(GRAYSCALE_INST);
            return 0U;
        }
        if ((status & DL_I2C_CONTROLLER_STATUS_BUSY) == 0U) {
            return 1U;
        }
    }
    return 0U; /* 超时，返回失败 */
}

static uint8_t Grayscale_I2C_Read(uint8_t *data, uint8_t length)
{
    uint8_t i;

    if ((length == 0U) || (Grayscale_I2C_Wait_Idle() == 0U)) {
        return 0U;
    }

    DL_I2C_flushControllerRXFIFO(GRAYSCALE_INST);
    DL_I2C_startControllerTransfer(GRAYSCALE_INST, GRAYSCALE_I2C_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_RX, length);

    for (i = 0U; i < length; i++) {
        uint32_t timeout = GRAYSCALE_I2C_TIMEOUT;
        while (DL_I2C_isControllerRXFIFOEmpty(GRAYSCALE_INST)) {
            uint32_t status = DL_I2C_getControllerStatus(GRAYSCALE_INST);
            if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
                DL_I2C_resetControllerTransfer(GRAYSCALE_INST);
                return 0U;
            }
            if (--timeout == 0U) {
                return 0U;
            }
        }
        data[i] = DL_I2C_receiveControllerData(GRAYSCALE_INST);
    }

    return Grayscale_I2C_Wait_Idle();
}

static uint8_t Grayscale_I2C_Read_Command(uint8_t command, uint8_t *data, uint8_t length)
{
    return (uint8_t)((Grayscale_I2C_Write(&command, 1U) != 0U) &&
        (Grayscale_I2C_Read(data, length) != 0U));
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
    uint8_t analog[8];
    uint8_t i;

    if ((Grayscale_I2C_Read_Command(GRAYSCALE_CMD_DIGITAL, &digital, 1U) == 0U) ||
        (Grayscale_I2C_Read_Command(GRAYSCALE_CMD_ANALOG_ALL, analog, 8U) == 0U)) {
        sensor->is_ok = 0U;
        return;
    }

    sensor->digital = 0U;
    for (i = 0U; i < 8U; i++) {
        uint8_t mapped = Grayscale_Map_Channel(i);
         sensor->analog_val[mapped] = (uint16_t)analog[i] << 4;
         sensor->normal_val[mapped] = sensor->analog_val[mapped];
        if ((digital & (1U << i)) != 0U) {
            sensor->digital |= (uint8_t)(1U << mapped);
        }
    }
    sensor->is_ok = 1U;
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
