#ifndef __SERVO_H
#define __SERVO_H

// 电机方向枚举
typedef enum
{
  MOTOR_FWD = 0,
  MOTOR_REV = 1
} MotorDir_t;

// 电机控制结构体 (L298N)
typedef struct
{
  TIM_HandleTypeDef *pwm_tim; // PWM 定时器句柄 (如 &htim3)
  uint32_t pwm_channel;       // PWM 通道 (如 TIM_CHANNEL_1)
  TIM_HandleTypeDef *enc_tim; // 编码器定时器句柄 (如 &htim2)

  GPIO_TypeDef *dir1_port;
  uint16_t dir1_pin;
  GPIO_TypeDef *dir2_port;
  uint16_t dir2_pin;

  int32_t total_count;  // 累计脉冲数
  int16_t last_enc_val; // 上一次硬件计数值
  int16_t delta_speed;  // 本次采样周期内的增量
} Servo_t;


void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch,
                TIM_HandleTypeDef *enc_tim, GPIO_TypeDef *dir1_port, uint16_t dir1_pin, GPIO_TypeDef *dir2_port,
                uint16_t dir2_pin);
void Servo_UpdatePos(Servo_t *servo);
void Servo_SetPWM(Servo_t *servo, int16_t duty);
void Servo_Stop(Servo_t *servo);
void Servo_SetSpeed(Servo_t *servo, int16_t speed);

#endif /* __SERVO_H */