#ifndef BLUE_H
#define BLUE_H

/**
 * @file blue.h
 * @brief 蓝牙串口通信与指令解析模块头文件 (MSPM0G3519 UART1)
 * 
 * 硬件资源: UART1 (SysConfig 中定义为 blue_INST)
 * 具备功能: 蓝牙字符指令中断接收、启动停止及分题 PID 参数在线调整
 */

#include <stdint.h>
#include "follow_line.h"
#include "encode.h"
#include "MG513XGMR.h"



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
 * @brief  通过蓝牙串口发送单字节数据
 * @param  ch 待发送字节
 */
void Bluetooth_Send_Byte(uint8_t ch);

/**
 * @brief  通过蓝牙串口发送字符串
 * @param  str 待发送字符串
 */
void Bluetooth_Send_String(const char *str);

/**
 * @brief  通过蓝牙串口发送系统测试数据
 */
void Bluetooth_Send_TestData(void);

/**
 * @brief  通过蓝牙串口发送当前题目的 PID 与基础速度给上位机
 */
void Bluetooth_Send_PID_Params(void);

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
 * @brief  按键启动动作，复用蓝牙启动逻辑
 */
void Key_Start(void);

/**
 * @brief  停止运动控制回调接口
 */
void BT_Stop(void);

#endif /* BLUE_H */
