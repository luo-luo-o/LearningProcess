#include "pid.h"
#include <stdlib.h>
#include <string.h>

static int PID_FormatFloat3(char *buf, size_t buf_size, float value)
{
    int32_t value_int = (int32_t)value;
    int32_t value_dec = abs((int32_t)((value - value_int) * 1000.0f));
    const char *sign = "";

    if (value < 0.0f && value_int == 0)
    {
        sign = "-";
    }

    return snprintf(buf, buf_size, "%s%ld.%03ld", sign, (long)value_int, (long)value_dec);
}

static int PID_AppendFloat3(char *buf, size_t buf_size, int offset, float value)
{
    int written;
    size_t remaining;

    if (offset < 0 || (size_t)offset >= buf_size)
    {
        return offset;
    }

    remaining = buf_size - (size_t)offset;
    written = PID_FormatFloat3(&buf[offset], remaining, value);
    if (written < 0)
    {
        return offset;
    }

    if ((size_t)written >= remaining)
    {
        return (int)buf_size - 1;
    }

    return offset + written;
}

static int PID_AppendText(char *buf, size_t buf_size, int offset, const char *text)
{
    int written;
    size_t remaining;

    if (offset < 0 || (size_t)offset >= buf_size)
    {
        return offset;
    }

    remaining = buf_size - (size_t)offset;
    written = snprintf(&buf[offset], remaining, "%s", text);
    if (written < 0)
    {
        return offset;
    }

    if ((size_t)written >= remaining)
    {
        return (int)buf_size - 1;
    }

    return offset + written;
}

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
    pid->Output = 0.0f;
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

    pid->Output = output;

    return pid->Output;
}

/**
 * @brief 自动上报数据到 VOFA+ (FireWater 格式) - 禁用浮点打印的高效绕过版本
 * @note  保留 3 位小数，格式：目标值,当前值\n
 */
void PID_SendToVofa(PID_TypeDef *pid)
{
    if (pid->vofa_uart == NULL) return;

    char tx_buf[160];
    int len = 0;

    // FireWater: Target,Current,Error,Output,Kp,Ki,Kd
    len = PID_AppendFloat3(tx_buf, sizeof(tx_buf), len, pid->Target);
    len = PID_AppendText(tx_buf, sizeof(tx_buf), len, ",");
    len = PID_AppendFloat3(tx_buf, sizeof(tx_buf), len, pid->Current);
    len = PID_AppendText(tx_buf, sizeof(tx_buf), len, ",");
    len = PID_AppendFloat3(tx_buf, sizeof(tx_buf), len, pid->Error);
    len = PID_AppendText(tx_buf, sizeof(tx_buf), len, ",");
    len = PID_AppendFloat3(tx_buf, sizeof(tx_buf), len, pid->Output);
    len = PID_AppendText(tx_buf, sizeof(tx_buf), len, ",");
    len = PID_AppendFloat3(tx_buf, sizeof(tx_buf), len, pid->Kp);
    len = PID_AppendText(tx_buf, sizeof(tx_buf), len, ",");
    len = PID_AppendFloat3(tx_buf, sizeof(tx_buf), len, pid->Ki);
    len = PID_AppendText(tx_buf, sizeof(tx_buf), len, ",");
    len = PID_AppendFloat3(tx_buf, sizeof(tx_buf), len, pid->Kd);
    len = PID_AppendText(tx_buf, sizeof(tx_buf), len, "\n");

    if ((size_t)len >= sizeof(tx_buf))
    {
        len = sizeof(tx_buf) - 1;
    }

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
