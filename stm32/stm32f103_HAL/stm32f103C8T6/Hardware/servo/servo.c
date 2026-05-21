#include "servo.h"
#include "stm32f1xx_hal_tim.h"

/**
 * @brief 初始化电机外设状态，并为内部 PID 绑定 VOFA+ 串口
 * @param vofa_uart 专门传给内部 PID 模块使用的 VOFA 串口句柄
 */
void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch, TIM_HandleTypeDef *enc_tim,
                GPIO_TypeDef *dir1_port, uint16_t dir1_pin, GPIO_TypeDef *dir2_port, uint16_t dir2_pin,
                UART_HandleTypeDef *vofa_uart)
{
    servo->pwm_tim = pwm_tim;
    servo->pwm_channel = pwm_ch;
    servo->enc_tim = enc_tim;
    servo->total_count = 0;
    servo->last_enc_val = 0;
    servo->delta_speed = 0;
    servo->dir1_pin = dir1_pin;
    servo->dir1_port = dir1_port;
    servo->dir2_pin = dir2_pin;
    servo->dir2_port = dir2_port;
    
    // 获取定时器的 ARR 值作为正向最大限制
    servo->max_speed = (int16_t)__HAL_TIM_GET_AUTORELOAD(pwm_tim);
    
    servo->target_position = 0;
    servo->is_pos_closed_loop = false;

    // 初始化内部 PID，并将串口句柄直接透传给 PID 模块实现自集成
    PID_Init(&servo->pos_pid, 0.0f, 0.0f, 0.0f, (float)(-servo->max_speed), (float)(servo->max_speed), 0.0f, vofa_uart);

    // 启动硬件外设
    HAL_TIM_PWM_Start(servo->pwm_tim, servo->pwm_channel);
    HAL_TIM_Encoder_Start(servo->enc_tim, TIM_CHANNEL_ALL);
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

/**
 * @brief 更新脉冲位置信息
 */
void Servo_UpdatePos(Servo_t *servo)
{
    int16_t current_val = (int16_t)__HAL_TIM_GET_COUNTER(servo->enc_tim);
    int16_t delta_speed = current_val - servo->last_enc_val;
    
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
    servo->is_pos_closed_loop = true;
}

/**
 * @brief 底层驱动：设置占空比
 */
void Servo_SetSpeed(Servo_t *servo, int16_t speed)
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
        Servo_Stop(servo);
    }
}

/**
 * @brief 停止电机
 */
void Servo_Stop(Servo_t *servo)
{
    servo->is_pos_closed_loop = false;
    __HAL_TIM_SET_COMPARE(servo->pwm_tim, servo->pwm_channel, 0);
    HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_RESET);
}

/**
 * @brief 核心周期控制任务 (应在 TIM4 10ms 中断中调用)
 * @note  计算完成后，自动触发内置的 PID_SendToVofa，实现彻底的内部自动化上报。
 */
void Servo_Task(Servo_t *servo)
{
    // 1. 无条件刷新最新位置反馈
    Servo_UpdatePos(servo);
    
    // 2. 闭环控制逻辑
    if (servo->is_pos_closed_loop)
    {
        // 隐式调用底层 PID 运算
        float pid_out = PID_Calc(&servo->pos_pid, (float)servo->target_position, (float)servo->total_count);
        
        if (servo->total_count == servo->target_position) {
            Servo_Stop(servo);
        } else {
            Servo_SetSpeed(servo, (int16_t)pid_out);
        }
    }
    
    // 3. 【核心集成点】计算完成后，PID 自主将当前的 Target 和 Current 吐给 VOFA+
    PID_SendToVofa(&servo->pos_pid);
}