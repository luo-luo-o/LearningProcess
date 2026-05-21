#ifndef __SERVO_H
#define __SERVO_H

#include "main.h"
#include "pid.h"   // 引入已经集成了 VOFA+ 的 pid 模块
#include <stdbool.h>

// 电机控制结构体
typedef struct
{
    TIM_HandleTypeDef *pwm_tim;     // PWM 定时器句柄 (如 &htim3)
    uint32_t pwm_channel;           // PWM 通道 (如 TIM_CHANNEL_1)
    TIM_HandleTypeDef *enc_tim;     // 编码器定时器句柄 (如 &htim2)

    GPIO_TypeDef *dir1_port;
    uint16_t dir1_pin;
    GPIO_TypeDef *dir2_port;
    uint16_t dir2_pin;

    int32_t total_count;            // 累计脉冲数位置
    int16_t last_enc_val;           // 上一次硬件计数值
    int16_t delta_speed;            // 本次采样周期内的增量速度
    int16_t max_speed;              // 硬件 ARR 最大限幅
    
    int32_t target_position;        // 目标位置
    bool is_pos_closed_loop;        // 是否开启位置闭环控制锁
    
    PID_TypeDef pos_pid;            // 内部嵌套：集成了 VOFA+ 的位置环 PID
} Servo_t;

/* 核心生命周期接口 */
void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch, TIM_HandleTypeDef *enc_tim,
                GPIO_TypeDef *dir1_port, uint16_t dir1_pin, GPIO_TypeDef *dir2_port, uint16_t dir2_pin, 
                UART_HandleTypeDef *vofa_uart);
void Servo_ConfigPID(Servo_t *servo, float Kp, float Ki, float Kd, float integral_max);
void Servo_UpdatePos(Servo_t *servo);

/* 控制动作接口 */
void Servo_SetTargetPos(Servo_t *servo, int32_t target_pos);
void Servo_SetSpeed(Servo_t *servo, int16_t speed);
void Servo_Stop(Servo_t *servo);

/* 定时器中断核心轮询 Task (内部调用 PID 计算与自动 VOFA 上报) */
void Servo_Task(Servo_t *servo);

#endif /* __SERVO_H */