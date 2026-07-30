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
#include "main.h"
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

// 全局传感器数据对象与系统时间计数
Grayscale_Sensor_t g_grayscale_sensor;
volatile uint32_t  g_system_timer_sec = 0; // 全局系统计时器 (秒)

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
                FollowLine_Update(&g_grayscale_sensor);
                MG513XGMR_Update();
                // 第三问里程计判断逻辑 g_odometer_mm (1500 ± 50 mm 范围内自动停车)
                if (FollowLine_Get_Active_Question() == 3U)
                {
                    if (g_odometer_mm >= 1638.0f && g_odometer_mm <= 1642.0f) {
                        BT_Stop();
                    }
                }
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
    static uint32_t s_tick_counter = 0;

    while (1) {
        /* 100Hz 采样计数：每 100 次循环 (1 秒) 递增 1 次系统秒数 */
         if(g_bt_running_flag==1)
         {
        s_tick_counter++;
        if (s_tick_counter >= 100U) {
            s_tick_counter = 0U;
            g_system_timer_sec++;
        }
            }
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
    tft_print(0, 0, TFT180_6X8_FONT, "=== SYSTEM ===");

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
        
        /* ---- 右上角显示系统运行秒数时间 (红色, 格式分:秒 mm:ss) ---- */
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft_print(96, 0, TFT180_6X8_FONT, "%02u:%02u", (unsigned int)(g_system_timer_sec / 60U), (unsigned int)(g_system_timer_sec % 60U));

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

      // 通过蓝牙发送灰度二值化字符串给上位机
      if(g_bt_running_flag==1){
        Bluetooth_Send_String(bin_str);
        Bluetooth_Send_String("\r\n");
      }
        /* ---- 循迹 PID 转向输出量 ---- */
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft_print(40, 138, TFT180_6X8_FONT, "%.2f", g_turn_output);

        vTaskDelay(pdMS_TO_TICKS(100U));
    }
}

void GROUP1_IRQHandler(void)
{
    uint32_t key_stat = DL_GPIO_getEnabledInterruptStatus(key_user_key_PORT,
                                                           key_user_key_PIN);

    if ((key_stat & key_user_key_PIN) != 0U) {
        DL_GPIO_clearInterruptStatus(key_user_key_PORT, key_user_key_PIN);
        Key_Start();
    }
}

int main(void)
{
    // 1. 系统底层外设与屏驱动初始化
    SYSCFG_DL_init();
    tft180_init();
    FollowLine_Init(&g_question2_line_controller, 5.20f, 0.0f, 0.002f, 880);
    FollowLine_Init(&g_question3_line_controller, 4.20f, 0.0f, 0.002f, 600);
    FollowLine_Select_Question(3U);
    Encode_Init();
    DRV8873_Init();
    MG513XGMR_Init();    // 初始化左右电机速度/角度双闭环 PID
    Bluetooth_Init();

    /* 使能 GPIOB 中断 (PB31 按键启动 + PB24 IMU INT2 共用 GROUP1) */
    NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
 

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
