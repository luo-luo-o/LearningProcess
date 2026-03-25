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

  int16_t min_speed; // 电机最低转速
  int16_t max_speed;
} Servo_t;

typedef struct
{
  float Kp, Ki, Kd;   // 比例、积分、微分系数
  float Target;       // 目标值
  float Current;      // 当前实际值
  float Error;        // 当前误差
  float Last_Error;   // 上一次误差
  float Pre_Error;    // 上上次误差（仅增量式需要）
  float Integral;     // 积分累加值（仅位置式需要）
  float Output;       // PID 输出值
  float Output_Max;   // 输出限幅（防止 PWM 溢出）
  float Output_Min;   // 输出下限（可选，防止反向过快）
  float Integral_Max; // 积分限幅（防止积分饱和/抗饱和）
} PID_TypeDef;

void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch,
                TIM_HandleTypeDef *enc_tim, GPIO_TypeDef *dir1_port, uint16_t dir1_pin, GPIO_TypeDef *dir2_port,
                uint16_t dir2_pin);
void Servo_UpdatePos(Servo_t *servo);
void Servo_SetPWM(Servo_t *servo, int16_t duty);
void Servo_Stop(Servo_t *servo);
void Servo_SetSpeed(Servo_t *servo, int16_t speed);

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float output_min ,float output_max, float integral_max);
void PID_Set_Kp(PID_TypeDef *pid, float Kp);
void PID_Set_Ki(PID_TypeDef *pid, float Ki);
void PID_Set_Kd(PID_TypeDef *pid, float Kd);
float PID_Position_Calc(PID_TypeDef *pid, float target, float current);

#endif /* __SERVO_H */