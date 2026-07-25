/*
* MSPM0G3519 FreeRTOS 双任务主程序:
* 1. SensorTask : 传感器数据采集任务 (高优先级，100Hz 定时采集陀螺仪与灰度循迹)
* 2. DisplayTask: TFT180 LCD 屏幕显示任务 (低优先级，20Hz 实时刷新显示)
*/

#include "ti_msp_dl_config.h"
#include "core/inc/zf_device_tft180.h"
#include "core/inc/zf_device_imu660rc.h"
#include "core/inc/Grayscale.h"
#include "FreeRTOS.h"
#include "task.h"

// 任务优先级与堆栈大小定义
#define SENSOR_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 2U)
#define SENSOR_TASK_PRIORITY       (tskIDLE_PRIORITY + 2U) // 高优先级，保障传感器采样实时性

#define DISPLAY_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define DISPLAY_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U) // 较低优先级，用于界面显示渲染

// 全局传感器数据对象
Grayscale_Sensor_t g_grayscale_sensor;

/**
 * @brief  传感器数据采集任务 (高优先级任务, 10ms 周期 / 100Hz 刷新率)
 */
static void SensorTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10U); // 10ms (100Hz) 定时周期

    while (1) {
        // 1. 更新 8 通道灰度循迹传感器 (选通多路开关、ADC0采样、二值化及归一化)
        Grayscale_Update(&g_grayscale_sensor);

        // 2. IMU660RC 说明: 
        // 姿态角由 PB24 (INT2) 引脚硬件中断 GROUP1_IRQHandler() 自动解算并更新；
        // 如需例行抓取原始加速度/陀螺仪，也可在此调用 imu660rc_get_acc() / imu660rc_get_gyro()。

        // 精确保持 10ms 定时采样周期
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
    tft180_show_string(0, 20, "Roll :");
    tft180_show_string(0, 36, "Pitch:");
    tft180_show_string(0, 52, "Yaw  :");

    tft180_show_string(0, 72, "--- Grayscale 8Ch ---");
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

    // 3. 初始化灰度循迹传感器 (使用预设默认阈值)
    Grayscale_Init_First(&g_grayscale_sensor);

    // 4. 创建传感器采集任务 (SensorTask: 优先级 2, 100Hz 采集周期)
    if (xTaskCreate(SensorTask, "SensorTask", SENSOR_TASK_STACK_SIZE, NULL,
            SENSOR_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    // 5. 创建屏幕显示刷新任务 (DisplayTask: 优先级 1, 20Hz 刷新率)
    if (xTaskCreate(DisplayTask, "DisplayTask", DISPLAY_TASK_STACK_SIZE, NULL,
            DISPLAY_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    // 6. 启动 FreeRTOS 任务调度器
    vTaskStartScheduler();

    while (1) {
    }
}