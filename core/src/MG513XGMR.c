/**
 * @file    MG513XGMR.c
 * @brief   MG513 直流减速电机 — 速度/角度双闭环串级 PID 控制实现
 *
 * 控制架构 (每个电机独立):
 *
 *   MG513XGMR_Set_Angle(deg)
 *   ┌────────────────────────────────────────────┐
 *   │  角度环 (外环, pid_type_def angle_pid)       │
 *   │  set = target_angle (deg)                  │
 *   │  fdb = pulse_accum * 360.0 / 55000.0       │
 *   │  out = target_speed  (pulses/10ms) ────────┤
 *   └──────────────────────────────────┬─────────┘
 *                                      ▼
 *   MG513XGMR_Set_Speed(pulses/10ms)
 *   ┌────────────────────────────────────────────┐
 *   │  速度环 (内环, pid_type_def speed_pid)       │
 *   │  set = target_speed (pulses/10ms)          │
 *   │  fdb = Encode_Get_Delta()                  │
 *   │  out = PWM [-1000, 1000] ──────────────────┤
 *   └──────────────────────────────────┬─────────┘
 *                                      ▼
 *              DRV8873_Set_Speed(ch, pwm)
 *
 * 更新周期: 每 10ms 调用一次 MG513XGMR_Update()
 */

#include "MG513XGMR.h"
#include "encode.h"
#include "drv8873.h"
#include "Q_pid.h"

/* ================================================================
 *  全局电机句柄定义
 * ================================================================ */

/** 左电机控制器句柄 (包含模式、PID对象、反馈状态及PWM输出) */
MG513XGMR_Motor_t g_motor_left;

/** 右电机控制器句柄 (包含模式、PID对象、反馈状态及PWM输出) */
MG513XGMR_Motor_t g_motor_right;

/* ================================================================
 *  内部辅助 — 通道映射
 * ================================================================ */

/**
 * @brief  获取指定通道的电机句柄指针
 * @param  ch 通道枚举 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @return 指向对应 MG513XGMR_Motor_t 结构体的指针
 */
static MG513XGMR_Motor_t *Get_Motor(MG513XGMR_Channel_t ch)
{
    return (ch == MG513XGMR_RIGHT) ? &g_motor_right : &g_motor_left;
}

/**
 * @brief  将 MG513XGMR 通道映射为编码器底层通道
 * @param  ch 电机通道枚举
 * @return 对应的编码器通道枚举 (ENCODE_LEFT / ENCODE_RIGHT)
 */
static Encode_Channel_t To_Encode_Ch(MG513XGMR_Channel_t ch)
{
    return (ch == MG513XGMR_RIGHT) ? ENCODE_RIGHT : ENCODE_LEFT;
}

/**
 * @brief  将 MG513XGMR 通道映射为 DRV8873 电机驱动芯片通道
 * @param  ch 电机通道枚举
 * @return 对应的 DRV8873 通道枚举 (DRV8873_CH1 / DRV8873_CH2)
 */
static DRV8873_Channel_t To_DRV8873_Ch(MG513XGMR_Channel_t ch)
{
    return (ch == MG513XGMR_RIGHT) ? DRV8873_CH2 : DRV8873_CH1;
}

/* ================================================================
 *  内部辅助 — PID 重置
 * ================================================================ */

/**
 * @brief  重置单个电机的速度/角度 PID 历史状态 (清空积分与微分历史，保留设定参数)
 * @param  motor 指向目标电机控制句柄的指针
 */
static void Reset_PID(MG513XGMR_Motor_t *motor)
{
    PID_clear(&motor->speed_pid);
    PID_clear(&motor->angle_pid);
    motor->pwm_output = 0;
}

/* ================================================================
 *  API — 初始化
 * ================================================================ */

/**
 * @brief  初始化左右电机 PID (默认参数)，清零编码器累计，停止所有电机
 * @note   速度环 PID 参数: Kp=0.5, Ki=0.15, Kd=0.0, PWM输出限制 [-1000, 1000]
 *         角度环 PID 参数: Kp=8.0, Ki=0.05, Kd=0.0, 目标速度输出限制 [-3000, 3000]
 */
void MG513XGMR_Init(void)
{
    MG513XGMR_Motor_t *motor;
    fp32 params[3];

    /* ---- 左轮初始化 ---- */
    motor = &g_motor_left;
    motor->mode          = MG513XGMR_MODE_STOP;
    motor->target_speed  = 0.0f;
    motor->current_speed = 0.0f;
    motor->target_angle  = 0.0f;
    motor->pulse_total   = 0;
    motor->current_angle = 0.0f;
    motor->pwm_output    = 0;

    /*  速度环 PID: Kp=0.5, Ki=0.15, Kd=0.0, out∈[-1000,1000], iout∈[-500,500] */
    params[0] = 0.5f;  params[1] = 0.15f; params[2] = 0.0f;
    PID_init(&motor->speed_pid, PID_POSITION, params,
             (fp32)MG513XGMR_MAX_PWM, (fp32)(MG513XGMR_MAX_PWM / 2));

    /*  角度环 PID: Kp=8.0, Ki=0.05, Kd=0.0, out∈[-3000,3000], iout∈[-1500,1500] */
    params[0] = 8.0f;  params[1] = 0.05f; params[2] = 0.0f;
    PID_init(&motor->angle_pid, PID_POSITION, params,
             MG513XGMR_MAX_SPEED, MG513XGMR_MAX_SPEED / 2.0f);

    /* ---- 右轮: 结构体拷贝复制初始化值 ---- */
    g_motor_right = g_motor_left;
    Reset_PID(&g_motor_right);

    /* 硬件层确保电机停止 */
    DRV8873_Stop_All();
}

/* ================================================================
 *  API — 模式设置
 * ================================================================ */

/**
 * @brief  设置速度闭环目标
 * @param  ch    电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @param  speed 目标速度 (pulses / 10ms)，范围 [-3000, 3000]，正值=正转，负值=反转
 */
void MG513XGMR_Set_Speed(MG513XGMR_Channel_t ch, fp32 speed)
{
    MG513XGMR_Motor_t *motor = Get_Motor(ch);

    /* 限幅到 [-MAX_SPEED, MAX_SPEED] */
    if (speed > MG513XGMR_MAX_SPEED)       speed = MG513XGMR_MAX_SPEED;
    else if (speed < -MG513XGMR_MAX_SPEED) speed = -MG513XGMR_MAX_SPEED;

    /* 从其他模式切入速度模式时，清除 PID 历史防止积分突变 */
    if (motor->mode != MG513XGMR_MODE_SPEED) {
        Reset_PID(motor);
    }

    motor->mode         = MG513XGMR_MODE_SPEED;
    motor->target_speed = speed;
}

/**
 * @brief  设置角度闭环目标
 * @param  ch        电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @param  angle_deg 目标角度 (单位: 度)，以最近一次 MG513XGMR_Clear_Angle() 位置为零点
 */
void MG513XGMR_Set_Angle(MG513XGMR_Channel_t ch, float angle_deg)
{
    MG513XGMR_Motor_t *motor = Get_Motor(ch);

    /* 从其他模式切入角度模式时，清除 PID 历史 */
    if (motor->mode != MG513XGMR_MODE_ANGLE) {
        Reset_PID(motor);
    }

    motor->mode         = MG513XGMR_MODE_ANGLE;
    motor->target_angle = angle_deg;
}

/**
 * @brief  停止指定电机 (切换模式为 STOP，清零 PWM 输出，重置 PID)
 * @param  ch 电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 */
void MG513XGMR_Stop(MG513XGMR_Channel_t ch)
{
    MG513XGMR_Motor_t *motor = Get_Motor(ch);

    motor->mode         = MG513XGMR_MODE_STOP;
    motor->pwm_output   = 0;
    motor->target_speed = 0.0f;
    Reset_PID(motor);
    DRV8873_Stop(To_DRV8873_Ch(ch));
}

/**
 * @brief  停止所有电机 (左右轮同时紧急停止)
 */
void MG513XGMR_Stop_All(void)
{
    MG513XGMR_Stop(MG513XGMR_LEFT);
    MG513XGMR_Stop(MG513XGMR_RIGHT);
}

/* ================================================================
 *  API — 参数整定
 * ================================================================ */

/**
 * @brief  热更新速度环 PID 参数 (在线整定，不重置 PID 历史积分)
 * @param  ch 电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @param  kp 比例系数
 * @param  ki 积分系数
 * @param  kd 微分系数
 */
void MG513XGMR_Set_Speed_PID(MG513XGMR_Channel_t ch, fp32 kp, fp32 ki, fp32 kd)
{
    MG513XGMR_Motor_t *motor = Get_Motor(ch);

    motor->speed_pid.Kp = kp;
    motor->speed_pid.Ki = ki;
    motor->speed_pid.Kd = kd;
}

/**
 * @brief  热更新角度环 PID 参数 (在线整定，不重置 PID 历史积分)
 * @param  ch 电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @param  kp 比例系数
 * @param  ki 积分系数
 * @param  kd 微分系数
 */
void MG513XGMR_Set_Angle_PID(MG513XGMR_Channel_t ch, fp32 kp, fp32 ki, fp32 kd)
{
    MG513XGMR_Motor_t *motor = Get_Motor(ch);

    motor->angle_pid.Kp = kp;
    motor->angle_pid.Ki = ki;
    motor->angle_pid.Kd = kd;
}

/**
 * @brief  清零角度累计 (将当前编码器位置重新标定为 0°)
 * @param  ch 电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 */
void MG513XGMR_Clear_Angle(MG513XGMR_Channel_t ch)
{
    MG513XGMR_Motor_t *motor = Get_Motor(ch);

    motor->pulse_total   = 0;
    motor->current_angle = 0.0f;
    motor->target_angle  = 0.0f;
    Reset_PID(motor);
}

/* ================================================================
 *  API — 状态查询
 * ================================================================ */

/**
 * @brief  读取指定电机的当前实测速度
 * @param  ch 电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @return 当前速度 (单位: pulses / 10ms)
 */
fp32 MG513XGMR_Get_Speed(MG513XGMR_Channel_t ch)
{
    return Get_Motor(ch)->current_speed;
}

/**
 * @brief  读取指定电机的当前实测累计角度
 * @param  ch 电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @return 当前累计角度 (单位: 度)
 */
float MG513XGMR_Get_Angle(MG513XGMR_Channel_t ch)
{
    return Get_Motor(ch)->current_angle;
}

/**
 * @brief  读取指定电机的当前 PWM 控制输出值
 * @param  ch 电机通道 (MG513XGMR_LEFT / MG513XGMR_RIGHT)
 * @return 当前 PWM 输出值，范围 [-1000, 1000]
 */
int16_t MG513XGMR_Get_PWM(MG513XGMR_Channel_t ch)
{
    return Get_Motor(ch)->pwm_output;
}

/* ================================================================
 *  API — 主更新函数 (每 10ms 调用)
 * ================================================================ */

/**
 * @brief  核心更新循环 — 对左右电机分别执行级联 PID 控制
 *
 *  流程:
 *   1. 读取编码器增量 → 更新 pulse_accum / current_speed / current_angle
 *   2. 根据当前模式分发:
 *      STOP  → 保持 PWM 输出为 0，不主动接管底层硬件驱动
 *      SPEED → 单环速度 PID 控制: ref=当前速度, set=目标速度 → 输出 PWM 驱动 DRV8873
 *      ANGLE → 级联双环 PID 控制:
 *              外环(角度): ref=当前角度, set=目标角度 → 输出目标速度
 *              内环(速度): ref=当前速度, set=外环输出速度 → 输出 PWM 驱动 DRV8873
 */
void MG513XGMR_Update(void)
{
    /* 左右电机依次处理 */
    for (int i = 0; i < 2; i++) {
        MG513XGMR_Channel_t ch    = (MG513XGMR_Channel_t)i; // 电机通道
        MG513XGMR_Motor_t  *motor = Get_Motor(ch);          // 电机控制句柄
        Encode_Channel_t    enc   = To_Encode_Ch(ch);
        DRV8873_Channel_t   drv   = To_DRV8873_Ch(ch);

        /* ---- 步骤1: 读取编码器，更新反馈量 ---- */
        int16_t delta = Encode_Get_Delta(enc);

        motor->pulse_total  += (int32_t)delta;
        motor->current_speed = (fp32)delta;
        motor->current_angle = (float)motor->pulse_total * 360.0f
                               / (float)MG513XGMR_PULSES_PER_REV;

        /* ---- 步骤2: 按模式执行 PID ---- */
        switch (motor->mode) {

        case MG513XGMR_MODE_STOP:
            /* STOP 模式不写硬件，留给循迹等其他控制逻辑接管 DRV8873 */
            motor->pwm_output   = 0;
            motor->target_speed = 0.0f;
            break;

        case MG513XGMR_MODE_SPEED:
            /* 速度环: ref=当前速度, set=目标速度 → 输出 PWM */
            motor->pwm_output = (int16_t)PID_calc(&motor->speed_pid,
                                                  motor->current_speed,
                                                  motor->target_speed);
            DRV8873_Set_Speed(drv, motor->pwm_output);
            break;

        case MG513XGMR_MODE_ANGLE:
            /* 角度环 (外环): ref=当前角度, set=目标角度 → 输出目标速度 */
            motor->target_speed = PID_calc(&motor->angle_pid,
                                           (fp32)motor->current_angle,
                                           (fp32)motor->target_angle);

            /* 速度环 (内环): ref=当前速度, set=角度环输出 → 输出 PWM */
            motor->pwm_output = (int16_t)PID_calc(&motor->speed_pid,
                                                  motor->current_speed,
                                                  motor->target_speed);
            DRV8873_Set_Speed(drv, motor->pwm_output);
            break;

        default:
            motor->pwm_output = 0;
            break;
        }
    }
}

