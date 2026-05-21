#ifndef __PID_H
#define __PID_H

#include "main.h" // 支持 UART_HandleTypeDef
#include <stdint.h>
#include <stdio.h>

// 深度集成 VOFA+ FireWater 的 PID 结构体
typedef struct
{
    /* PID 数学参数 */
    float Kp, Ki, Kd;
    float Target;
    float Current;
    float Error;
    float Last_Error;
    float Integral;
    float Output;
    
    /* 限幅参数 */
    float Output_Max;
    float Output_Min;
    float Integral_Max;

    /* VOFA+ 集成组件 */
    UART_HandleTypeDef *vofa_uart; // 绑定的硬件串口句柄
} PID_TypeDef;

/* 核心方法 */
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float output_min, float output_max, float integral_max, UART_HandleTypeDef *huart);
float PID_Calc(PID_TypeDef *pid, float target, float current);

/* VOFA+ FireWater 集成接口 */
void PID_SendToVofa(PID_TypeDef *pid);
void PID_ParseCommand(PID_TypeDef *pid, char *cmd_str);

#endif /* __PID_H */
