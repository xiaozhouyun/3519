#include "icm42688p.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ICM-42686-P USER BANK 0 寄存器。 */
#define ICM42688P_REG_ACCEL_DATA_X1 (0x1FU)
#define ICM42688P_REG_GYRO_DATA_X1  (0x25U)
#define ICM42688P_REG_WHO_AM_I      (0x75U)
#define ICM42688P_REG_PWR_MGMT0     (0x4EU)
#define ICM42688P_REG_GYRO_CONFIG0  (0x4FU)
#define ICM42688P_REG_ACCEL_CONFIG0 (0x50U)

#define ICM42688P_SPI_READ        (0x80U)
#define ICM42688P_TIMEOUT         (100000U)
#define ICM42688P_ACC_LSB_PER_G   (2048.0f) /* ±16 g */
#define ICM42688P_GYRO_LSB_PER_DPS (16.4f)  /* ±2000 dps */
#define ICM42688P_MAHONY_KP       (1.0f)

/* 保持与 IMU660RC 相同的 SPI 实例及 CS 引脚。 */
#define ICM42688P_CS_LOW()  DL_GPIO_clearPins(imuInt_PORT, imuInt_CS_PIN)
#define ICM42688P_CS_HIGH() DL_GPIO_setPins(imuInt_PORT, imuInt_CS_PIN)

int16_t icm42688p_acc_x;
int16_t icm42688p_acc_y;
int16_t icm42688p_acc_z;
int16_t icm42688p_gyro_x;
int16_t icm42688p_gyro_y;
int16_t icm42688p_gyro_z;
float icm42688p_roll;
float icm42688p_pitch;
float icm42688p_yaw;

static float q0 = 1.0f;
static float q1;
static float q2;
static float q3;

static uint8_t icm42688p_spi_wait_idle(void)
{
    uint32_t timeout = ICM42688P_TIMEOUT;

    while ((!DL_SPI_isTXFIFOEmpty(IMU660RC_INST) || DL_SPI_isBusy(IMU660RC_INST)) && --timeout) {
    }
    return (timeout != 0U) ? 0U : 1U;
}

static void icm42688p_spi_flush_rx(void)
{
    while (!DL_SPI_isRXFIFOEmpty(IMU660RC_INST)) {
        (void)DL_SPI_receiveData8(IMU660RC_INST);
    }
}

static uint8_t icm42688p_write_register(uint8_t reg, uint8_t value)
{
    uint32_t timeout = ICM42688P_TIMEOUT;

    ICM42688P_CS_LOW();
    icm42688p_spi_flush_rx();
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
    }
    if (timeout == 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }
    DL_SPI_transmitData8(IMU660RC_INST, reg);
    timeout = ICM42688P_TIMEOUT;
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
    }
    if (timeout == 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }
    DL_SPI_transmitData8(IMU660RC_INST, value);
    if (icm42688p_spi_wait_idle() != 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }
    icm42688p_spi_flush_rx();
    ICM42688P_CS_HIGH();
    return 0U;
}

static uint8_t icm42688p_read_registers(uint8_t reg, uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint32_t timeout = ICM42688P_TIMEOUT;

    ICM42688P_CS_LOW();
    icm42688p_spi_flush_rx();
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
    }
    if (timeout == 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }
    DL_SPI_transmitData8(IMU660RC_INST, reg | ICM42688P_SPI_READ);
    for (i = 0U; i < len; i++) {
        timeout = ICM42688P_TIMEOUT;
        while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --timeout) {
        }
        if (timeout == 0U) {
            ICM42688P_CS_HIGH();
            return 1U;
        }
        DL_SPI_transmitData8(IMU660RC_INST, 0x00U);
    }
    if (icm42688p_spi_wait_idle() != 0U) {
        ICM42688P_CS_HIGH();
        return 1U;
    }
    (void)DL_SPI_receiveData8(IMU660RC_INST);
    for (i = 0U; i < len; i++) {
        if (DL_SPI_isRXFIFOEmpty(IMU660RC_INST)) {
            ICM42688P_CS_HIGH();
            return 1U;
        }
        data[i] = (uint8_t)DL_SPI_receiveData8(IMU660RC_INST);
    }
    ICM42688P_CS_HIGH();
    return 0U;
}

uint8_t icm42688p_read_id(void)
{
    uint8_t id = 0U;

    (void)icm42688p_read_registers(ICM42688P_REG_WHO_AM_I, &id, 1U);
    return id;
}

uint8_t icm42688p_init(void)
{
    ICM42688P_CS_HIGH();
    delay_cycles(CPUCLK_FREQ / 1000U * 2U);
    if (icm42688p_read_id() != ICM42688P_WHO_AM_I_VALUE) {
        return 1U;
    }

    if (icm42688p_write_register(ICM42688P_REG_PWR_MGMT0, 0x00U) != 0U ||
        icm42688p_write_register(ICM42688P_REG_GYRO_CONFIG0, 0x06U) != 0U ||
        icm42688p_write_register(ICM42688P_REG_ACCEL_CONFIG0, 0x06U) != 0U ||
        icm42688p_write_register(ICM42688P_REG_PWR_MGMT0, 0x0FU) != 0U) {
        return 1U;
    }
    delay_cycles(CPUCLK_FREQ / 20U);
    return 0U;
}

static void icm42688p_mahony_update(float gx, float gy, float gz, float ax, float ay, float az, float dt)
{
    float norm;
    float half_vx;
    float half_vy;
    float half_vz;
    float half_ex;
    float half_ey;
    float half_ez;
    float half_q0;
    float half_q1;
    float half_q2;
    float half_q3;

    norm = fast_sqrtf(ax * ax + ay * ay + az * az);
    if (norm > 0.0001f) {
        ax /= norm;
        ay /= norm;
        az /= norm;
        half_vx = q1 * q3 - q0 * q2;
        half_vy = q0 * q1 + q2 * q3;
        half_vz = q0 * q0 - 0.5f + q3 * q3;
        half_ex = ay * half_vz - az * half_vy;
        half_ey = az * half_vx - ax * half_vz;
        half_ez = ax * half_vy - ay * half_vx;
        gx += ICM42688P_MAHONY_KP * half_ex;
        gy += ICM42688P_MAHONY_KP * half_ey;
        gz += ICM42688P_MAHONY_KP * half_ez;
    }

    half_q0 = 0.5f * q0;
    half_q1 = 0.5f * q1;
    half_q2 = 0.5f * q2;
    half_q3 = 0.5f * q3;
    q0 += (-half_q1 * gx - half_q2 * gy - half_q3 * gz) * dt;
    q1 += (half_q0 * gx + half_q2 * gz - half_q3 * gy) * dt;
    q2 += (half_q0 * gy - half_q1 * gz + half_q3 * gx) * dt;
    q3 += (half_q0 * gz + half_q1 * gy - half_q2 * gx) * dt;

    norm = fast_sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    if (norm > 0.0001f) {
        q0 /= norm;
        q1 /= norm;
        q2 /= norm;
        q3 /= norm;
    }
}

void icm42688p_update(float dt)
{
    uint8_t data[12];
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;

    if (dt <= 0.0f || icm42688p_read_registers(ICM42688P_REG_ACCEL_DATA_X1, data, sizeof(data)) != 0U) {
        return;
    }

    icm42688p_acc_x = (int16_t)(((uint16_t)data[0] << 8) | data[1]);
    icm42688p_acc_y = (int16_t)(((uint16_t)data[2] << 8) | data[3]);
    icm42688p_acc_z = (int16_t)(((uint16_t)data[4] << 8) | data[5]);
    icm42688p_gyro_x = (int16_t)(((uint16_t)data[6] << 8) | data[7]);
    icm42688p_gyro_y = (int16_t)(((uint16_t)data[8] << 8) | data[9]);
    icm42688p_gyro_z = (int16_t)(((uint16_t)data[10] << 8) | data[11]);

    ax = (float)icm42688p_acc_x / ICM42688P_ACC_LSB_PER_G;
    ay = (float)icm42688p_acc_y / ICM42688P_ACC_LSB_PER_G;
    az = (float)icm42688p_acc_z / ICM42688P_ACC_LSB_PER_G;
    gx = (float)icm42688p_gyro_x / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gy = (float)icm42688p_gyro_y / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gz = (float)icm42688p_gyro_z / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);

    icm42688p_mahony_update(gx, gy, gz, ax, ay, az, dt);
    icm42688p_roll = fast_atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * (180.0f / M_PI);
    {
        float sin_pitch = 2.0f * (q0 * q2 - q3 * q1);

        if (sin_pitch > 1.0f) {
            sin_pitch = 1.0f;
        } else if (sin_pitch < -1.0f) {
            sin_pitch = -1.0f;
        }
        icm42688p_pitch = fast_atan2f(sin_pitch, fast_sqrtf(1.0f - sin_pitch * sin_pitch)) * (180.0f / M_PI);
    }
    icm42688p_yaw = fast_atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * (180.0f / M_PI);
}
