/**
 * @file follow_line.c
 * @brief 灰度循迹 PID 控制算法实现文件
 * 
 * 实现算法:
 * 1. 8 通道加权中心位置偏差解算 (重心法 Weighted Average)
 * 2. 位置式 PID 控制器解算 (带抗积分饱和限幅与输出限制)
 * 3. 左右轮差速映射输出
 */

#include "follow_line.h"
#include "drv8873.h"
#include "MG513XGMR.h"
// 默认全局循迹 PID 控制器实例
LineController_t g_line_controller = {
    .kp           = 0.5f,
    .ki           = 0.0f,
    .kd           = 0.2f,
    .integral     = 0.0f,
    .integral_max = 500.0f,
    .last_error   = 0.0f,
    .max_output   = 800.0f
};

// 全局循迹转向 PID 控制输出量
float g_turn_output = 0.0f;

// 8 通道线序权重数组 (-3500 ~ 3500)
static const float s_channel_weights[8] = {
    -350.0f, -250.0f, -150.0f, -50.0f,
      50.0f,  150.0f,  250.0f, 350.0f
};

/**
 * @brief  初始化循迹 PID 控制器参数
 */
void FollowLine_Init(LineController_t *p_ctrl, float kp, float ki, float kd)
{
    if (p_ctrl == NULL) return;

    p_ctrl->kp           = kp;
    p_ctrl->ki           = ki;
    p_ctrl->kd           = kd;
    p_ctrl->integral     = 0.0f;
    p_ctrl->integral_max = 500.0f;
    p_ctrl->last_error   = 0.0f;
    p_ctrl->max_output   = 100.0f;
}

/**
 * @brief  在线修改设置 PID 参数
 */
void FollowLine_Set_PID(LineController_t *p_ctrl, float kp, float ki, float kd)
{
    if (p_ctrl == NULL) return;

    p_ctrl->kp = kp;
    p_ctrl->ki = ki;
    p_ctrl->kd = kd;
}

/**
 * @brief  重置 PID 历史积分与上一次偏差记录
 */
void FollowLine_Reset(LineController_t *p_ctrl)
{
    if (p_ctrl == NULL) return;

    p_ctrl->integral   = 0.0f;
    p_ctrl->last_error = 0.0f;
}

/**
 * @brief  根据 8 通道数字量黑白开关信号计算偏离黑线中心的位置偏差 (Error)
 *         偏差说明: 0 表示精准居中，正数表示小车偏右(黑线在左)，负数表示小车偏左(黑线在右)
 */
float FollowLine_Calc_Error(Grayscale_Sensor_t *sensor)
{
    if (sensor == NULL) return 0.0f;

    // 获取 8 通道黑白开关二值化状态掩码字节 (Bit0 ~ Bit7)
    uint8_t dig = Grayscale_Get_Digital(sensor);
    float weighted_sum = 0.0f;
    float count        = 0.0f;

    // 统计检测到黑线的通道加权位置 (假设 Bit 位为 1 时代表踩在黑线上，若实际为 0 表示黑线则用 !(dig & (1<<i)))
    for (int i = 0; i < 8; i++) {
        if (dig & (1 << (7 - i))) {
            weighted_sum += s_channel_weights[i];
            count        += 1.0f;
        }
    }

    // 防零除保护：若 8 个通道均未触发（全白/脱轨），维持上一次记录的历史偏差
    if (count < 0.1f) {
        return g_line_controller.last_error;
    }

    return (weighted_sum / count);
}

/**
 * @brief  位置式 PID 计算核心函数
 */
float FollowLine_PID_Compute(LineController_t *p_ctrl, float error)
{
    if (p_ctrl == NULL) p_ctrl = &g_line_controller;

    // 1. P 比例项
    float p_out = p_ctrl->kp * error;

    // 2. I 积分项 (带抗积分饱和限幅)
    p_ctrl->integral += error;
    if (p_ctrl->integral > p_ctrl->integral_max) {
        p_ctrl->integral = p_ctrl->integral_max;
    } else if (p_ctrl->integral < -p_ctrl->integral_max) {
        p_ctrl->integral = -p_ctrl->integral_max;
    }
    float i_out = p_ctrl->ki * p_ctrl->integral;

    // 3. D 微分项
    
    // 采样时间为 10ms (0.01s)
    float dt = 0.01f; 
    float derivative = (error - p_ctrl->last_error) / dt;
    float d_out = p_ctrl->kd * derivative;

    // 更新上一次偏差记录
    p_ctrl->last_error = error;

    // 4. 总输出合成与输出限幅
    float total_output = p_out + i_out + d_out;
    if (total_output > p_ctrl->max_output) {
        total_output = p_ctrl->max_output;
    } else if (total_output < -p_ctrl->max_output) {
        total_output = -p_ctrl->max_output;
    }

    return total_output;
}

/**
 * @brief  循迹闭环控制更新主接口 (基于数字量)
 * @param  p_ctrl     PID 控制器句柄指针 (如 &g_line_controller)
 * @param  sensor     灰度传感器结构体指针 (如 &g_grayscale_sensor)
 * @param  base_speed 基础巡航速度/模式标示
 * @return float 转向控制调节输出量 (turn_output)
 */
float FollowLine_Update(LineController_t *p_ctrl, Grayscale_Sensor_t *sensor, int16_t base_speed)
{
    if (p_ctrl == NULL) {
        p_ctrl = &g_line_controller;
    }

    // 1. 根据数字量黑白二值化掩码解算当前偏差
    float error = FollowLine_Calc_Error(sensor);

    // 2. PID 解算输出转向控制量
    g_turn_output = FollowLine_PID_Compute(p_ctrl, error);
    MG513XGMR_Set_Speed(MG513XGMR_LEFT, base_speed - g_turn_output);
    MG513XGMR_Set_Speed(MG513XGMR_RIGHT, base_speed + g_turn_output);
    
    return g_turn_output;
}
