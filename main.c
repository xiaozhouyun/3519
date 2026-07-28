/*
* MSPM0G3519 FreeRTOS 三任务主程序:
* 1. ChassisTask: 底盘电机控制任务 (最高优先级，100Hz 速度/角度闭环 PID)
* 2. SensorTask : 传感器数据采集任务 (高优先级，100Hz 灰度循迹采集)
* 3. DisplayTask: TFT180 LCD 屏幕显示任务 (低优先级，20Hz 实时刷新显示)
*/

#include "ti_msp_dl_config.h"
#include "zf_device_tft180.h"
#include "zf_device_imu660rc.h"
#include "Grayscale.h"
#include "encode.h"
#include "drv8873.h"
#include "blue.h"
#include "follow_line.h"
#include "MG513XGMR.h"
#include "FreeRTOS.h"
#include "task.h"


// 任务优先级与堆栈大小定义
#define CHASSIS_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define CHASSIS_TASK_PRIORITY      (tskIDLE_PRIORITY + 3U) // 最高优先级，电机闭环控制实时性

#define SENSOR_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 2U)
#define SENSOR_TASK_PRIORITY       (tskIDLE_PRIORITY + 2U) // 高优先级，保障传感器采样实时性

#define DISPLAY_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define DISPLAY_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U) // 较低优先级，用于界面显示渲染

// 全局传感器数据对象
Grayscale_Sensor_t g_grayscale_sensor;

/**
 * @brief  底盘电机控制任务 (最高优先级, 10ms 周期 / 100Hz)
 *
 *  执行 MG513XGMR 速度/角度双闭环串级 PID 控制:
 *    编码器增量读取 → 角度环PID → 速度环PID → PWM → DRV8873
 */
static void ChassisTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10U); // 10ms (100Hz)

    while (1) {
        /* 电机闭环更新: 编码器 → 速度/角度 PID → DRV8873 PWM */
        MG513XGMR_Update();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief  传感器数据采集任务 (高优先级, 10ms 周期 / 100Hz)
 */
static void SensorTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10U); // 10ms (100Hz)

    while (1) {
        /* 更新 8 通道灰度循迹传感器 (选通多路开关、ADC0采样、二值化及归一化) */
        Grayscale_Update(&g_grayscale_sensor);
        MG513XGMR_Set_Angle(MG513XGMR_LEFT,0.0f);

        /* 循迹差速控制 (使用 MG513XGMR 闭环时暂时注释) */
        // FollowLine_Update(&g_line_controller, &g_grayscale_sensor, 100);

        /* IMU660RC: 姿态角由 PB24 (INT2) 引脚硬件中断 GROUP1_IRQHandler() 自动解算并更新 */

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief  TFT180 LCD 屏幕显示刷新任务 (20Hz 刷新率)
 */
static void DisplayTask(void *pvParameters)
{
    (void)pvParameters;
    char bin_str[9];

    // 清屏并设置初始显示样式
    tft180_clear();
    tft180_set_color(RGB565_BLUE, RGB565_WHITE);
    tft180_show_string(0, 0, "=== IMU & TRACK ===");

    // 静态标签打印
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_show_string(0, 20, "ROLL :");
    tft180_show_string(0, 36, "PITCH:");
    tft180_show_string(0, 52, "YAW  :");

    tft180_show_string(0, 72, "--- GRAYSCALE 8CH ---");
    tft180_show_string(0, 88, "BIN :");
    tft180_show_string(0, 104, "A0-1:");
    tft180_show_string(0, 120, "A4-5:");

    while (1) {
        // 1. 显示 IMU 实时欧拉角 (由 PB24 INT2 中断自动实时更新)
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft180_show_float(50, 20, imu660rc_roll, 4, 2);

        tft180_set_color(RGB565_GREEN, RGB565_WHITE);
        tft180_show_float(50, 36, imu660rc_pitch, 4, 2);

        tft180_set_color(RGB565_PURPLE, RGB565_WHITE);
        tft180_show_float(50, 52, imu660rc_yaw, 4, 2);

        // 2. 格式化并显示 SensorTask 采集到的 8 位黑白开关状态字符串 (如 "11000011")
        uint8_t dig = Grayscale_Get_Digital(&g_grayscale_sensor);
        for (int i = 0; i < 8; i++) {
            bin_str[i] = (dig & (1 << (7 - i))) ? '1' : '0';
        }
        bin_str[8] = '\0';

        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft180_show_string(45, 88, bin_str);

        // 3. 显示 8 通道原始模拟量数据
        tft180_set_color(RGB565_BLACK, RGB565_WHITE);
        tft180_show_uint(45, 104, g_grayscale_sensor.analog_val[0], 4);
        tft180_show_uint(85, 104, g_grayscale_sensor.analog_val[1], 4);
 
        tft180_show_uint(45, 120, g_grayscale_sensor.analog_val[4], 4);
        tft180_show_uint(85, 120, g_grayscale_sensor.analog_val[5], 4);

        // 50ms 刷新一次屏幕 (20Hz 刷新率)
        vTaskDelay(pdMS_TO_TICKS(50U));
    }
}

int main(void)
{
    // 1. 系统底层外设与屏驱动初始化
    SYSCFG_DL_init();
    tft180_init();

    // 2. 初始化 IMU660RC 六轴传感器 (120Hz 姿态解算，硬件 INT2 自动触发中断更新)
    (void)imu660rc_init(IMU660RC_QUARTERNION_120HZ);


    // 3. 初始化灰度循迹传感器、编码器、电机驱动与蓝牙模块
    Grayscale_Init_First(&g_grayscale_sensor);
    Encode_Init();
    DRV8873_Init();
    MG513XGMR_Init();    // 初始化左右电机速度/角度双闭环 PID
    Bluetooth_Init();
    FollowLine_Init(&g_line_controller, 0.5f, 0.1f, 0.05f); // 初始化循迹 PID 参数

    // 4. 创建底盘电机控制任务 (ChassisTask: 优先级 3, 100Hz 闭环控制)
    if (xTaskCreate(ChassisTask, "ChassisTask", CHASSIS_TASK_STACK_SIZE, NULL,
            CHASSIS_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    // 5. 创建传感器采集任务 (SensorTask: 优先级 2, 100Hz 灰度采集)
    if (xTaskCreate(SensorTask, "SensorTask", SENSOR_TASK_STACK_SIZE, NULL,
            SENSOR_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    // 6. 创建屏幕显示刷新任务 (DisplayTask: 优先级 1, 20Hz 刷新率)
    if (xTaskCreate(DisplayTask, "DisplayTask", DISPLAY_TASK_STACK_SIZE, NULL,
            DISPLAY_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    // 7. 启动 FreeRTOS 任务调度器
    vTaskStartScheduler();

    while (1) {
    }
}
