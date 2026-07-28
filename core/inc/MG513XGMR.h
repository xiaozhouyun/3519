#ifndef MG513XGMR_H
#define MG513XGMR_H

/**
 * @file    MG513XGMR.h
 * @brief   MG513 直流减速电机 — 速度/角度双闭环串级 PID 控制
 *
 * 架构:  角度环(外环) → 速度环(内环) → PWM → DRV8873 → 编码器反馈
 *
 *          ┌───────────┐     ┌──────────┐     ┌──────┐     ┌──────┐
 *  angle──→│ Angle PID │────→│ Speed PID│────→│ PWM  │────→│Motor │
 *   deg    │ (Q_pid)   │ rpm │ (Q_pid)  │ pwm │[-1k..│     │      │
 *       ┌──│ fdb: accum│  ┌──│ fdb: Δenc│     │+1k]  │     │      │
 *       │  └───────────┘  │  └──────────┘     └──────┘     └──┬───┘
 *       │      pulse_accum│      Encode_Get_Delta()            │
 *       └─────────────────┴───────────────────────────────────┘
 *
 * 编码器规格: 55000 脉冲/圈
 * 更新频率:   每 10 ms (100 Hz) 在 SensorTask 中调用 MG513XGMR_Update()
 */

#include <stdint.h>
#include "Q_pid.h"

/* ================================================================
 *  硬件常量
 * ================================================================ */

/** 编码器一圈脉冲数 (AB 相 4 倍频后) */
#define MG513XGMR_PULSES_PER_REV      (55000U)

/** 速度设定上限 (pulses / 10ms)，对应约 300 RPM */
#define MG513XGMR_MAX_SPEED           (3000.0f)

/** PWM 输出上限 (匹配 DRV8873_SPEED_MAX) */
#define MG513XGMR_MAX_PWM             (1000)

/* ================================================================
 *  枚举定义
 * ================================================================ */

/** 电机通道选择 */
typedef enum {
    MG513XGMR_LEFT  = 0,          /**< 左轮 (对应 ENCODE_LEFT + DRV8873_CH1) */
    MG513XGMR_RIGHT = 1           /**< 右轮 (对应 ENCODE_RIGHT + DRV8873_CH2) */
} MG513XGMR_Channel_t;

/** 电机工作模式 */
typedef enum {
    MG513XGMR_MODE_STOP  = 0,     /**< 停止 (PWM=0) */
    MG513XGMR_MODE_SPEED = 1,     /**< 速度闭环 (直接控 PWM → 控轮速) */
    MG513XGMR_MODE_ANGLE = 2,     /**< 角度闭环 (角度环 → 速度环 → PWM) */
} MG513XGMR_Mode_t;

/* ================================================================
 *  单电机控制结构体
 * ================================================================ */

typedef struct {
    MG513XGMR_Mode_t mode;        /**< 当前运行模式 */

    /* ---- 速度环 ---- */
    pid_type_def speed_pid;       /**< Q_pid 速度 PID 句柄 */
    fp32         target_speed;    /**< 目标速度 (pulses / 10ms) */
    fp32         current_speed;   /**< 当前实测速度 (pulses / 10ms) */

    /* ---- 角度环 ---- */
    pid_type_def angle_pid;       /**< Q_pid 角度 PID 句柄 */
    float        target_angle;    /**< 目标角度 (deg, 无 wrap) */
    int32_t      pulse_total;     /**< 编码器脉冲累计总数 */
    float        current_angle;   /**< 当前实测角度 (deg = accum × 360 / 55000) */

    /* ---- 输出 ---- */
    int16_t      pwm_output;      /**< 最终 PWM 输出值 [-1000, 1000] */
} MG513XGMR_Motor_t;

/* ================================================================
 *  全局电机句柄
 * ================================================================ */

extern MG513XGMR_Motor_t g_motor_left;
extern MG513XGMR_Motor_t g_motor_right;

/* ================================================================
 *  API — 初始化 & 更新
 * ================================================================ */

/**
 * @brief  初始化左右电机 PID，清零编码器累计，停止所有电机
 */
void MG513XGMR_Init(void);

/**
 * @brief  每 10ms 调用一次 (在 SensorTask 中):
 *         1. 读取左右编码器增量 → 更新速度/角度反馈
 *         2. 根据模式执行 角度PID → 速度PID → PWM 串级控制
 */
void MG513XGMR_Update(void);

/* ================================================================
 *  API — 模式设置 (左右轮共用)
 * ================================================================ */

/**
 * @brief  设置速度闭环目标
 * @param  ch    通道: MG513XGMR_LEFT / MG513XGMR_RIGHT
 * @param  speed 目标速度 (pulses / 10ms)，范围 [-3000, 3000]
 *               正值 = 正转, 负值 = 反转
 */
void MG513XGMR_Set_Speed(MG513XGMR_Channel_t ch, fp32 speed);

/**
 * @brief  设置角度闭环目标
 * @param  ch        通道: MG513XGMR_LEFT / MG513XGMR_RIGHT
 * @param  angle_deg 目标角度 (度)，以当前清零位置为零点
 */
void MG513XGMR_Set_Angle(MG513XGMR_Channel_t ch, float angle_deg);

/**
 * @brief  停止指定电机 (模式 = STOP, PWM = 0)
 */
void MG513XGMR_Stop(MG513XGMR_Channel_t ch);

/**
 * @brief  停止所有电机
 */
void MG513XGMR_Stop_All(void);

/* ================================================================
 *  API — 参数整定
 * ================================================================ */

/**
 * @brief  设置速度环 PID 三参数
 * @note   建议初值: Kp=0.5, Ki=0.1, Kd=0.0
 */
void MG513XGMR_Set_Speed_PID(MG513XGMR_Channel_t ch, fp32 kp, fp32 ki, fp32 kd);

/**
 * @brief  设置角度环 PID 三参数
 * @note   建议初值: Kp=8.0, Ki=0.05, Kd=0.0
 */
void MG513XGMR_Set_Angle_PID(MG513XGMR_Channel_t ch, fp32 kp, fp32 ki, fp32 kd);

/**
 * @brief  清零指定电机角度累计 (重新标定当前编码器位置为 0°)
 */
void MG513XGMR_Clear_Angle(MG513XGMR_Channel_t ch);

/* ================================================================
 *  API — 状态查询
 * ================================================================ */

/** 读取当前速度 (pulses / 10ms) */
fp32 MG513XGMR_Get_Speed(MG513XGMR_Channel_t ch);

/** 读取当前角度 (deg) */
float MG513XGMR_Get_Angle(MG513XGMR_Channel_t ch);

/** 读取当前 PWM 输出 */
int16_t MG513XGMR_Get_PWM(MG513XGMR_Channel_t ch);

#endif /* MG513XGMR_H */
