#ifndef __OLED_H
#define __OLED_H

/*
    4口OLED显示驱动，使用I2C通信，适用于常见的0.96寸12864点阵OLED屏幕
    屏幕可容纳 4 行 16 列 字符，每个字符占 8x16 像素
*/

#define ROW 4
#define COL 16

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Update(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#endif
