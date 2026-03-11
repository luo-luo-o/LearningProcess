#include "main.h" 
#include "OLED.h"
#include "OLED_Font.h"

// 声明 CubeMX 生成的 I2C 句柄
extern I2C_HandleTypeDef hi2c2;

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
 * @brief  向 OLED 发送数据
 */
void OLED_WriteData(uint8_t Data)
{
  uint8_t buffer[2] = {0x40, Data}; // 0x40 表示后面是数据
  HAL_I2C_Master_Transmit(&hi2c2, OLED_ADDR, buffer, 2, HAL_MAX_DELAY);
}

/* --- 以下逻辑部分与标准库基本一致，仅移除了底层模拟逻辑 --- */

void OLED_SetCursor(uint8_t Y, uint8_t X)
{
  OLED_WriteCommand(0xB0 | Y);
  OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
  OLED_WriteCommand(0x00 | (X & 0x0F));
}

void OLED_Clear(void)
{
  uint8_t i, j;
  for (j = 0; j < 8; j++)
  {
    OLED_SetCursor(j, 0);
    for (i = 0; i < 128; i++)
    {
      OLED_WriteData(0x00);
    }
  }
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
  uint8_t i;
  OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8); // 设置光标位置在上半部分
  for (i = 0; i < 8; i++)
  {
    OLED_WriteData(OLED_F8x16[Char - ' '][i]); // 显示上半部分内容
  }
  OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8); // 设置光标位置在下半部分
  for (i = 0; i < 8; i++)
  {
    OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]); // 显示下半部分内容
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
  // 使用 HAL 自带延时，替代之前的 for 循环死等
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
  OLED_WriteCommand(0xAF);
  OLED_Clear();
}