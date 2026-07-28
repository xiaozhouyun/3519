#include "icm42688p.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ICM-42686-P USER BANK 0 寄存器。 */
#define ICM42688P_REG_DEVICE_CONFIG     (0x11U)
#define ICM42688P_REG_ACCEL_DATA_X1     (0x1FU)
#define ICM42688P_REG_GYRO_DATA_X1      (0x25U)
#define ICM42688P_REG_PWR_MGMT0         (0x4EU)
#define ICM42688P_REG_GYRO_CONFIG0      (0x4FU)
#define ICM42688P_REG_ACCEL_CONFIG0     (0x50U)
#define ICM42688P_REG_GYRO_ACCEL_CONFIG0 (0x52U)
#define ICM42688P_REG_WHO_AM_I          (0x75U)

#define ICM42688P_SPI_READ     (0x80U)
#define ICM42688P_TIMEOUT      (100000U)
#define ICM42688P_MAHONY_KP    (0.3f)   /* 降低 Kp 减少 yaw 耦合漂移 */

/* 灵敏度: 默认 ±16g / ±2000dps */
#define ICM42688P_ACC_LSB_PER_G    (2048.0f)
#define ICM42688P_GYRO_LSB_PER_DPS (16.4f)

/* ================================================================
 *  SPI 低位驱动
 * ================================================================ */
#define CS_LOW()   DL_GPIO_clearPins(imuInt_PORT, imuInt_CS_PIN)
#define CS_HIGH()  DL_GPIO_setPins(imuInt_PORT, imuInt_CS_PIN)

static void spi_flush_rx(void)
{
    while (!DL_SPI_isRXFIFOEmpty(IMU660RC_INST))
        (void)DL_SPI_receiveData8(IMU660RC_INST);
}

static uint8_t spi_wait_done(void)
{
    uint32_t to = ICM42688P_TIMEOUT;
    while ((!DL_SPI_isTXFIFOEmpty(IMU660RC_INST) || DL_SPI_isBusy(IMU660RC_INST)) && --to) {}
    return (to != 0U) ? 0U : 1U;
}

static uint8_t reg_write(uint8_t reg, uint8_t val)
{
    uint32_t to = ICM42688P_TIMEOUT;
    CS_LOW(); spi_flush_rx();

    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --to) {}
    if (!to) { CS_HIGH(); return 1U; }
    DL_SPI_transmitData8(IMU660RC_INST, reg);

    to = ICM42688P_TIMEOUT;
    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --to) {}
    if (!to) { CS_HIGH(); return 1U; }
    DL_SPI_transmitData8(IMU660RC_INST, val);

    if (spi_wait_done()) { CS_HIGH(); return 1U; }
    spi_flush_rx();
    CS_HIGH();
    return 0U;
}

static uint8_t reg_read(uint8_t reg, uint8_t *buf, uint32_t n)
{
    uint32_t i, to = ICM42688P_TIMEOUT;

    CS_LOW(); spi_flush_rx();

    while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --to) {}
    if (!to) { CS_HIGH(); return 1U; }
    DL_SPI_transmitData8(IMU660RC_INST, reg | ICM42688P_SPI_READ);

    for (i = 0U; i < n; i++) {
        to = ICM42688P_TIMEOUT;
        while (DL_SPI_isTXFIFOFull(IMU660RC_INST) && --to) {}
        if (!to) { CS_HIGH(); return 1U; }
        DL_SPI_transmitData8(IMU660RC_INST, 0x00U);
    }
    if (spi_wait_done()) { CS_HIGH(); return 1U; }

    (void)DL_SPI_receiveData8(IMU660RC_INST); /* 丢弃 dummy */

    for (i = 0U; i < n; i++) {
        if (DL_SPI_isRXFIFOEmpty(IMU660RC_INST)) { CS_HIGH(); return 1U; }
        buf[i] = (uint8_t)DL_SPI_receiveData8(IMU660RC_INST);
    }
    CS_HIGH();
    return 0U;
}

/* 安全读取: 分多次每次 ≤3 字节，避免 MSPM0 4 深 FIFO 溢出 */
static uint8_t reg_read_safe(uint8_t reg, uint8_t *buf, uint32_t total)
{
    uint32_t off = 0U;
    while (off < total) {
        uint32_t n = total - off;
        if (n > 3U) n = 3U;
        if (reg_read((uint8_t)(reg + off), buf + off, n)) return 1U;
        off += n;
    }
    return 0U;
}

/* ================================================================
 *  全局数据
 * ================================================================ */
int16_t icm42688p_acc_x,  icm42688p_acc_y,  icm42688p_acc_z;
int16_t icm42688p_gyro_x, icm42688p_gyro_y, icm42688p_gyro_z;
float   icm42688p_roll,  icm42688p_pitch,  icm42688p_yaw;

static float q0 = 1.0f, q1, q2, q3;
static float g_bias_x, g_bias_y, g_bias_z;  /* 陀螺零偏 */

/* ================================================================
 *  陀螺零偏校准 (开机静止采样 200 次 ≈ 1s)
 * ================================================================ */
static void gyro_calibrate(void)
{
    uint8_t d[6];
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    int i;

    for (i = 0; i < 200; i++) {
        if (reg_read_safe(ICM42688P_REG_GYRO_DATA_X1, d, 6U) == 0U) {
            sum_x += (int16_t)(((uint16_t)d[0] << 8) | d[1]);
            sum_y += (int16_t)(((uint16_t)d[2] << 8) | d[3]);
            sum_z += (int16_t)(((uint16_t)d[4] << 8) | d[5]);
        }
        delay_cycles(CPUCLK_FREQ / 200U); /* ~5ms */
    }
    g_bias_x = (float)sum_x / 200.0f;
    g_bias_y = (float)sum_y / 200.0f;
    g_bias_z = (float)sum_z / 200.0f;
}

/* ================================================================
 *  初始化 (参照科宇 ICM42686 驱动)
 * ================================================================ */

uint8_t icm42688p_read_id(void)
{
    uint8_t id = 0U;
    reg_read(ICM42688P_REG_WHO_AM_I, &id, 1U);
    return id;
}

uint8_t icm42688p_init(void)
{
    uint8_t id;
    int retry;

    CS_HIGH();
    delay_cycles(CPUCLK_FREQ / 1000U * 2U);

    /* 1. 软复位 */
    reg_write(ICM42688P_REG_DEVICE_CONFIG, 0x01U);
    delay_cycles(CPUCLK_FREQ / 100U); /* ~10ms */

    /* 2. WHO_AM_I 检查 (循环重试，参照科宇驱动) */
    for (retry = 0; retry < 20; retry++) {
        reg_read(ICM42688P_REG_WHO_AM_I, &id, 1U);
        if (id == 0x44U) break;
        delay_cycles(CPUCLK_FREQ / 100U); /* ~10ms */
    }
    if (id != 0x44U) return 1U;

    /* 3. 使能传感器 (LN 模式) */
    reg_write(ICM42688P_REG_PWR_MGMT0, 0x0FU);
    delay_cycles(CPUCLK_FREQ / 20U); /* ~50ms */

    /*
     * 4. 加速度计: ±16g (FS=1 << 5) | ODR=1kHz (6)
     *    陀螺仪:   ±2000dps (FS=0 << 5) | ODR=1kHz (6)
     *    位布局 (参照科宇): bits[7:5]=FS, bits[3:0]=ODR
     */
    reg_write(ICM42688P_REG_ACCEL_CONFIG0, 0x26U); /* (1<<5)|6 */
    reg_write(ICM42688P_REG_GYRO_CONFIG0,  0x06U); /* (0<<5)|6 */

    /* 5. UI 滤波器带宽 (参照科宇: GYRO_ACCEL_CONFIG0 = 0x11) */
    reg_write(ICM42688P_REG_GYRO_ACCEL_CONFIG0, 0x11U);

    delay_cycles(CPUCLK_FREQ / 100U); /* ~10ms */

    /* 6. 陀螺零偏校准 (开机保持静止) */
    gyro_calibrate();

    return 0U;
}

/* ================================================================
 *  Mahony AHRS
 * ================================================================ */
static void mahony(float gx, float gy, float gz,
                   float ax, float ay, float az, float dt)
{
    float n, hx, hy, hz, ex, ey, ez, hq0, hq1, hq2, hq3;

    n = fast_sqrtf(ax * ax + ay * ay + az * az);
    if (n > 0.0001f) {
        ax /= n; ay /= n; az /= n;
        hx = q1 * q3 - q0 * q2;
        hy = q0 * q1 + q2 * q3;
        hz = q0 * q0 - 0.5f + q3 * q3;
        ex = ay * hz - az * hy;
        ey = az * hx - ax * hz;
        ez = ax * hy - ay * hx;
        gx += ICM42688P_MAHONY_KP * ex;
        gy += ICM42688P_MAHONY_KP * ey;
        gz += ICM42688P_MAHONY_KP * ez;
    }
    hq0 = 0.5f * q0; hq1 = 0.5f * q1;
    hq2 = 0.5f * q2; hq3 = 0.5f * q3;
    q0 += (-hq1 * gx - hq2 * gy - hq3 * gz) * dt;
    q1 += ( hq0 * gx + hq2 * gz - hq3 * gy) * dt;
    q2 += ( hq0 * gy - hq1 * gz + hq3 * gx) * dt;
    q3 += ( hq0 * gz + hq1 * gy - hq2 * gx) * dt;
    n = fast_sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    if (n > 0.0001f) { q0 /= n; q1 /= n; q2 /= n; q3 /= n; }
}

/* ================================================================
 *  周期更新 (参照科宇: 分开读加速度和陀螺)
 * ================================================================ */
void icm42688p_update(float dt)
{
    uint8_t d[6];
    float ax, ay, az, gx, gy, gz, sp;

    if (dt <= 0.0f) return;

    /* 读加速度 6 字节 (分 2 次 × 3) */
    if (reg_read_safe(ICM42688P_REG_ACCEL_DATA_X1, d, 6U)) return;
    icm42688p_acc_x = (int16_t)(((uint16_t)d[0] << 8) | d[1]);
    icm42688p_acc_y = (int16_t)(((uint16_t)d[2] << 8) | d[3]);
    icm42688p_acc_z = (int16_t)(((uint16_t)d[4] << 8) | d[5]);

    /* 读陀螺 6 字节 (分 2 次 × 3) */
    if (reg_read_safe(ICM42688P_REG_GYRO_DATA_X1, d, 6U)) return;
    icm42688p_gyro_x = (int16_t)(((uint16_t)d[0] << 8) | d[1]);
    icm42688p_gyro_y = (int16_t)(((uint16_t)d[2] << 8) | d[3]);
    icm42688p_gyro_z = (int16_t)(((uint16_t)d[4] << 8) | d[5]);

    /* 换算为物理单位 并 减去零偏 */
    ax = (float)icm42688p_acc_x  / ICM42688P_ACC_LSB_PER_G;
    ay = (float)icm42688p_acc_y  / ICM42688P_ACC_LSB_PER_G;
    az = (float)icm42688p_acc_z  / ICM42688P_ACC_LSB_PER_G;
    gx = ((float)icm42688p_gyro_x - g_bias_x) / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gy = ((float)icm42688p_gyro_y - g_bias_y) / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gz = ((float)icm42688p_gyro_z - g_bias_z) / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);

    mahony(gx, gy, gz, ax, ay, az, dt);

    icm42688p_roll  = fast_atan2f(2.0f * (q0 * q1 + q2 * q3),
                                  1.0f - 2.0f * (q1 * q1 + q2 * q2)) * (180.0f / M_PI);
    sp = 2.0f * (q0 * q2 - q3 * q1);
    if (sp > 1.0f) sp = 1.0f; else if (sp < -1.0f) sp = -1.0f;
    icm42688p_pitch = fast_atan2f(sp, fast_sqrtf(1.0f - sp * sp)) * (180.0f / M_PI);
    icm42688p_yaw   = fast_atan2f(2.0f * (q0 * q3 + q1 * q2),
                                  1.0f - 2.0f * (q2 * q2 + q3 * q3)) * (180.0f / M_PI);
}
