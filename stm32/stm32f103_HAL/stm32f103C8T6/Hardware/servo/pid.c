#include "pid.h"
#include <string.h>

/**
 * @brief 初始化 PID，并绑定 VOFA+ 串口
 */
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float output_min, float output_max, float integral_max, UART_HandleTypeDef *huart)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->Target = 0.0f;
    pid->Current = 0.0f;
    pid->Error = 0.0f;
    pid->Last_Error = 0.0f;
    pid->Integral = 0.0f;
    pid->Output_Min = output_min;
    pid->Output_Max = output_max;
    pid->Integral_Max = integral_max;
    
    pid->vofa_uart = huart; // 绑定串口
}

/**
 * @brief PID 核心计算
 */
float PID_Calc(PID_TypeDef *pid, float target, float current)
{
    pid->Target = target;
    pid->Current = current;
    pid->Error = pid->Target - pid->Current;

    pid->Integral += pid->Error;
    if (pid->Integral > pid->Integral_Max)  pid->Integral = pid->Integral_Max;
    if (pid->Integral < -pid->Integral_Max) pid->Integral = -pid->Integral_Max;

    float output = (pid->Kp * pid->Error) + (pid->Ki * pid->Integral) + (pid->Kd * (pid->Error - pid->Last_Error));
    pid->Last_Error = pid->Error;

    if (output > pid->Output_Max) output = pid->Output_Max;
    if (output < pid->Output_Min) output = pid->Output_Min;

    return output;
}

/**
 * @brief 自动上报数据到 VOFA+ (FireWater 格式) - 禁用浮点打印的高效绕过版本
 * @note  保留 3 位小数，格式：目标值,当前值\n
 */
void PID_SendToVofa(PID_TypeDef *pid)
{
    if (pid->vofa_uart == NULL) return;

    char tx_buf[64];
    
    // 1. 拆分 Target (目标值) 的整数与 3 位小数
    int32_t target_int = (int32_t)pid->Target;
    // 乘以 1000 保留三位，取绝对值防止出现类似 "0.-125" 的错误
    int32_t target_dec = abs((int32_t)((pid->Target - target_int) * 1000)); 

    // 2. 拆分 Current (当前值) 的整数与 3 位小数
    int32_t current_int = (int32_t)pid->Current;
    int32_t current_dec = abs((int32_t)((pid->Current - current_int) * 1000));

    // 3. 针对负数情况的特殊边界优化（处理 -0.XXX 的情况）
    // 当浮点数在 -1.0 到 0.0 之间时，整数部分为 0，负号会丢失，需要手动补上
    char target_sign[2] = "";
    if (pid->Target < 0 && target_int == 0) {
        target_sign[0] = '-';
        target_sign[1] = '\0';
    }
    
    char current_sign[2] = "";
    if (pid->Current < 0 && current_int == 0) {
        current_sign[0] = '-';
        current_sign[1] = '\0';
    }

    // 4. 组合为 FireWater 字符串文本
    // %03d 的意思是：如果小数部分不足 3 位（比如 0.005 乘 1000 得到 5），前面自动补 0 变成 "005"
    int len = sprintf(tx_buf, "%s%ld.%03ld,%s%ld.%03ld\n", 
                      target_sign, target_int, target_dec,
                      current_sign, current_int, current_dec);

    // 5. 串口发送纯文本
    HAL_UART_Transmit(pid->vofa_uart, (uint8_t*)tx_buf, len, 10);
}

/**
 * @brief PID 自解析调参字符串
 * @param cmd_str 传入的清洗后的字符串，如 "P=1.234567" 或 "I=0.500000"
 */
/**
 * @brief PID 自解析调参字符串（避开 sscanf 匹配 Bug 稳定版本）
 */
void PID_ParseCommand(PID_TypeDef *pid, char *cmd_str)
{
    // 基础合法性校验：防止空指针或错乱格式
    if (cmd_str == NULL || cmd_str[0] == '\0' || cmd_str[1] != '=') {
        return;
    }

    char type = cmd_str[0];      // 提取 'P', 'I', 'D'
    char *val_ptr = cmd_str + 2; // 指向数字起始位置

    // 1. 符号位解析
    int sign = 1;
    if (*val_ptr == '-') {
        sign = -1;
        val_ptr++;
    } else if (*val_ptr == '+') {
        val_ptr++;
    }

    // 2. 解析整数部分
    int32_t int_part = 0;
    while (*val_ptr >= '0' && *val_ptr <= '9') {
        int_part = int_part * 10 + (*val_ptr - '0');
        val_ptr++;
    }

    // 3. 解析小数部分
    float dec_part = 0.0f;
    if (*val_ptr == '.') {
        val_ptr++;
        float weight = 0.1f;
        while (*val_ptr >= '0' && *val_ptr <= '9') {
            dec_part += (*val_ptr - '0') * weight;
            weight *= 0.1f;
            val_ptr++;
        }
    }

    // 4. 组合最终的高精度浮点数值
    float final_val = (float)sign * ((float)int_part + dec_part);

    // 5. 安全精确分发
    if (type == 'P') {
        pid->Kp = final_val;
    } else if (type == 'I') {
        pid->Ki = final_val;
    } else if (type == 'D') {
        pid->Kd = final_val;
    }
}