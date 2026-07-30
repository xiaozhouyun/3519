/**
 * @file blue.c
 * @brief 蓝牙串口通信与指令解析模块实现文件
 * 

 */

#include "blue.h"
#include "Grayscale.h"
#include "main.h"
#ifndef BLUE_HOST_TEST
#include "ti_msp_dl_config.h"
#endif

// 全局变量定义

volatile uint8_t g_bluetooth_data   = 0;
volatile uint8_t g_bt_speed_grade   = 1;
volatile uint8_t g_bt_running_flag  = 0;

/**
 * @brief  初始化蓝牙 UART1 中断
 */
void Bluetooth_Init(void)
{
#ifndef BLUE_HOST_TEST
    // 1. 设置蓝牙串口 NVIC 中断优先级为 2 (MSPM0 范围 0~3，0 最高，2 为适合串口数据的中等优先级)
    NVIC_SetPriority(blue_INST_INT_IRQN, 2);
    // 2. 使能 UART1 接收 FIFO 非空中断
    DL_UART_Main_enableInterrupt(blue_INST, DL_UART_MAIN_INTERRUPT_RX);
    // 3. 使能 NVIC 中断向量线
    NVIC_EnableIRQ(blue_INST_INT_IRQN);
#endif
}


#include <stdio.h>
#include <string.h>

/**
 * @brief  通过蓝牙串口发送单字节
 */
void Bluetooth_Send_Byte(uint8_t ch)
{
#ifndef BLUE_HOST_TEST
    DL_UART_Main_transmitDataBlocking(blue_INST, ch);
#endif
}

/**
 * @brief  通过蓝牙串口发送字符串
 */
void Bluetooth_Send_String(const char *str)
{
    if (str == NULL) return;
    while (*str != '\0') {
        Bluetooth_Send_Byte((uint8_t)*str);
        str++;
    }
}

/**
 * @brief  通过蓝牙串口发送系统测试数据
 */
void Bluetooth_Send_TestData(void)
{
    char buf[80];
    snprintf(buf, sizeof(buf), "[TEST] Run:%d, Speed:%d, EncL:%ld, EncR:%ld\r\n",
             (int)g_bt_running_flag, (int)g_bt_speed_grade,
             (long)g_motor_left.pulse_total, (long)g_motor_right.pulse_total);
    Bluetooth_Send_String(buf);
}

/**
 * @brief  通过蓝牙串口发送当前循迹 PID 参数 (Kp, Ki, Kd) 给上位机
 */
void Bluetooth_Send_PID_Params(void)
{
    char pid_buf[64];
    if (g_active_line_controller == NULL) {
        Bluetooth_Send_String("[PID] No question selected\r\n");
        return;
    }

    snprintf(pid_buf, sizeof(pid_buf), "[Q%u] Kp:%.2f Ki:%.3f Kd:%.3f V:%d\r\n",
             (unsigned int)FollowLine_Get_Active_Question(),
             g_active_line_controller->kp, g_active_line_controller->ki,
             g_active_line_controller->kd, g_active_line_controller->base_speed);
    Bluetooth_Send_String(pid_buf);
}

/**
 * @brief  解析接收到的单个蓝牙字节指令
 * @param  data 接收到的字符 ASCII 码
 */
void Bluetooth_Process_Byte(uint8_t data)
{
    g_bluetooth_data = data;

    // 解析蓝牙命令（仅处理有效命令，无效字节静默丢弃）
    switch (data) {
        case 'R':  // 切换红色模式
           
            break;
        case 'G':  // 切换绿色模式
           
            break;
        case 'B':  // 切换蓝色模式
           
            break;
        case '1':  // 保留
         
            break;
        case '2':  // 选择第二问
            FollowLine_Select_Question(2U);
            Bluetooth_Send_PID_Params();
            break;
        case '3':  // 选择第三问
            FollowLine_Select_Question(3U);
            Bluetooth_Send_PID_Params();
            break;
        case 'S':  // 启动小车
        case 's':
            BT_Start();
            Bluetooth_Send_String("System Started!\r\n");
            break;
        case 'X':  // 停止小车
        case 'x':
            BT_Stop();
            Bluetooth_Send_String("System Stopped!\r\n");
            break;
        case 'T':  // 测试数据回传
        case 't':
            Bluetooth_Send_TestData();
            break;
        case 'Q':  // 查询并上发循迹 PID 参数 (Kp, Ki, Kd)
        case 'q':
            Bluetooth_Send_PID_Params();
            break;
        case 'P':  // 调整 PID 参数: Kp + 0.1
            if (g_active_line_controller != NULL) {
                g_active_line_controller->kp += 0.1f;
                Bluetooth_Send_PID_Params();
            }
            break;
        case 'p':  // 调整 PID 参数: Kp - 0.1
            if (g_active_line_controller != NULL) {
                g_active_line_controller->kp -= 0.1f;
                Bluetooth_Send_PID_Params();
            }
            break;
        case 'I':  // 调整 PID 参数: Ki + 0.01
            if (g_active_line_controller != NULL) {
                g_active_line_controller->ki += 0.01f;
                Bluetooth_Send_PID_Params();
            }
            break;
        case 'i':  // 调整 PID 参数: Ki - 0.01
            if (g_active_line_controller != NULL) {
                g_active_line_controller->ki -= 0.01f;
                Bluetooth_Send_PID_Params();
            }
            break;
        case 'K':  // 调整 PID 参数: Kd + 0.5
        case 'D':
            if (g_active_line_controller != NULL) {
                g_active_line_controller->kd += 0.5f;
                Bluetooth_Send_PID_Params();
            }
            break;
        case 'k':  // 调整 PID 参数: Kd - 0.5
        case 'd':
            if (g_active_line_controller != NULL) {
                g_active_line_controller->kd -= 0.5f;
                Bluetooth_Send_PID_Params();
            }
            break;
        default:
            // 忽略未定义或无效字符
            break;
    }
}

/**
 * @brief  默认蓝牙设置颜色动作函数 (可根据需要扩展)
 */


/**
 * @brief  默认蓝牙设置速度动作函数 (可根据需要扩展)
 */
void BT_SetSpeed(uint8_t speed)
{
    g_bt_speed_grade = speed;
}

/**
 * @brief  默认蓝牙启动动作函数
 */
void BT_Start(void)
{
    g_system_timer_sec = 0U;
    g_bt_running_flag = 1;
}

void Key_Start(void)
{
    BT_Start();
}

/**
 * @brief  默认蓝牙停止动作函数
 */
void BT_Stop(void)
{
    g_bt_running_flag = 0;
}

#ifndef BLUE_HOST_TEST
/**
 * @brief  MSPM0G3519 UART1 硬件接收中断服务函数
 * @note  每次 FIFO 收到字节时自动响应
 */
void blue_INST_IRQHandler(void)
{
    // 读取并判断中断标志
    switch (DL_UART_Main_getPendingInterrupt(blue_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            // 读取接收 FIFO 中的所有有效字节
            while (!DL_UART_Main_isRXFIFOEmpty(blue_INST)) {
                uint8_t rx_byte = DL_UART_Main_receiveData(blue_INST);
                Bluetooth_Process_Byte(rx_byte);
            }
            break;
        default:
            break;
    }
}
#endif
