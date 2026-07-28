/*
* MSPM0G3519 FreeRTOS 双任务主程序:
* 1. SensorTask : 传感器数据采集任务 (高优先级，100Hz 定时采集陀螺仪与灰度循迹)
* 2. DisplayTask: TFT180 LCD 屏幕显示任务 (低优先级，20Hz 实时刷新显示)
*/

#include "ti_msp_dl_config.h"
#include "zf_device_tft180.h"
#include "zf_device_imu660rc.h"
#include "Grayscale.h"
#include "encode.h"
#include "drv8873.h"
#include "blue.h"
#include "follow_line.h"
#include "FreeRTOS.h"
#include "task.h"


// 任务优先级与堆栈大小定义
#define SENSOR_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 2U)
#define SENSOR_TASK_PRIORITY       (tskIDLE_PRIORITY + 3U) // 最高优先级，保障传感器采样实时性

#define MOVING_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 2U)
#define MOVING_TASK_PRIORITY       (tskIDLE_PRIORITY + 2U) // 运动控制任务优先级

#define DISPLAY_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define DISPLAY_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U) // 较低优先级，用于界面显示渲染

// 全局传感器数据对象
Grayscale_Sensor_t g_grayscale_sensor;
volatile int16_t g_encoder_left_delta;
volatile int16_t g_encoder_right_delta;
volatile int32_t g_encoder_left_total = 0;  // 32位有符号左编码器累计总脉冲数
volatile int32_t g_encoder_right_total = 0; // 32位有符号右编码器累计总脉冲数

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
        g_encoder_left_delta = Encode_Get_Delta(ENCODE_LEFT);
        g_encoder_right_delta = Encode_Get_Delta(ENCODE_RIGHT);
        
        // 累加计算左右通道有符号总脉冲数
        g_encoder_left_total += g_encoder_left_delta;
        g_encoder_right_total += g_encoder_right_delta;

        // 精确保持 10ms 定时采样周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief  小车运动控制任务 (10ms 周期 / 100Hz 控制频率)
 * @note   检查蓝牙标志位 g_bt_running_flag：
 *         为 1 时根据速度档位驱动循迹运动；
 *         为 0 时停车并清除 PID 积分状态。
 */
static void MovingTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10U);
    // //   DRV8873_Set_Speed(DRV8873_CH1, 300);
    // //   DRV8873_Set_Speed(DRV8873_CH2, 300);
    //    vTaskDelay(pdMS_TO_TICKS(1000));
    //    DRV8873_Set_Speed(DRV8873_CH1, 0);
    //   DRV8873_Set_Speed(DRV8873_CH2, 0);
    while (1) {
       if (g_bt_running_flag == 1) {
           // 根据蓝牙速度档位 (1:80, 2:120, 3:160) 计算基础速度
        //    int16_t base_speed = (int16_t)(g_bt_speed_grade * 40 + 40);
//           FollowLine_Update(&g_line_controller, &g_grayscale_sensor, 100);
             DRV8873_Set_Speed(DRV8873_CH1, 300);
      DRV8873_Set_Speed(DRV8873_CH2, 300);
//           DRV8873_Set_Speed(DRV8873_CH1, 0);
       } else {
           // 停车并关断电机 PWM
           DRV8873_Set_Speed(DRV8873_CH1, 0);
           DRV8873_Set_Speed(DRV8873_CH2, 0);
           FollowLine_Reset(&g_line_controller);
       }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief  TFT180 屏幕动态内容刷新函数
 */
void tft_display(void)
{
    char bin_str[9];

    // 1. IMU 欧拉角数据 (先注释掉)
    /*
    tft180_set_color(RGB565_RED, RGB565_WHITE);
    tft180_show_float(50, 20, imu660rc_roll, 4, 2);

    tft180_set_color(RGB565_GREEN, RGB565_WHITE);
    tft180_show_float(50, 36, imu660rc_pitch, 4, 2);

    tft180_set_color(RGB565_PURPLE, RGB565_WHITE);
    tft180_show_float(50, 52, imu660rc_yaw, 4, 2);
    */

    // 2. 显示左右编码器有符号累计总脉冲数 (有符号整数 int32_t，支持正反转正负号)
    tft180_set_color(RGB565_BLUE, RGB565_WHITE);
    tft180_show_int(50, 20, g_encoder_left_total, 6); // 左轮有符号总脉冲数

    tft180_set_color(RGB565_PURPLE, RGB565_WHITE);
    tft180_show_int(50, 36, g_encoder_right_total, 6); // 右轮有符号总脉冲数
    // 注: 若需显示 16 位硬件定时器单圈有符号计数值，也可使用:
    // (int16_t)Encode_Get_Count(ENCODE_LEFT) 和 (int16_t)Encode_Get_Count(ENCODE_RIGHT)

    // 3. 格式化并显示 SensorTask 采集到的 8 位黑白开关状态字符串 (如 "11000011")
    uint8_t dig = Grayscale_Get_Digital(&g_grayscale_sensor);
    for (int i = 0; i < 8; i++) {
        bin_str[i] = (dig & (1 << (7 - i))) ? '1' : '0';
    }
    bin_str[8] = '\0';

    tft180_set_color(RGB565_RED, RGB565_WHITE);
    tft180_show_string(45, 88, bin_str);

    // 4. 显示 8 通道原始模拟量数据
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_show_uint(45, 104, g_grayscale_sensor.analog_val[0], 4);
    tft180_show_uint(85, 104, g_grayscale_sensor.analog_val[1], 4);

    tft180_show_uint(45, 120, g_grayscale_sensor.analog_val[4], 4);
    tft180_show_uint(85, 120, g_grayscale_sensor.analog_val[5], 4);
//    tft180_clear();
}

/**
 * @brief  TFT180 LCD 屏幕显示刷新任务 (20Hz 刷新率)
 */
static void DisplayTask(void *pvParameters)
{
    (void)pvParameters;

    // 清屏并设置初始显示样式
    tft180_clear();
    tft180_set_color(RGB565_BLUE, RGB565_WHITE);
    tft180_show_string(0, 0, "=== ENCODER & TRACK ===");

    // 静态标签打印
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_show_string(0, 20, "EncL :");
    tft180_show_string(0, 36, "EncR :");
    // tft180_show_string(0, 52, "Yaw  :"); // 陀螺仪标签先注释

    tft180_show_string(0, 72, "--- Grayscale 8Ch ---");
    tft180_show_string(0, 88, "BIN :");
    tft180_show_string(0, 104, "A0-1:");
    tft180_show_string(0, 120, "A4-5:");

    while (1) {
        // 调用统一显示函数
        tft_display();

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
//    (void)imu660rc_init(IMU660RC_QUARTERNION_120HZ);

    // 3. 初始化灰度循迹传感器、编码器、电机驱动与蓝牙模块
  Grayscale_Init_First(&g_grayscale_sensor);
   Encode_Init();
   DRV8873_Init();
   Bluetooth_Init();
   Bluetooth_Send_String("123\r\n"); // 蓝牙串口上发开机测试数据
   FollowLine_Init(&g_line_controller, 0.5f, 0.1f, 0.05f); // 初始化循迹 PID 参数

    // 4. 创建传感器采集任务 (SensorTask: 优先级 3, 100Hz 采集周期)
    if (xTaskCreate(SensorTask, "SensorTask", SENSOR_TASK_STACK_SIZE, NULL,
            SENSOR_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    // 5. 创建小车运动控制任务 (MovingTask: 优先级 2, 100Hz 控制周期, 联动蓝牙标志位)
    if (xTaskCreate(MovingTask, "MovingTask", MOVING_TASK_STACK_SIZE, NULL,
            MOVING_TASK_PRIORITY, NULL) != pdPASS) {
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
