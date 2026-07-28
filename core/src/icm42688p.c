#include "icm42688p.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* =========================================================================
 * ICM-42688-P 寄存器宏定义 (Bank 0)
 * ========================================================================= */
#define ICM42688P_REG_DEVICE_CONFIG     (0x11U)  /* 设备配置寄存器 (软复位) */
#define ICM42688P_REG_ACCEL_DATA_X1     (0x1FU)  /* 加速度计 X轴高字节数据寄存器地址 */
#define ICM42688P_REG_GYRO_DATA_X1      (0x25U)  /* 陀螺仪 X轴高字节数据寄存器地址 */
#define ICM42688P_REG_PWR_MGMT0         (0x4EU)  /* 电源管理寄存器 0 (模式切换) */
#define ICM42688P_REG_GYRO_CONFIG0      (0x4FU)  /* 陀螺仪配置寄存器 0 (量程与 ODR) */
#define ICM42688P_REG_ACCEL_CONFIG0     (0x50U)  /* 加速度计配置寄存器 0 (量程与 ODR) */
#define ICM42688P_REG_GYRO_ACCEL_CONFIG0 (0x52U) /* 抗混叠与 UI 滤波器带宽配置 */
#define ICM42688P_REG_WHO_AM_I          (0x75U)  /* 设备 ID 寄存器地址 (应返回 0x44) */

/* =========================================================================
 * SPI 读写标志与传感器参数宏定义
 * ========================================================================= */
#define ICM42688P_SPI_READ     (0x80U)    /* SPI 读操作最高有效位置 1 */
#define ICM42688P_TIMEOUT      (100000U)  /* SPI 读写超时等待循环计数 */
#define ICM42688P_MAHONY_KP    (0.3f)     /* 降低 Kp 减少 yaw 耦合漂移 */

/* 灵敏度转换系数: 默认 ±16g / ±2000dps */
#define ICM42688P_ACC_LSB_PER_G    (2048.0f)  /* ±16g 量程对应 2048 LSB/g */
#define ICM42688P_GYRO_LSB_PER_DPS (16.4f)   /* ±2000dps 量程对应 16.4 LSB/(°/s) */

/* =========================================================================
 * SPI 底层宏控制
 * ========================================================================= */
#define CS_LOW()   DL_GPIO_clearPins(imuInt_PORT, imuInt_CS_PIN)  /* 片选拉低: 选中芯片 */
#define CS_HIGH()  DL_GPIO_setPins(imuInt_PORT, imuInt_CS_PIN)   /* 片选拉高: 释放芯片 */

/**
 * @brief 清空 SPI 接收 FIFO 中的残留数据
 */
static void spi_flush_rx(void)
{
    while (!DL_SPI_isRXFIFOEmpty(IMU660RC_INST))
        (void)DL_SPI_receiveData8(IMU660RC_INST);
}

/**
 * @brief 等待 SPI 传输空闲
 * @return 0: 成功; 1: 超时
 */
static uint8_t spi_wait_done(void)
{
    uint32_t to = ICM42688P_TIMEOUT;
    while ((!DL_SPI_isTXFIFOEmpty(IMU660RC_INST) || DL_SPI_isBusy(IMU660RC_INST)) && --to) {}
    return (to != 0U) ? 0U : 1U;
}

/**
 * @brief 向指定寄存器写入单字节
 */
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

/**
 * @brief 连续读取指定寄存器
 */
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

    (void)DL_SPI_receiveData8(IMU660RC_INST); /* 丢弃 dummy 字节 */

    for (i = 0U; i < n; i++) {
        if (DL_SPI_isRXFIFOEmpty(IMU660RC_INST)) { CS_HIGH(); return 1U; }
        buf[i] = (uint8_t)DL_SPI_receiveData8(IMU660RC_INST);
    }
    CS_HIGH();
    return 0U;
}

/**
 * @brief 安全分块读取 (每次最多 3 字节)，避免 MSPM0 4 深度 FIFO 溢出
 */
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

/* =========================================================================
 * 全局数据与姿态状态
 * ========================================================================= */
int16_t icm42688p_acc_x,  icm42688p_acc_y,  icm42688p_acc_z;   /* 加速度计原始 ADC 值 */
int16_t icm42688p_gyro_x, icm42688p_gyro_y, icm42688p_gyro_z;  /* 陀螺仪原始 ADC 值 */
float   icm42688p_roll,  icm42688p_pitch,  icm42688p_yaw;      /* 姿态角 (单位: 度) */

static float q0 = 1.0f, q1, q2, q3;                            /* 四元数 */
static float g_bias_x, g_bias_y, g_bias_z;                     /* 陀螺仪零偏 */

/**
 * @brief 陀螺零偏校准 (开机保持静止采样 200 次 ≈ 1s)
 */
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

/* =========================================================================
 * 传感器驱动公有接口
 * ========================================================================= */

/**
 * @brief 读取设备 WHO_AM_I ID
 */
uint8_t icm42688p_read_id(void)
{
    uint8_t id = 0U;
    reg_read(ICM42688P_REG_WHO_AM_I, &id, 1U);
    return id;
}

/**
 * @brief 初始化传感器并进行零偏校准
 */
uint8_t icm42688p_init(void)
{
    uint8_t id;
    int retry;

    CS_HIGH();
    delay_cycles(CPUCLK_FREQ / 1000U * 2U);

    /* 1. 软复位 */
    reg_write(ICM42688P_REG_DEVICE_CONFIG, 0x01U);
    delay_cycles(CPUCLK_FREQ / 100U); /* ~10ms */

    /* 2. WHO_AM_I 检查 (循环重试) */
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
     */
    reg_write(ICM42688P_REG_ACCEL_CONFIG0, 0x26U); /* (1<<5)|6 */
    reg_write(ICM42688P_REG_GYRO_CONFIG0,  0x06U); /* (0<<5)|6 */

    /* 5. UI 滤波器带宽配置 */
    reg_write(ICM42688P_REG_GYRO_ACCEL_CONFIG0, 0x11U);

    delay_cycles(CPUCLK_FREQ / 100U); /* ~10ms */

    /* 6. 自动执行陀螺零偏校准 (开机保持静止) */
    gyro_calibrate();

    return 0U;
}

/* =========================================================================
 * Mahony 姿态解算
 * ========================================================================= */
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

/* =========================================================================
 * 周期更新函数
 * ========================================================================= */
void icm42688p_update(float dt)
{
    uint8_t d[6];
    float ax, ay, az, gx, gy, gz, sp;

    if (dt <= 0.0f) return;

    /* 1. 分步安全读取加速度数据 (6 字节) */
    if (reg_read_safe(ICM42688P_REG_ACCEL_DATA_X1, d, 6U)) return;
    icm42688p_acc_x = (int16_t)(((uint16_t)d[0] << 8) | d[1]);
    icm42688p_acc_y = (int16_t)(((uint16_t)d[2] << 8) | d[3]);
    icm42688p_acc_z = (int16_t)(((uint16_t)d[4] << 8) | d[5]);

    /* 2. 分步安全读取陀螺仪数据 (6 字节) */
    if (reg_read_safe(ICM42688P_REG_GYRO_DATA_X1, d, 6U)) return;
    icm42688p_gyro_x = (int16_t)(((uint16_t)d[0] << 8) | d[1]);
    icm42688p_gyro_y = (int16_t)(((uint16_t)d[2] << 8) | d[3]);
    icm42688p_gyro_z = (int16_t)(((uint16_t)d[4] << 8) | d[5]);

    /* 3. 单位转换并扣除陀螺仪零偏 */
    ax = (float)icm42688p_acc_x  / ICM42688P_ACC_LSB_PER_G;
    ay = (float)icm42688p_acc_y  / ICM42688P_ACC_LSB_PER_G;
    az = (float)icm42688p_acc_z  / ICM42688P_ACC_LSB_PER_G;
    gx = ((float)icm42688p_gyro_x - g_bias_x) / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gy = ((float)icm42688p_gyro_y - g_bias_y) / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);
    gz = ((float)icm42688p_gyro_z - g_bias_z) / ICM42688P_GYRO_LSB_PER_DPS * (M_PI / 180.0f);

    /* 4. 运行姿态融合 */
    mahony(gx, gy, gz, ax, ay, az, dt);

    /* 5. 计算欧拉角 (Roll, Pitch, Yaw) */
    icm42688p_roll  = fast_atan2f(2.0f * (q0 * q1 + q2 * q3),
                                  1.0f - 2.0f * (q1 * q1 + q2 * q2)) * (180.0f / M_PI);
    sp = 2.0f * (q0 * q2 - q3 * q1);
    if (sp > 1.0f) sp = 1.0f; else if (sp < -1.0f) sp = -1.0f;
    icm42688p_pitch = fast_atan2f(sp, fast_sqrtf(1.0f - sp * sp)) * (180.0f / M_PI);
    icm42688p_yaw   = fast_atan2f(2.0f * (q0 * q3 + q1 * q2),
                                  1.0f - 2.0f * (q2 * q2 + q3 * q3)) * (180.0f / M_PI);
}
