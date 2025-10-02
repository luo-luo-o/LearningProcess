#include "stm32f10x.h"                  // Device header
#include "Delay.h"

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PB引脚初始化为上拉输入
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~15，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作，当按键按住不放时，函数会卡住，直到按键松手
  */
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0xff;		//定义变量，默认键码值为0xff
	
	for (int num = 0; num < 16; num++)	//循环12次，读取12个按键的状态
	{
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_(num)) == 0)			//读PB1~PB11输入寄存器的状态，如果为0，则代表对应按键按下
		{
			Delay_ms(20);												//延时消抖
			while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_(num)) == 0);	//等待按键松手
			Delay_ms(20);												//延时消抖
			KeyNum = num;											    //置键码为对应按键的编号+1
			return KeyNum;												//跳出循环，避免多个按键同时按下时，键码被后面的按键覆盖
		}
	}

	
	return KeyNum;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0xff
}
