/*
* MSPM0G3519 TFT180 LCD 与 IMU660RC 姿态解算实时显示主程序
*/

#include "ti_msp_dl_config.h"
#include "core/inc/zf_device_tft180.h"
#include "core/inc/zf_device_imu660rc.h"
#include "FreeRTOS.h"
#include "task.h"

#define DISPLAY_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define DISPLAY_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U)

static void DisplayTask(void *pvParameters)
{
    (void)pvParameters;

    // 清屏并设置初始显示样式
    tft180_clear();
    tft180_set_color(RGB565_BLUE, RGB565_WHITE);
    tft180_show_string(0, 0, "=== IMU660RC ===");

    // 静态标签打印
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_show_string(0, 20, "Roll :");
    tft180_show_string(0, 40, "Pitch:");
    tft180_show_string(0, 60, "Yaw  :");

    tft180_show_string(0, 85, "--- Accel (g) ---");
    tft180_show_string(0, 100, "Ax:");
    tft180_show_string(0, 115, "Ay:");
    tft180_show_string(0, 130, "Az:");

    while (1) {
        // 1. 显示实时欧拉角 (由 PB24 INT2 中断触发硬件融合解算更新)
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft180_show_float(50, 20, imu660rc_roll, 4, 2);

        tft180_set_color(RGB565_GREEN, RGB565_WHITE);
        tft180_show_float(50, 40, imu660rc_pitch, 4, 2);

        tft180_set_color(RGB565_PURPLE, RGB565_WHITE);
        tft180_show_float(50, 60, imu660rc_yaw, 4, 2);

        // 2. 显示物理加速度 (单位 g)
        tft180_set_color(RGB565_BLACK, RGB565_WHITE);
        tft180_show_float(30, 100, imu660rc_acc_transition(imu660rc_acc_x), 3, 2);
        tft180_show_float(30, 115, imu660rc_acc_transition(imu660rc_acc_y), 3, 2);
        tft180_show_float(30, 130, imu660rc_acc_transition(imu660rc_acc_z), 3, 2);

        // 50ms 刷新一次屏幕 (20Hz 刷新率)
        vTaskDelay(pdMS_TO_TICKS(50U));
    }
}

int main(void)
{
    // 1. 系统底层外设与屏驱动初始化
    SYSCFG_DL_init();
    tft180_init();

    // 2. 初始化 IMU660RC 六轴传感器 (120Hz 姿态解算，解算完成触发 PB24 中断自动更新)
    (void)imu660rc_init(IMU660RC_QUARTERNION_120HZ);

    // 3. 创建屏幕显示刷新任务
    if (xTaskCreate(DisplayTask, "Display", DISPLAY_TASK_STACK_SIZE, NULL,
            DISPLAY_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    // 4. 启动 FreeRTOS 任务调度
    vTaskStartScheduler();

    while (1) {
    }
}