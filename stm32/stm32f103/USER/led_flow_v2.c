/*  Include Modules  */
#include "stm32f10x.h"

#include "luoluo_LIB_stm32f10x.h"

/*  End of Incldue  */

/*  Defines  */

#define GPIO_Pin_(num) (1 << num)
#define LED_PORT GPIOA
#define LED_PINs GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7
#define KEY_PORT GPIOA
#define KEY_PIN GPIO_Pin_8

/*  End of Defines  */

/*  Global Variable  */

// 用volatile修饰中断共享变量，防止编译器优化
volatile uint16_t reverse = 0;

/*  End of Global Variable  */

/*  Inits  */

void GPIO_Config()
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE); // 开启GPIOA的时钟

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure; // 定义结构体变量

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // GPIO模式，赋值为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = LED_PINs;			  // GPIO引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速度，赋值为50MHz
	GPIO_Init(LED_PORT, &GPIO_InitStructure);			  // 实现LED_PORT的初始化

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // GPIO模式，赋值为上拉输入模式
	GPIO_InitStructure.GPIO_Pin = KEY_PIN;
	GPIO_Init(LED_PORT, &GPIO_InitStructure); // 实现LED_PORT的初始化

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8); // 选择GPIOA的第8脚作为外部中断源

	EXTI_InitTypeDef EXTI_InitStructure;					// 定义结构体变量
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;				// 选择外部中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// 设置为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				// 使能外部中断线
	EXTI_Init(&EXTI_InitStructure);							// 实现外部中断线的初始化

	/*中断优先级配置*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			  // 设置优先级分组2
	NVIC_InitTypeDef NVIC_InitStructure;					  // 定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;		  // 选择中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		  // 子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // 使能中断通道
	NVIC_Init(&NVIC_InitStructure);							  // 实现中断通道的初始化
}

void SystemClock_Config(void)
{
    /* 复位RCC寄存器到默认状态 */
    RCC_DeInit();
    
    /* 使能外部高速晶振HSE */
    RCC_HSEConfig(RCC_HSE_ON);
    
    /* 等待HSE准备就绪 */
    ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        /* 设置HCLK（AHB时钟）= SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        
        /* 设置PCLK2（APB2时钟）= HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1);
        
        /* 设置PCLK1（APB1时钟）= HCLK/2 (最大36MHz) */
        RCC_PCLK1Config(RCC_HCLK_Div2);
        
        /* 配置PLL：HSE * 9 = 8MHz * 9 = 72MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        
        /* 使能PLL */
        RCC_PLLCmd(ENABLE);
        
        /* 等待PLL准备就绪 */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        
        /* 选择PLL作为系统时钟源 */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        
        /* 等待PLL被选为系统时钟源 */
        while (RCC_GetSYSCLKSource() != 0x08);
    }
    
    /* 设置Flash延迟：2个等待状态（72MHz需要） */
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
}

/*  End of Init  */

/*  Methods  */

/**
 * @brief   实时检测按键状态
 * @param   KEY: 按键引脚编号
 * @return  1-按键按下，0-按键松开
 */
uint8_t Key_GetState(uint8_t KEY)
{
	// 读取引脚状态，消抖处理
	uint8_t state1 = GPIO_ReadInputDataBit(KEY_PORT, GPIO_Pin_(KEY));
	Delay_ms_Blocking(10); // 简易消抖
	uint8_t state2 = GPIO_ReadInputDataBit(KEY_PORT, GPIO_Pin_(KEY));

	// 两次读取都为低电平才认为按键按下
	return (state1 == 0 && state2 == 0) ? 1 : 0;
}

/*  End of Methods  */

/*  Process  */


// 新增：LED状态机枚举（标记当前应执行“亮”还是“灭”）
typedef enum {
    LED_STATE_WAIT_ON,  // 等待执行LED_ON（延时结束后亮）
    LED_STATE_WAIT_OFF  // 等待执行LED_OFF（亮500ms后灭）
} LED_WorkState;
LED_WorkState led_work_state = LED_STATE_WAIT_ON; // 初始状态：等待亮
uint16_t LED_PIN;

void LED_ON(void)
{
	GPIO_WriteBit(LED_PORT, GPIO_Pin_(LED_PIN), Bit_SET);
}
void LED_OFF(void)
{
	GPIO_WriteBit(LED_PORT, GPIO_Pin_(LED_PIN), Bit_RESET);
}
void LED_Process(void)
{
	if (led_work_state == LED_STATE_WAIT_ON)
	{
		LED_ON();
		led_work_state = LED_STATE_WAIT_OFF; // 切换状态，等待灭
	}
	else if (led_work_state == LED_STATE_WAIT_OFF)
	{
		LED_OFF();
		// 切换到下一个LED引脚
		if (!reverse)
		{
			LED_PIN++;
			if (LED_PIN > 7)
				LED_PIN = 0;
		}
		else
		{
			if (LED_PIN == 0)
				LED_PIN = 7;
			else
				LED_PIN--;
		}
		led_work_state = LED_STATE_WAIT_ON; // 切换状态，等待亮
	}
}

/*  End of Process  */

/*  Main  */

int main(void)
{
	SystemClock_Config();
	Delay_Init(); // 初始化非阻塞延时模块
	GPIO_Config();

	if (!reverse)
		LED_PIN = 0; // 当前点亮的LED引脚编号
	else
		LED_PIN = 7;

	Delay_Handle LED_delay_handle;					  // 定义非阻塞延时句柄
	LED_delay_handle.state = DELAY_NON_BLOCK_IDLE;    // 初始化状态

	while (1)
	{
		// LED_Process();
		do_Process_with_delay_ms(LED_Process, &LED_delay_handle, 50);

	
	}
}

/*  End of Main  */
