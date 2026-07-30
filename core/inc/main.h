#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "Grayscale.h"
#include "follow_line.h"
#include "MG513XGMR.h"

/**
 * @brief 跨模块全局对象声明；实体定义保留在各自的 .c 文件。
 */
extern Grayscale_Sensor_t g_grayscale_sensor;
extern volatile uint32_t g_system_timer_sec;
extern volatile uint8_t g_bluetooth_data;
extern volatile uint8_t g_bt_color_mode;
extern volatile uint8_t g_bt_speed_grade;
extern volatile uint8_t g_bt_running_flag;
extern LineController_t g_question2_line_controller;
extern LineController_t g_question3_line_controller;
extern LineController_t *g_active_line_controller;
extern float g_turn_output;
extern MG513XGMR_Motor_t g_motor_left;
extern MG513XGMR_Motor_t g_motor_right;
extern float g_odometer_mm;

#endif /* MAIN_H */
