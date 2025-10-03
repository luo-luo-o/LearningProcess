/*  Include Modules  */
#include "stm32f10x.h"

#include "luoluo_LIB_stm32f10x.h"

/*  End of Incldue  */

/*  Defines  */

/*  End of Defines  */

/*  Global Variable  */

/*  End of Global Variable  */

/*  Inits  */

void GPIO_Config()
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE); // 开启GPIOA的时钟

    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure; // 定义结构体变量

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // GPIO模式，赋值为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = LED_PINs;           // GPIO引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速度，赋值为50MHz
    GPIO_Init(LED_PORT, &GPIO_InitStructure);         // 实现LED_PORT的初始化

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // GPIO模式，赋值为上拉输入模式
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
    GPIO_Init(LED_PORT, &GPIO_InitStructure); // 实现LED_PORT的初始化

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8); // 选择GPIOA的第8脚作为外部中断源

    EXTI_InitTypeDef EXTI_InitStructure;                           // 定义结构体变量
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;                     // 选择外部中断线
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;            // 设置为中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;                      // 使能外部中断线
    EXTI_Init(&EXTI_InitStructure);                                // 实现外部中断线的初始化

    /*中断优先级配置*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);           // 设置优先级分组2
    NVIC_InitTypeDef NVIC_InitStructure;                      // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;        // 选择中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 使能中断通道
    NVIC_Init(&NVIC_InitStructure);                           // 实现中断通道的初始化
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
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
            ;

        /* 选择PLL作为系统时钟源 */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* 等待PLL被选为系统时钟源 */
        while (RCC_GetSYSCLKSource() != 0x08)
            ;
    }

    /* 设置Flash延迟：2个等待状态（72MHz需要） */
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
}

/*  End of Init  */

/*  Methods  */

/*  End of Methods  */

/*  Process  */

/*  End of Process  */

/*  Main  */
int main(void)
{
    SystemClock_Config();
    Delay_Init(); // 初始化非阻塞延时模块
    GPIO_Config();

    while (1)
    {
        // Process Lists
    }
}
/*  End of Main  */
