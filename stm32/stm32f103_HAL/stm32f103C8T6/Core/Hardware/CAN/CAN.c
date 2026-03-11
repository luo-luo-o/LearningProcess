#include "main.h"
#include <stdint.h>
#include "CAN.h"

extern CAN_HandleTypeDef hcan;

// 私有变量：发送和接收缓存
static CAN_TxHeaderTypeDef TxHeader;
static uint32_t TxMailbox;

/**
 * @brief 封装好的CAN初始化（含过滤器和中断开启）
 */
void CAN_Init(void)
{
  // 1. 配置过滤器（全通模式）
  CAN_FilterTypeDef sFilterConfig = {.FilterBank = 0,
                                     .FilterMode = CAN_FILTERMODE_IDMASK,
                                     .FilterScale = CAN_FILTERSCALE_32BIT,
                                     .FilterIdHigh = 0x0000,
                                     .FilterIdLow = 0x0000,
                                     .FilterMaskIdHigh = 0x0000,
                                     .FilterMaskIdLow = 0x0000,
                                     .FilterFIFOAssignment = CAN_RX_FIFO0,
                                     .FilterActivation = ENABLE,
                                     .SlaveStartFilterBank = 14};
  HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);

  // 2. 启动 CAN
  HAL_CAN_Start(&hcan);

  // 3. 开启接收中断
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/**
 * @brief 封装好的发送函数
 */
HAL_StatusTypeDef CAN_Send_Msg(uint32_t id, uint8_t *data, uint8_t len)
{
  TxHeader.StdId = id;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = (len > 8) ? 8 : len;
  TxHeader.TransmitGlobalTime = DISABLE;

  // 等待邮箱空闲
  while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
    ;

  return HAL_CAN_AddTxMessage(&hcan, &TxHeader, data, &TxMailbox);
}

/**
  * @brief  发送字符串的便捷函数
  */
HAL_StatusTypeDef CAN_Send_String(uint32_t id, const char *str)
{
  uint8_t data[8] = {0};
  strncpy((char *)data, str, 8); // 只取前8字节
  return CAN_Send_Msg(id, data, strlen((char *)data));
}

/**
  * @brief  发送数字的便捷函数（以字符串形式）
  */
HAL_StatusTypeDef CAN_Send_Num(uint32_t id, uint32_t num)
{
  uint8_t i = 0;
  uint8_t rdata[8] = {0};
  uint8_t data[8] = {0};

  // 特殊处理数字 0
  if (num == 0)
  {
    data[0] = '0';
    return CAN_Send_Msg(id, data, 1);
  }

  while(num > 0 && i < 8)
  {
    rdata[i] = (num % 10) + '0'; // 转换为字符
    num /= 10;
    i++;
  }

    // 反转字符串
    for (uint8_t j = 0; j < i; j++) {
        data[j] = rdata[i - j - 1];
    }

  return CAN_Send_Msg(id, data, strlen((char *)data));
}