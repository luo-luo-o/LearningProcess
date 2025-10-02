/*  Include Modules  */
#include <stm32f10x.h>
#include <Delay.h>
/*  End of Incldue  */

/*  Defines  */
#define GPIO_Pin_(num) (1 << num)
/*  End of Defines  */

/*  Inits  */
void GPIO_Config()
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 开启GPIOA的时钟

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure; // 定义结构体变量

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // GPIO模式，赋值为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = 0x00ff;			  // GPIO引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速度，赋值为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // 实现GPIOA的初始化

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = 0xff00;
	GPIO_Init(GPIOA, &GPIO_InitStructure); // 实现GPIOA的初始化
}
/*  End of Init  */

/*  Funtions  */
/**
 * @brief   实时检测按键状态
 * @param   KEY_PIN: 按键引脚编号
 * @return  1-按键按下，0-按键松开
 */
uint8_t Key_GetState(uint8_t KEY_PIN)
{
	// 读取引脚状态，消抖处理
	uint8_t state1 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_(KEY_PIN));
	Delay_ms(10); // 简易消抖
	uint8_t state2 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_(KEY_PIN));

	// 两次读取都为低电平才认为按键按下
	return (state1 == 0 && state2 == 0) ? 1 : 0;
}

/*  End of Funtions  */

int main(void)
{

	GPIO_Config();

	uint16_t reverse = 0;
	uint16_t i = 0;
	if (Key_GetState(8)) // 如果有按键按下
	{
		i = 7;
	}

	while (1)
	{

		// 点亮当前LED
		GPIO_WriteBit(GPIOA, GPIO_Pin_(i), Bit_SET);
		Delay_ms(100);
		// 熄灭当前LED
		GPIO_WriteBit(GPIOA, GPIO_Pin_(i), Bit_RESET);

		// 检测按键（非阻塞）
		if (Key_GetState(8))
		{
			reverse = 1;
		}
		else
		{
			reverse = 0;
		}

		// 根据方向更新LED位置
		if (reverse == 0)
		{
			if (i == 7)
				i = 0; // 正向循环
			else
				i++;
		}
		else if (reverse == 1)
		{
			if (i == 0)
				i = 7; // 处理反向边界
			else
				i--; // 反向循环
		}
	}
}
