#ifndef FOLLOW_LINE_H
#define FOLLOW_LINE_H

/**
 * @file follow_line.h
 * @brief 灰度循迹 PID 控制算法头文件
 * 
 * 提供 8 通道灰度传感器加权位置偏差解算、位置式 PID 算法以及左右轮差速分配接口。
 */

#ifndef FOLLOW_LINE_HOST_TEST
#include "ti_msp_dl_config.h"
#endif

#include <stdint.h>
#include <stdbool.h>
#include "Grayscale.h"

/**
 * @brief 循迹 PID 控制器结构体
 */
typedef struct {
    float kp;             /**< 比例系数 P */
    float ki;             /**< 积分系数 I */
    float kd;             /**< 微分系数 D */
    float integral;       /**< 积分累加量 */
    float integral_max;   /**< 积分上限幅值 */
    float last_error;     /**< 上一次位置偏差 */
    int    base_speed;    /**< 本题基础巡航速度 */
    float max_output;     /**< PID 输出最大限制幅值 */
} LineController_t;

/**
 * @brief  初始化循迹 PID 控制器参数
 * @param  p_ctrl 控制器结构体指针
 * @param  kp 比例参数
 * @param  ki 积分参数
 * @param  kd 微分参数
 * @param  base_speed 本题基础巡航速度
 */
void FollowLine_Init(LineController_t *p_ctrl, float kp, float ki, float kd,
                     int base_speed);

/**
 * @brief  设置 PID 参数
 * @param  p_ctrl 控制器结构体指针
 * @param  kp 比例参数
 * @param  ki 积分参数
 * @param  kd 微分参数
 */
void FollowLine_Set_PID(LineController_t *p_ctrl, float kp, float ki, float kd);

/**
 * @brief  重置/清空 PID 控制器的历史积分与上一次偏差记录
 * @param  p_ctrl 控制器结构体指针
 */
void FollowLine_Reset(LineController_t *p_ctrl);

/**
 * @brief  选择当前循迹题目并清除该题 PID 历史
 * @param  question 题号，仅支持 2 或 3
 * @return bool 选择成功返回 true
 */
bool FollowLine_Select_Question(uint8_t question);

/**
 * @brief  获取当前选中的循迹题号
 */
uint8_t FollowLine_Get_Active_Question(void);

/**
 * @brief  根据 8 通道数字量黑白开关信号计算偏离中心的位置偏差 (Error)
 * @param  sensor 灰度传感器句柄指针 (&g_grayscale_sensor)
 * @return float 偏差值 (负数表示偏左，正数表示偏右，0表示居中)
 */
float FollowLine_Calc_Error(LineController_t *p_ctrl, Grayscale_Sensor_t *sensor);

/**
 * @brief  位置式 PID 计算核心函数
 * @param  p_ctrl 控制器结构体指针
 * @param  error 当前位置偏差
 * @return float PID 转向控制输出量 (Turn Output)
 */
float FollowLine_PID_Compute(LineController_t *p_ctrl, float error);

/**
 * @brief  循迹闭环控制更新主接口 (数字量模式)
 * @param  sensor     灰度传感器句柄指针 (如 &g_grayscale_sensor)
 * @return float 转向控制调节输出量 (turn_output)
 */
float FollowLine_Update(Grayscale_Sensor_t *sensor);

#endif /* FOLLOW_LINE_H */


