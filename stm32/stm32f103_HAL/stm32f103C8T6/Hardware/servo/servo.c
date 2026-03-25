#include "main.h"
#include "servo.h"

/**
 * @brief 初始化电机外设
 * @param servo 电机控制结构体指针
 * @param pwm_tim PWM 定时器句柄
 * @param pwm_ch PWM 通道
 * @param enc_tim 编码器定时器句柄
 * @param dir1_port 方向控制引脚1的 GPIO 端口
  * @param dir1_pin 方向控制引脚1的 GPIO 引脚号
  * @param dir2_port 方向控制引脚2的 GPIO 端口
  * @param dir2_pin 方向控制引脚2的 GPIO 引脚号
 */
void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch, TIM_HandleTypeDef *enc_tim,
                GPIO_TypeDef *dir1_port, uint16_t dir1_pin, GPIO_TypeDef *dir2_port, uint16_t dir2_pin)
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
  servo->min_speed = 250;
  servo->max_speed = 1000;

  // 启动硬件
  HAL_TIM_PWM_Start(servo->pwm_tim, servo->pwm_channel);
  HAL_TIM_Encoder_Start(servo->enc_tim, TIM_CHANNEL_ALL);
}

/**
 * @brief 更新位置信息（通用部分：处理 16 位溢出和累计增量）
 */
void Servo_UpdatePos(Servo_t *servo)
{
  // 获取当前硬件计数值 (0~65535)
  int16_t current_val = (int16_t)__HAL_TIM_GET_COUNTER(servo->enc_tim);

  // 利用 int16_t 的溢出特性自动计算差值（即使从 0 减到 65535 也会正确得到 -1）
  int16_t delta_speed = current_val - servo->last_enc_val;
  servo->delta_speed = -delta_speed;

  // 累计到长整型位移中
  servo->total_count += servo->delta_speed;

  // 更新记录
  servo->last_enc_val = current_val;
}

/**
 * @brief 设置 PWM 占空比
 * @param duty 占空比数值，范围应在 0 ~ ARR 之间
 */
void Servo_SetPWM(Servo_t *servo, int16_t duty)
{
  if (duty < 0)
    duty = 0;
  __HAL_TIM_SET_COMPARE(servo->pwm_tim, servo->pwm_channel, duty);
}

/**
 * @brief 紧急停止
 */
void Servo_Stop(Servo_t *servo)
{
  __HAL_TIM_SET_COMPARE(servo->pwm_tim, servo->pwm_channel, 0);
  HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_RESET);
}

/**
 * @brief 设置电机转速和方向
 * @param speed 速度值，正数表示正转，负数表示反转，绝对值越大速度越快
 */
void Servo_SetSpeed(Servo_t *servo, int16_t speed)
{
  // 限制速度范围（PWM 周期为 1000） 
  if (speed > servo->max_speed) speed = servo->max_speed;
  if (speed < -servo->max_speed) speed = -servo->max_speed;

  if (speed > 0 && speed < servo->min_speed) speed = servo->min_speed; // 正转最低速度
  if (speed < 0 && speed > -servo->min_speed) speed = -servo->min_speed; // 反转最低速度

  if (speed > 0) {
    // 正转
    HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_RESET);
    Servo_SetPWM(servo, speed);
  } else if (speed < 0) {
    // 反转
    HAL_GPIO_WritePin(servo->dir1_port, servo->dir1_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(servo->dir2_port, servo->dir2_pin, GPIO_PIN_SET);
    Servo_SetPWM(servo, -speed); // 占空比为正数
  } else {
    // 停止
    Servo_Stop(servo);
  }
}

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float output_min, float output_max, float integral_max)
{
  pid->Kp = Kp;
  pid->Ki = Ki;
  pid->Kd = Kd;
  pid->Target = 0;
  pid->Current = 0;
  pid->Error = 0;
  pid->Last_Error = 0;
  pid->Pre_Error = 0;
  pid->Integral = 0;
  pid->Output = 0;
  pid->Output_Max = output_max;
  pid->Output_Min = output_min;
  pid->Integral_Max = integral_max;
}

void PID_Set_Kp(PID_TypeDef *pid, float Kp)
{
  pid->Kp = Kp;
}

void PID_Set_Ki(PID_TypeDef *pid, float Ki)
{
  pid->Ki = Ki;
}

void PID_Set_Kd(PID_TypeDef *pid, float Kd)
{
  pid->Kd = Kd;
}

float PID_Position_Calc(PID_TypeDef *pid, float target, float current)
{
  pid->Target = target;
  pid->Current = current;
  pid->Error = pid->Target - pid->Current;

  // 1. 积分累加（带限幅，防止积分过冲）
  pid->Integral += pid->Error;
  if (pid->Integral > pid->Integral_Max)
    pid->Integral = pid->Integral_Max;
  if (pid->Integral < -pid->Integral_Max)
    pid->Integral = -pid->Integral_Max;

  // 2. 计算公式: Output = Kp*e + Ki*sum(e) + Kd*(e - e_last)
  pid->Output = pid->Kp * pid->Error + pid->Ki * pid->Integral + pid->Kd * (pid->Error - pid->Last_Error);

  pid->Last_Error = pid->Error;

  // 3. 输出限幅
  if (pid->Output > pid->Output_Max)
    pid->Output = pid->Output_Max;
  if (pid->Output < -pid->Output_Max)
    pid->Output = -pid->Output_Max;

  return pid->Output;
}