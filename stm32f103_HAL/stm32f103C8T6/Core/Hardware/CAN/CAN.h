#ifndef __CAN_H
#define __CAN_H

#include "stm32f1xx_hal_def.h"
void CAN_Init(void);
HAL_StatusTypeDef CAN_Send_Msg(uint32_t id, uint8_t *data, uint8_t len);
HAL_StatusTypeDef CAN_Send_String(uint32_t id, const char *str);
HAL_StatusTypeDef CAN_Send_Num(uint32_t id, uint32_t num);

#endif /* CAN_H */