/*
* MSPM0G3519 FreeRTOS 三任务主程序:
* 1. ChassisTask: 底盘电机控制任务 (最高优先级，100Hz 速度/角度闭环 PID)
* 2. SensorTask : 传感器数据采集任务 (高优先级，100Hz 灰度循迹采集)
* 3. DisplayTask: TFT180 LCD 屏幕显示任务 (低优先级，20Hz 实时刷新显示)
*/

#include "ti_msp_dl_config.h"
#include "zf_device_tft180.h"
#include "icm42688p.h"
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
         Grayscale_Update(&g_grayscale_sensor);
        if(g_bt_running_flag==1)
        {     
                FollowLine_Update(&g_line_controller, &g_grayscale_sensor, 300);
                MG513XGMR_Update();
        }
        else
        {
            MG513XGMR_Stop_All();
        }

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
 
        /* ICM42688P: 姿态解算更新 (10ms 周期, dt=0.01s) */
        // icm42688p_update(0.01f);

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

    tft180_clear();
    tft180_set_color(RGB565_BLUE, RGB565_WHITE);
    tft_print(0, 0, TFT180_6X8_FONT, "=== MOTOR & SYSTEM ===");

    /* 静态标签 */
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft_print(0,  14, TFT180_6X8_FONT, "L_PWM:");
    tft_print(0,  26, TFT180_6X8_FONT, "R_PWM:");
    tft_print(0,  38, TFT180_6X8_FONT, "L_SPD:");
    tft_print(0,  50, TFT180_6X8_FONT, "R_SPD:");
    tft_print(0,  62, TFT180_6X8_FONT, "L_TOT:");
    tft_print(0,  74, TFT180_6X8_FONT, "R_TOT:");
    tft_print(0,  88, TFT180_6X8_FONT, "--- GRAY 8CH ---");
    tft_print(0, 100, TFT180_6X8_FONT, "BIN:");
    tft_print(0, 112, TFT180_6X8_FONT, "0-1:");
    tft_print(0, 124, TFT180_6X8_FONT, "4-5:");
    tft_print(0, 138, TFT180_6X8_FONT, "TURN:");

    while (1) {
        /* ---- 左右电机 PWM 输出 ---- */
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft_print(45, 14, TFT180_6X8_FONT, "%d", (int)g_motor_left.pwm_output);
        tft_print(45, 26, TFT180_6X8_FONT, "%d", (int)g_motor_right.pwm_output);

        /* ---- 左右电机 目标速度(T) / 实测速度(R) ---- */
        tft180_set_color(RGB565_BLUE, RGB565_WHITE);
        tft_print(45, 38, TFT180_6X8_FONT, "T:%-5d R:%-5d", (int)g_motor_left.target_speed, (int)g_motor_left.current_speed);
        tft_print(45, 50, TFT180_6X8_FONT, "T:%-5d R:%-5d", (int)g_motor_right.target_speed, (int)g_motor_right.current_speed);

        /* ---- 编码器累计总脉冲数 (pulse_total) ---- */
        tft180_set_color(RGB565_BLACK, RGB565_WHITE);
        tft_print(45, 62, TFT180_6X8_FONT, "%d", (int)g_motor_left.pulse_total);
        tft_print(45, 74, TFT180_6X8_FONT, "%d", (int)g_motor_right.pulse_total);

        /* ---- 灰度二值化字符串 ---- */
        uint8_t dig = Grayscale_Get_Digital(&g_grayscale_sensor);
        for (int i = 0; i < 8; i++) {
            bin_str[i] = (dig & (1 << (7 - i))) ? '1' : '0';
        }
        bin_str[8] = '\0';
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft_print(35, 100, TFT180_6X8_FONT, "%s", bin_str);

        /* ---- 灰度模拟量 ---- */
        tft180_set_color(RGB565_BLACK, RGB565_WHITE);
        tft_print(35, 112, TFT180_6X8_FONT, "%u", (unsigned int)g_grayscale_sensor.analog_val[0]);
        tft_print(80, 112, TFT180_6X8_FONT, "%u", (unsigned int)g_grayscale_sensor.analog_val[1]);
        tft_print(35, 124, TFT180_6X8_FONT, "%u", (unsigned int)g_grayscale_sensor.analog_val[4]);
        tft_print(80, 124, TFT180_6X8_FONT, "%u", (unsigned int)g_grayscale_sensor.analog_val[5]);

        /* ---- 循迹 PID 转向输出量 ---- */
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft_print(40, 138, TFT180_6X8_FONT, "%.2f", g_turn_output);

        vTaskDelay(pdMS_TO_TICKS(50U));
    }
}
int main(void)
{
    // 1. 系统底层外设与屏驱动初始化
    SYSCFG_DL_init();
    tft180_init();

    // 2. 初始化 ICM42688P 六轴传感器
    // if (icm42688p_init() != 0U) {
    //     /* 初始化失败 —— 打印实际读到的 ID 便于排查，然后挂起 */
    //     tft180_set_color(RGB565_RED, RGB565_WHITE);
    //     tft_print(0, 136, TFT180_6X8_FONT, "IMU init FAIL!");
    //     tft_print(120, 136, TFT180_6X8_FONT, "%u", (unsigned int)icm42688p_read_id());
    //     while (1) {}
    // }

    // 3. 初始化灰度循迹传感器、编码器、电机驱动与蓝牙模块
    Grayscale_Init_First(&g_grayscale_sensor);
    FollowLine_Init(&g_line_controller, 2.0f, 0.0f, 0.0f); // 初始化循迹 PID 参数
    Encode_Init();
    DRV8873_Init();
    MG513XGMR_Init();    // 初始化左右电机速度/角度双闭环 PID
    Bluetooth_Init();
 

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
