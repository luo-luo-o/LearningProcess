#ifndef __SERVO_H
#define __SERVO_H

#include "main.h"
#include "pid.h"
#include <stdbool.h>

typedef struct
{
    TIM_HandleTypeDef *pwm_tim;
    uint32_t pwm_channel;
    TIM_HandleTypeDef *enc_tim;

    GPIO_TypeDef *dir1_port;
    uint16_t dir1_pin;
    GPIO_TypeDef *dir2_port;
    uint16_t dir2_pin;

    int32_t total_count;
    uint16_t last_enc_val;
    int16_t delta_speed;
    int16_t max_speed;

    int32_t target_position;
    int16_t target_speed;
    int16_t max_target_speed;
    int16_t pwm_output;
    bool is_pos_closed_loop;

    PID_TypeDef pos_pid;
    PID_TypeDef speed_pid;
#if PID_ENABLE_VOFA
    PID_Comm_t telemetry_comm;
#endif
} Servo_t;

void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch, TIM_HandleTypeDef *enc_tim,
                GPIO_TypeDef *dir1_port, uint16_t dir1_pin, GPIO_TypeDef *dir2_port, uint16_t dir2_pin,
                UART_HandleTypeDef *vofa_uart);
void Servo_ConfigPID(Servo_t *servo, float Kp, float Ki, float Kd, float integral_max);
void Servo_ConfigSpeedPID(Servo_t *servo, float Kp, float Ki, float Kd, float integral_max);
void Servo_SetMaxTargetSpeed(Servo_t *servo, int16_t max_target_speed);
void Servo_ResetEncoder(Servo_t *servo);
void Servo_UpdatePos(Servo_t *servo);

void Servo_SetTargetPos(Servo_t *servo, int32_t target_pos);
void Servo_SetSpeed(Servo_t *servo, int16_t speed);
void Servo_Stop(Servo_t *servo);
void Servo_SendTelemetry(Servo_t *servo);
int Servo_ParsePidCommand(Servo_t *servo, const char *cmd_str);
void Servo_Task(Servo_t *servo);

#endif /* __SERVO_H */
