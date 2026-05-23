#include "pid.h"
#include <stdio.h>
#include <stdlib.h>

#if PID_ENABLE_VOFA
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

    if (buf == NULL || offset < 0 || (size_t)offset >= buf_size)
    {
        return offset;
    }

    remaining = buf_size - (size_t)offset;
    written = PID_FormatFloat3(&buf[offset], remaining, value);
    if (written < 0 || (size_t)written >= remaining)
    {
        return offset;
    }

    return offset + written;
}

static int PID_AppendText(char *buf, size_t buf_size, int offset, const char *text)
{
    int written;
    size_t remaining;

    if (buf == NULL || text == NULL || offset < 0 || (size_t)offset >= buf_size)
    {
        return offset;
    }

    remaining = buf_size - (size_t)offset;
    written = snprintf(&buf[offset], remaining, "%s", text);
    if (written < 0 || (size_t)written >= remaining)
    {
        return offset;
    }

    return offset + written;
}
#endif

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float output_min, float output_max, float integral_max)
{
    if (pid == NULL)
    {
        return;
    }

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
}

void PID_Reset(PID_TypeDef *pid)
{
    if (pid == NULL)
    {
        return;
    }

    pid->Error = 0.0f;
    pid->Last_Error = 0.0f;
    pid->Integral = 0.0f;
    pid->Output = 0.0f;
}

float PID_Calc(PID_TypeDef *pid, float target, float current)
{
    float output;

    if (pid == NULL)
    {
        return 0.0f;
    }

    pid->Target = target;
    pid->Current = current;
    pid->Error = pid->Target - pid->Current;

    pid->Integral += pid->Error;
    if (pid->Integral > pid->Integral_Max)  pid->Integral = pid->Integral_Max;
    if (pid->Integral < -pid->Integral_Max) pid->Integral = -pid->Integral_Max;

    output = (pid->Kp * pid->Error) + (pid->Ki * pid->Integral) + (pid->Kd * (pid->Error - pid->Last_Error));
    pid->Last_Error = pid->Error;

    if (output > pid->Output_Max) output = pid->Output_Max;
    if (output < pid->Output_Min) output = pid->Output_Min;

    pid->Output = output;
    return pid->Output;
}

#if PID_ENABLE_VOFA
static void PID_VofaAppendSeparator(PID_VofaMsg_t *msg)
{
    int next_len;

    if (msg == NULL || msg->field_count == 0U)
    {
        return;
    }

    next_len = PID_AppendText(msg->buf, msg->size, msg->len, ",");
    if (next_len == msg->len)
    {
        msg->truncated = 1U;
    }
    msg->len = next_len;
}

void PID_VofaBegin(PID_VofaMsg_t *msg, char *buf, size_t size)
{
    if (msg == NULL)
    {
        return;
    }

    msg->buf = buf;
    msg->size = size;
    msg->len = 0;
    msg->field_count = 0U;
    msg->truncated = 0U;

    if (buf != NULL && size > 0U)
    {
        buf[0] = '\0';
    }
}

void PID_VofaAppendFloat(PID_VofaMsg_t *msg, float value)
{
    int next_len;

    if (msg == NULL || msg->buf == NULL || msg->size == 0U || msg->truncated)
    {
        return;
    }

    PID_VofaAppendSeparator(msg);
    next_len = PID_AppendFloat3(msg->buf, msg->size, msg->len, value);
    if (next_len == msg->len)
    {
        msg->truncated = 1U;
    }
    msg->len = next_len;
    msg->field_count++;
}

void PID_VofaAppend(PID_VofaMsg_t *msg, const PID_TypeDef *pid)
{
    if (pid == NULL)
    {
        return;
    }

    PID_VofaAppendFloat(msg, pid->Target);
    PID_VofaAppendFloat(msg, pid->Current);
    PID_VofaAppendFloat(msg, pid->Error);
    PID_VofaAppendFloat(msg, pid->Output);
    PID_VofaAppendFloat(msg, pid->Kp);
    PID_VofaAppendFloat(msg, pid->Ki);
    PID_VofaAppendFloat(msg, pid->Kd);
}

void PID_VofaAppendList(PID_VofaMsg_t *msg, PID_TypeDef *const *pids, size_t pid_count)
{
    size_t i;

    if (pids == NULL)
    {
        return;
    }

    for (i = 0U; i < pid_count; i++)
    {
        PID_VofaAppend(msg, pids[i]);
    }
}

void PID_VofaEnd(PID_VofaMsg_t *msg)
{
    int next_len;

    if (msg == NULL || msg->buf == NULL || msg->size == 0U || msg->truncated)
    {
        return;
    }

    next_len = PID_AppendText(msg->buf, msg->size, msg->len, "\n");
    if (next_len == msg->len)
    {
        msg->truncated = 1U;
    }
    msg->len = next_len;
}

int PID_VofaSend(PID_VofaMsg_t *msg, const PID_Comm_t *comm)
{
    if (msg == NULL || comm == NULL || comm->write == NULL || msg->buf == NULL || msg->len <= 0 || msg->truncated)
    {
        return 0;
    }

    return comm->write(comm->ctx, (const uint8_t *)msg->buf, (size_t)msg->len);
}

int PID_VofaSendList(const PID_Comm_t *comm, PID_TypeDef *const *pids, size_t pid_count, char *buf, size_t size)
{
    PID_VofaMsg_t msg;

    if (comm == NULL || pids == NULL || buf == NULL || size == 0U || pid_count == 0U)
    {
        return 0;
    }

    PID_VofaBegin(&msg, buf, size);
    PID_VofaAppendList(&msg, pids, pid_count);
    PID_VofaEnd(&msg);

    return PID_VofaSend(&msg, comm);
}

int PID_VofaSendListAuto(const PID_Comm_t *comm, PID_TypeDef *const *pids, size_t pid_count)
{
    if (pid_count == 0U || pid_count > PID_VOFA_MAX_AUTO_PID_COUNT)
    {
        return 0;
    }

    {
        char buf[PID_VOFA_MSG_SIZE(pid_count)];

        return PID_VofaSendList(comm, pids, pid_count, buf, sizeof(buf));
    }
}
#endif

static int PID_ParseFloat(const char *text, float *value)
{
    const char *val_ptr = text;
    int has_digit = 0;
    int sign = 1;
    int32_t int_part = 0;
    float dec_part = 0.0f;

    if (text == NULL || value == NULL)
    {
        return 0;
    }

    if (*val_ptr == '-')
    {
        sign = -1;
        val_ptr++;
    }
    else if (*val_ptr == '+')
    {
        val_ptr++;
    }

    while (*val_ptr >= '0' && *val_ptr <= '9')
    {
        has_digit = 1;
        int_part = int_part * 10 + (*val_ptr - '0');
        val_ptr++;
    }

    if (*val_ptr == '.')
    {
        float weight = 0.1f;

        val_ptr++;
        while (*val_ptr >= '0' && *val_ptr <= '9')
        {
            has_digit = 1;
            dec_part += (*val_ptr - '0') * weight;
            weight *= 0.1f;
            val_ptr++;
        }
    }

    if (!has_digit || *val_ptr != '\0')
    {
        return 0;
    }

    *value = (float)sign * ((float)int_part + dec_part);
    return 1;
}

int PID_ParseCommand(PID_TypeDef *pid, const char *cmd_str)
{
    char type;
    float final_val;

    if (pid == NULL || cmd_str == NULL || cmd_str[0] == '\0' || cmd_str[1] != '=')
    {
        return 0;
    }

    type = cmd_str[0];
    if (!PID_ParseFloat(cmd_str + 2, &final_val))
    {
        return 0;
    }

    if (type == 'P')
    {
        pid->Kp = final_val;
        return 1;
    }
    else if (type == 'I')
    {
        pid->Ki = final_val;
        return 1;
    }
    else if (type == 'D')
    {
        pid->Kd = final_val;
        return 1;
    }

    return 0;
}

#if PID_ENABLE_VOFA
int PID_ParseIndexedCommand(PID_TypeDef *const *pids, size_t pid_count, const char *cmd_str)
{
    size_t index = 0U;
    const char *ptr = cmd_str;

    if (pids == NULL || cmd_str == NULL)
    {
        return 0;
    }

    while (*ptr >= '0' && *ptr <= '9')
    {
        index = (index * 10U) + (size_t)(*ptr - '0');
        ptr++;
    }

    if (ptr == cmd_str || index == 0U || index > pid_count || pids[index - 1U] == NULL)
    {
        return 0;
    }

    return PID_ParseCommand(pids[index - 1U], ptr);
}
#endif
