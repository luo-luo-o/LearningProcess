#ifndef __PID_H
#define __PID_H

#include <stddef.h>
#include <stdint.h>

#ifndef PID_ENABLE_VOFA
#define PID_ENABLE_VOFA 1
#endif

typedef struct
{
    float Kp, Ki, Kd;
    float Target;
    float Current;
    float Error;
    float Last_Error;
    float Integral;
    float Output;

    float Output_Max;
    float Output_Min;
    float Integral_Max;
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float output_min, float output_max, float integral_max);
void PID_Reset(PID_TypeDef *pid);
float PID_Calc(PID_TypeDef *pid, float target, float current);
int PID_ParseCommand(PID_TypeDef *pid, const char *cmd_str);

#if PID_ENABLE_VOFA
#define PID_VOFA_FIELDS_PER_PID 7U
#define PID_VOFA_FLOAT_FIELD_BYTES 20U
#define PID_VOFA_MSG_SIZE(pid_count) \
    (((pid_count) * PID_VOFA_FIELDS_PER_PID * PID_VOFA_FLOAT_FIELD_BYTES) + ((pid_count) * PID_VOFA_FIELDS_PER_PID) + 2U)

#ifndef PID_VOFA_MAX_AUTO_PID_COUNT
#define PID_VOFA_MAX_AUTO_PID_COUNT 4U
#endif

typedef int (*PID_WriteFunc_t)(void *ctx, const uint8_t *data, size_t len);

typedef struct
{
    PID_WriteFunc_t write;
    void *ctx;
} PID_Comm_t;

typedef struct
{
    char *buf;
    size_t size;
    int len;
    uint8_t field_count;
    uint8_t truncated;
} PID_VofaMsg_t;

void PID_VofaBegin(PID_VofaMsg_t *msg, char *buf, size_t size);
void PID_VofaAppendFloat(PID_VofaMsg_t *msg, float value);
void PID_VofaAppend(PID_VofaMsg_t *msg, const PID_TypeDef *pid);
void PID_VofaAppendList(PID_VofaMsg_t *msg, PID_TypeDef *const *pids, size_t pid_count);
void PID_VofaEnd(PID_VofaMsg_t *msg);
int PID_VofaSend(PID_VofaMsg_t *msg, const PID_Comm_t *comm);
int PID_VofaSendList(const PID_Comm_t *comm, PID_TypeDef *const *pids, size_t pid_count, char *buf, size_t size);
int PID_VofaSendListAuto(const PID_Comm_t *comm, PID_TypeDef *const *pids, size_t pid_count);
int PID_ParseIndexedCommand(PID_TypeDef *const *pids, size_t pid_count, const char *cmd_str);
#endif

#endif /* __PID_H */
