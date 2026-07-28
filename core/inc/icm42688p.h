#ifndef ICM42688P_H
#define ICM42688P_H

#include "ti_msp_dl_config.h"
#include <fast_math.h>
#include <stdint.h>

/* ICM-42686-P 的 WHO_AM_I 固定返回值。 */
#define ICM42688P_WHO_AM_I_VALUE (0x44U)

extern int16_t icm42688p_acc_x;
extern int16_t icm42688p_acc_y;
extern int16_t icm42688p_acc_z;
extern int16_t icm42688p_gyro_x;
extern int16_t icm42688p_gyro_y;
extern int16_t icm42688p_gyro_z;

extern float icm42688p_roll;
extern float icm42688p_pitch;
extern float icm42688p_yaw;

uint8_t icm42688p_init(void);
uint8_t icm42688p_read_id(void);
void icm42688p_update(float dt);

#endif
