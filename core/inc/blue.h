#ifndef BLUE_H
#define BLUE_H

/**
 * @file blue.h
 * @brief 蓝牙串口通信与指令解析模块头文件 (MSPM0G3519 UART1)
 * 
 * 硬件资源: UART1 (SysConfig 中定义为 blue_INST)
 * 具备功能: 蓝牙字符指令中断接收、颜色切换、速度调节、启动停止及 PID 参数在线调整
 */

#include <stdint.h>



/**
 * @brief 循迹控制器 PID 参数结构体
 */
typedef struct {
    float kp;  /**< 比例系数 P */
    float ki;  /**< 积分系数 I */
    float kd;  /**< 微分系数 D */
} LineController_t;

// 全局控制变量与接收数据声明
extern LineController_t g_line_controller;
extern volatile uint8_t g_bluetooth_data;
extern volatile uint8_t g_bt_color_mode;
extern volatile uint8_t g_bt_speed_grade;
extern volatile uint8_t g_bt_running_flag;

/**
 * @brief  初始化蓝牙模块 (使能 UART1 接收中断及 NVIC 中断线)
 */
void Bluetooth_Init(void);

/**
 * @brief  单字节蓝牙指令解析函数
 * @param  data 接收到的字符字节
 */
void Bluetooth_Process_Byte(uint8_t data);

/**
 * @brief  设置颜色模式回调接口
 * @param  color 目标颜色 (@ref BT_Color_t)
 */
void BT_SetColor(uint8_t color);

/**
 * @brief  设置运行速度档位回调接口
 * @param  speed 速度档位 (1:低速, 2:中速, 3:高速)
 */
void BT_SetSpeed(uint8_t speed);

/**
 * @brief  启动运动控制回调接口
 */
void BT_Start(void);

/**
 * @brief  停止运动控制回调接口
 */
void BT_Stop(void);

#endif /* BLUE_H */
