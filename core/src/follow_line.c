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
/* 第二、三问各自保存独立的循迹参数和 PID 历史。 */
LineController_t g_question2_line_controller;
LineController_t g_question3_line_controller;
LineController_t *g_active_line_controller = NULL;
static uint8_t s_active_question = 0U;

// 全局循迹转向 PID 控制输出量
float g_turn_output = 0.0f;

// 8 通道线序权重数组 (-52.5f ~ 52.5f)
static const float s_channel_weights[8] = {
    -52.5f, -37.5f, -22.5f, -7.5f,
      7.5f,  22.5f,  37.5f, 52.5f
};

/**
 * @brief  初始化循迹 PID 控制器参数
 */
void FollowLine_Init(LineController_t *p_ctrl, float kp, float ki, float kd,
                     int base_speed)
{
    if (p_ctrl == NULL) return;

    p_ctrl->kp           = kp;
    p_ctrl->ki           = ki;
    p_ctrl->kd           = kd;
    p_ctrl->integral     = 0.0f;
    p_ctrl->integral_max = 100.0f;
    p_ctrl->last_error   = 0.0f;
    p_ctrl->base_speed   = base_speed;
    p_ctrl->max_output   = 300.0f;
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

bool FollowLine_Select_Question(uint8_t question)
{
    LineController_t *p_ctrl;

    if (question == 2U) {
        p_ctrl = &g_question2_line_controller;
    } else if (question == 3U) {
        p_ctrl = &g_question3_line_controller;
    } else {
        return false;
    }

    g_active_line_controller = p_ctrl;
    s_active_question = question;
    FollowLine_Reset(p_ctrl);
    return true;
}

uint8_t FollowLine_Get_Active_Question(void)
{
    return s_active_question;
}

/**
 * @brief  根据 8 通道数字量黑白开关信号计算偏离黑线中心的位置偏差 (Error)
 *         偏差说明: 0 表示精准居中，正数表示小车偏右(黑线在左)，负数表示小车偏左(黑线在右)
 */
float FollowLine_Calc_Error(LineController_t *p_ctrl, Grayscale_Sensor_t *sensor)
{
    if (p_ctrl == NULL || sensor == NULL) return 0.0f;

    // 获取 8 通道黑白开关二值化状态掩码字节 (Bit0 ~ Bit7)
    uint8_t dig = Grayscale_Get_Digital(sensor);
    float cluster_sum = 0.0f;
    float best_error = p_ctrl->last_error;
    float best_distance = 1000.0f;
    uint8_t cluster_count = 0U;
    uint8_t has_cluster = 0U;

    // 连续黑线视为同一段；多段时选择最接近上一帧位置的一段。
    for (int i = 0; i <= 8; i++) {
        uint8_t is_black = (i < 8) && ((dig & (1U << (7 - i))) == 0U);

        if (is_black) {
            cluster_sum += s_channel_weights[i];
            cluster_count++;
        } else if (cluster_count > 0U) {
            float cluster_error = cluster_sum / (float)cluster_count;
            float distance = cluster_error - p_ctrl->last_error;

            if (distance < 0.0f) {
                distance = -distance;
            }

            if (!has_cluster || distance < best_distance) {
                best_error = cluster_error;
                best_distance = distance;
                has_cluster = 1U;
            }

            cluster_sum = 0.0f;
            cluster_count = 0U;
        }
    }

    // 若 8 个通道均未触发，维持上一次记录的历史偏差。
    if (!has_cluster) {
        return p_ctrl->last_error;
    }

    return best_error;
}

/**
 * @brief  位置式 PID 计算核心函数
 */
float FollowLine_PID_Compute(LineController_t *p_ctrl, float error)
{
    if (p_ctrl == NULL) {
        p_ctrl = g_active_line_controller;
    }
    if (p_ctrl == NULL) return 0.0f;

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
 * @param  sensor     灰度传感器结构体指针 (如 &g_grayscale_sensor)
 * @return float 转向控制调节输出量 (turn_output)
 */
float FollowLine_Update(Grayscale_Sensor_t *sensor)
{
    LineController_t *p_ctrl = g_active_line_controller;
    if (p_ctrl == NULL) return 0.0f;

    // 1. 根据数字量黑白二值化掩码解算当前偏差
    float error = FollowLine_Calc_Error(p_ctrl, sensor);

    // 2. PID 解算输出转向控制量
    g_turn_output = FollowLine_PID_Compute(p_ctrl, error);
    MG513XGMR_Set_Speed(MG513XGMR_LEFT, p_ctrl->base_speed + g_turn_output);
    MG513XGMR_Set_Speed(MG513XGMR_RIGHT, p_ctrl->base_speed - g_turn_output);
    
    return g_turn_output;
}
