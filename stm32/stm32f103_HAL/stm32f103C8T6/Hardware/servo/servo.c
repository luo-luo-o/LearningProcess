#include "servo.h"
#include "stm32f1xx_hal_tim.h"

#define SERVO_POSITION_DEADBAND 8
#define SERVO_SPEED_DEADBAND 1

#define SERVO_VOFA_ENABLE_POS_PID 1
#define SERVO_VOFA_ENABLE_SPEED_PID 1

#if PID_ENABLE_VOFA
static int Servo_UartWrite(void *ctx, const uint8_t *data, size_t len)
{
    if (ctx == NULL || data == NULL || len == 0U)
    {
        return 0;
    }

    return HAL_UART_Transmit((UART_HandleTypeDef *)ctx, (uint8_t *)data, (uint16_t)len, 10) == HAL_OK;
}

static size_t Servo_CollectVofaPids(Servo_t *servo, PID_TypeDef **pids, size_t max_count)
{
    size_t count = 0U;

    if (servo == NULL || pids == NULL)
    {
        return 0U;
    }

#if SERVO_VOFA_ENABLE_POS_PID
    if (count < max_count)
    {
        pids[count++] = &servo->pos_pid;
    }
#endif

#if SERVO_VOFA_ENABLE_SPEED_PID
    if (count < max_count)
    {
        pids[count++] = &servo->speed_pid;
    }
#endif

    return count;
}
#endif

static void Servo_ApplySpeed(Servo_t *servo, int16_t speed)
{
    if (speed > servo->max_speed)  speed = servo->max_speed;
    if (speed < -servo->max_speed) speed = -servo->max_speed;

    if (speed > 0) {
        HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(servo->pwm_tim, servo->pwm_channel, speed);
    } else if (speed < 0) {
        HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(servo->pwm_tim, servo->pwm_channel, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(servo->pwm_tim, servo->pwm_channel, 0);
        HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_RESET);
    }
}

static int16_t Servo_ClampToInt16(float value, int16_t limit)
{
    if (limit < 0)
    {
        limit = -limit;
    }
    if (value > (float)limit)
    {
        return limit;
    }
    if (value < (float)-limit)
    {
        return -limit;
    }
    return (int16_t)value;
}

static bool Servo_IsInDeadband(Servo_t *servo)
{
    int32_t position_error = servo->target_position - servo->total_count;
    int16_t speed = servo->delta_speed;

    if (position_error < 0)
    {
        position_error = -position_error;
    }
    if (speed < 0)
    {
        speed = -speed;
    }

    return position_error <= SERVO_POSITION_DEADBAND && speed <= SERVO_SPEED_DEADBAND;
}

/**
 * @brief 初始化电机外设状态，并绑定遥测串口
 * @param vofa_uart 用于 VOFA+ 遥测输出的串口句柄
 */
void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch, TIM_HandleTypeDef *enc_tim,
                GPIO_TypeDef *dir1_port, uint16_t dir1_pin, GPIO_TypeDef *dir2_port, uint16_t dir2_pin,
                UART_HandleTypeDef *vofa_uart)
{
    servo->pwm_tim = pwm_tim;
    servo->pwm_channel = pwm_ch;
    servo->enc_tim = enc_tim;
    servo->total_count = 0;
    servo->last_enc_val = 0U;
    servo->delta_speed = 0;
    servo->dir1_pin = dir1_pin;
    servo->dir1_port = dir1_port;
    servo->dir2_pin = dir2_pin;
    servo->dir2_port = dir2_port;
#if PID_ENABLE_VOFA
    servo->telemetry_comm.write = Servo_UartWrite;
    servo->telemetry_comm.ctx = vofa_uart;
#else
    (void)vofa_uart;
#endif
    
    // 获取定时器的 ARR 值作为正向最大限制
    servo->max_speed = (int16_t)__HAL_TIM_GET_AUTORELOAD(pwm_tim);
    
    servo->target_position = 0;
    servo->target_speed = 0;
    servo->max_target_speed = 80;
    servo->pwm_output = 0;
    servo->is_pos_closed_loop = false;

    // 初始化内部 PID，并将串口句柄直接透传给 PID 模块实现自集成
    PID_Init(&servo->pos_pid, 0.0f, 0.0f, 0.0f, (float)(-servo->max_target_speed), (float)(servo->max_target_speed), 0.0f);
    PID_Init(&servo->speed_pid, 0.0f, 0.0f, 0.0f, (float)(-servo->max_speed), (float)(servo->max_speed), 0.0f);

    // 启动硬件外设
    HAL_TIM_PWM_Start(servo->pwm_tim, servo->pwm_channel);
    HAL_TIM_Encoder_Start(servo->enc_tim, TIM_CHANNEL_ALL);
    Servo_ResetEncoder(servo);
}

/**
 * @brief 配置伺服内部参数
 */
void Servo_ConfigPID(Servo_t *servo, float Kp, float Ki, float Kd, float integral_max)
{
    servo->pos_pid.Kp = Kp;
    servo->pos_pid.Ki = Ki;
    servo->pos_pid.Kd = Kd;
    servo->pos_pid.Integral_Max = integral_max;
}

/* Configure inner speed PID. */
void Servo_ConfigSpeedPID(Servo_t *servo, float Kp, float Ki, float Kd, float integral_max)
{
    servo->speed_pid.Kp = Kp;
    servo->speed_pid.Ki = Ki;
    servo->speed_pid.Kd = Kd;
    servo->speed_pid.Integral_Max = integral_max;
}

void Servo_SetMaxTargetSpeed(Servo_t *servo, int16_t max_target_speed)
{
    if (max_target_speed < 0)
    {
        max_target_speed = -max_target_speed;
    }
    if (max_target_speed == 0)
    {
        max_target_speed = 1;
    }

    servo->max_target_speed = max_target_speed;
    servo->pos_pid.Output_Min = (float)-max_target_speed;
    servo->pos_pid.Output_Max = (float)max_target_speed;
    servo->target_speed = Servo_ClampToInt16((float)servo->target_speed, servo->max_target_speed);
}

void Servo_ResetEncoder(Servo_t *servo)
{
    if (servo == NULL)
    {
        return;
    }

    __HAL_TIM_SET_COUNTER(servo->enc_tim, 0U);
    servo->last_enc_val = 0U;
    servo->delta_speed = 0;
    servo->total_count = 0;
}

/* Update encoder-derived position and speed. */
void Servo_UpdatePos(Servo_t *servo)
{
    uint16_t current_val = (uint16_t)__HAL_TIM_GET_COUNTER(servo->enc_tim);
    int16_t delta_speed = (int16_t)(current_val - servo->last_enc_val);
    
    servo->delta_speed = -delta_speed; 
    servo->total_count += servo->delta_speed;
    servo->last_enc_val = current_val;
}

/**
 * @brief 命令电机去往特定目标位置
 */
void Servo_SetTargetPos(Servo_t *servo, int32_t target_pos)
{
    servo->target_position = target_pos;
    PID_Reset(&servo->pos_pid);
    PID_Reset(&servo->speed_pid);
    servo->is_pos_closed_loop = true;
}

/**
 * @brief 底层驱动：设置占空比
 */
void Servo_SetSpeed(Servo_t *servo, int16_t speed)
{
    if (speed == 0) {
        Servo_Stop(servo);
    } else {
        Servo_ApplySpeed(servo, speed);
    }
}

/**
 * @brief 停止电机
 */
void Servo_Stop(Servo_t *servo)
{
    servo->is_pos_closed_loop = false;
    servo->target_speed = 0;
    servo->pwm_output = 0;
    PID_Reset(&servo->pos_pid);
    PID_Reset(&servo->speed_pid);
    __HAL_TIM_SET_COMPARE(servo->pwm_tim, servo->pwm_channel, 0);
    HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_RESET);
}

/**
 * @brief 核心周期控制任务 (应在 TIM4 10ms 中断中调用)
 * @note  控制任务只更新状态，遥测由主循环调用 Servo_SendTelemetry 发送。
 */
void Servo_Task(Servo_t *servo)
{
    // 1. 无条件刷新最新位置反馈
    Servo_UpdatePos(servo);
    
    // 2. 闭环控制逻辑
    if (servo->is_pos_closed_loop)
    {
        float target_speed = PID_Calc(&servo->pos_pid, (float)servo->target_position, (float)servo->total_count);
        servo->target_speed = Servo_ClampToInt16(target_speed, servo->max_target_speed);

        if (Servo_IsInDeadband(servo))
        {
            servo->target_speed = 0;
            servo->pwm_output = 0;
            servo->pos_pid.Output = 0.0f;
            PID_Reset(&servo->speed_pid);
        }
        else
        {
            float pwm_out = PID_Calc(&servo->speed_pid, (float)servo->target_speed, (float)servo->delta_speed);
            servo->pwm_output = Servo_ClampToInt16(pwm_out, servo->max_speed);
        }

        Servo_ApplySpeed(servo, servo->pwm_output);
    }
    else
    {
        servo->target_speed = 0;
        servo->pwm_output = 0;
        servo->pos_pid.Target = (float)servo->target_position;
        servo->pos_pid.Current = (float)servo->total_count;
        servo->pos_pid.Error = servo->pos_pid.Target - servo->pos_pid.Current;
        servo->pos_pid.Output = 0.0f;
        servo->speed_pid.Target = 0.0f;
        servo->speed_pid.Current = (float)servo->delta_speed;
        servo->speed_pid.Error = servo->speed_pid.Target - servo->speed_pid.Current;
        servo->speed_pid.Output = 0.0f;
    }
    
}

void Servo_SendTelemetry(Servo_t *servo)
{
#if PID_ENABLE_VOFA
    PID_TypeDef *pids[2];
    size_t pid_count = Servo_CollectVofaPids(servo, pids, sizeof(pids) / sizeof(pids[0]));

    (void)PID_VofaSendListAuto(&servo->telemetry_comm, pids, pid_count);
#else
    (void)servo;
#endif
}

int Servo_ParsePidCommand(Servo_t *servo, const char *cmd_str)
{
#if PID_ENABLE_VOFA
    PID_TypeDef *pids[2];
    size_t pid_count = Servo_CollectVofaPids(servo, pids, sizeof(pids) / sizeof(pids[0]));

    return PID_ParseIndexedCommand(pids, pid_count, cmd_str);
#else
    (void)servo;
    (void)cmd_str;
    return 0;
#endif
}
