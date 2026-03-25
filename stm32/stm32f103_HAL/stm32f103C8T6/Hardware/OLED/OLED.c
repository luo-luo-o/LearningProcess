#include "main.h" 
#include "OLED.h"
#include "OLED_Font.h"

// 声明 CubeMX 生成的 I2C 句柄
extern I2C_HandleTypeDef hi2c2;

static uint8_t OLED_DisplayBuf[1024];

#define OLED_ADDR 0x78 // OLED 的 I2C 地址

/**
 * @brief  向 OLED 发送命令
 */
void OLED_WriteCommand(uint8_t Command)
{
  uint8_t buffer[2] = {0x00, Command}; // 0x00 表示后面是命令
  HAL_I2C_Master_Transmit(&hi2c2, OLED_ADDR, buffer, 2, HAL_MAX_DELAY);
}

/**
 * @brief  通过 DMA 将显存内容刷新到屏幕
 * @note   建议在 main 循环中每 50ms-100ms 调用一次
 */
void OLED_Update(void)
{
  // 只有当 I2C 处于 READY 状态（上一帧传完了）才开启新的一帧
  if (hi2c2.State == HAL_I2C_STATE_READY)
  {
    // 设置光标回到 (0,0)
    static uint8_t cursor_cmd[] = {0x21, 0x00, 0x7F, 0x22, 0x00, 0x07};
    // 这里超时时间设短一点（10ms），防止干扰导致死等
    if (HAL_I2C_Master_Transmit(&hi2c2, OLED_ADDR, cursor_cmd, 6, 10) == HAL_OK)
    {
      // 启动 DMA
      HAL_I2C_Mem_Write_DMA(&hi2c2, OLED_ADDR, 0x40, I2C_MEMADD_SIZE_8BIT, OLED_DisplayBuf, 1024);
    }
  }
}

bool _check_range(uint8_t Line, uint8_t Column)
{
  if (Line < 1 || Line > ROW || Column < 1 || Column > COL)
  {
    return false;
  }
  return true;

}

/**
  * @brief  OLED清屏
*/
void OLED_Clear(void)
{
  memset(OLED_DisplayBuf, 0, 1024); // 极快，不阻塞 PID
}

/**
 * @brief  OLED显示一个字符
 * @param  Line 行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Char 要显示的一个字符，范围：ASCII可见字符
 * @retval 无
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
  if (!_check_range(Line, Column))
  {
    // Error_Handler();
    return;
  }

  uint8_t i;
  uint16_t base_idx;
  uint8_t char_idx = Char - ' ';

  // 计算显存中的起始位置
  // 每行占 2 页 (16像素高)，每列占 8 像素宽

  // 上半部分 (8字节)
  base_idx = ((Line - 1) * 2) * 128 + (Column - 1) * 8;
  for (i = 0; i < 8; i++)
  {
    OLED_DisplayBuf[base_idx + i] = OLED_F8x16[char_idx][i];
  }

  // 下半部分 (8字节)
  base_idx = ((Line - 1) * 2 + 1) * 128 + (Column - 1) * 8;
  for (i = 0; i < 8; i++)
  {
    OLED_DisplayBuf[base_idx + i] = OLED_F8x16[char_idx][i + 8];
  }
}

/**
 * @brief  OLED显示字符串
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  String 要显示的字符串，范围：ASCII可见字符
 * @retval 无
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
  uint8_t i;
  for (i = 0; String[i] != '\0'; i++)
  {
    OLED_ShowChar(Line, Column + i, String[i]);
  }
}

/**
 * @brief  OLED次方函数
 * @retval 返回值等于X的Y次方
 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
  uint32_t Result = 1;
  while (Y--)
  {
    Result *= X;
  }
  return Result;
}

/**
 * @brief  OLED显示数字（十进制，正数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：0~4294967295
 * @param  Length 要显示数字的长度，范围：1~10
 * @retval 无
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
  uint8_t i;
  for (i = 0; i < Length; i++)
  {
    OLED_ShowChar(Line, Column + i,
                  Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
  }
}

/**
 * @brief  OLED显示数字（十进制，带符号数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：-2147483648~2147483647
 * @param  Length 要显示数字的长度，范围：1~10
 * @retval 无
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number,
                        uint8_t Length)
{
  uint8_t i;
  uint32_t Number1;
  if (Number >= 0)
  {
    OLED_ShowChar(Line, Column, '+');
    Number1 = Number;
  }
  else
  {
    OLED_ShowChar(Line, Column, '-');
    Number1 = -Number;
  }
  for (i = 0; i < Length; i++)
  {
    OLED_ShowChar(Line, Column + i + 1,
                  Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
  }
}

/**
 * @brief  OLED显示数字（十六进制，正数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
 * @param  Length 要显示数字的长度，范围：1~8
 * @retval 无
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number,
                     uint8_t Length)
{
  uint8_t i, SingleNumber;
  for (i = 0; i < Length; i++)
  {
    SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
    if (SingleNumber < 10)
    {
      OLED_ShowChar(Line, Column + i, SingleNumber + '0');
    }
    else
    {
      OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
    }
  }
}

/**
 * @brief  OLED显示数字（二进制，正数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
 * @param  Length 要显示数字的长度，范围：1~16
 * @retval 无
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number,
                     uint8_t Length)
{
  uint8_t i;
  for (i = 0; i < Length; i++)
  {
    OLED_ShowChar(Line, Column + i,
                  Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
  }
}

void OLED_Init(void)
{
  HAL_Delay(100);

  // I2C 端口初始化已由 CubeMX 在 main() 中通过 MX_I2C1_Init() 完成
  // 此处直接发送初始化序列
  OLED_WriteCommand(0xAE);
  OLED_WriteCommand(0xD5);
  OLED_WriteCommand(0x80);
  OLED_WriteCommand(0xA8);
  OLED_WriteCommand(0x3F);
  OLED_WriteCommand(0xD3);
  OLED_WriteCommand(0x00);
  OLED_WriteCommand(0x40);
  OLED_WriteCommand(0xA1);
  OLED_WriteCommand(0xC8);
  OLED_WriteCommand(0xDA);
  OLED_WriteCommand(0x12);
  OLED_WriteCommand(0x81);
  OLED_WriteCommand(0xCF);
  OLED_WriteCommand(0xD9);
  OLED_WriteCommand(0xF1);
  OLED_WriteCommand(0xDB);
  OLED_WriteCommand(0x30);
  OLED_WriteCommand(0xA4);
  OLED_WriteCommand(0xA6);
  OLED_WriteCommand(0x8D);
  OLED_WriteCommand(0x14);

  // reset
  // 1. 先设置为页寻址模式 (Page Addressing Mode)
  OLED_WriteCommand(0x20);
  OLED_WriteCommand(0x02);

  // 2. 显式归位 (Page 0, Column 0)
  OLED_WriteCommand(0xB0); // Page 0
  OLED_WriteCommand(0x00); // Low column 0
  OLED_WriteCommand(0x10); // High column 0

  OLED_WriteCommand(0x20);
  OLED_WriteCommand(0x00);

  OLED_WriteCommand(0xAF);
  OLED_Clear();
  OLED_Update();
}