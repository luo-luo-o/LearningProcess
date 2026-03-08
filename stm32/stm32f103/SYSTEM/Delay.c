/**
 * @file Delay.c
 * @brief STM32阻塞式和非阻塞式延时库的实现文件
 * @details 基于SysTick定时器实现，包含us/ms/s三级单位的阻塞式和非阻塞延时函数，
 *          非阻塞模式通过64位时间戳实现超长计时，无溢出风险。
 * @author luo-luo-o
 */
#include "Delay.h"

/**
 * @brief 全局us级计数器
 * @details 由SysTick中断每1us更新一次，用于非阻塞延时的时间戳基准，
 *          64位无符号整数确保超长计时（约58000年不溢出）
 */
volatile uint64_t g_total_us = 0;

/**
 * @def SYSTICK_RELOAD_US
 * @brief SysTick定时器1us的重装载值
 * @details 基于72MHz系统时钟计算：1us = 72个时钟周期，计数器范围0~71，故重装载值为71
 */
#define SYSTICK_RELOAD_US 71

/**
 * @def DELAY_BLOCKING_BASE_COUNT
 * @brief 阻塞延时的基本循环计数值
 * @details 1us延时的循环计数值。可根据实际延迟表现
 */
#define DELAY_BLOCKING_BASE_COUNT 72 // 基于72MHz时钟，约1us延时

/**
 * @brief 内部函数：获取当前系统总us数
 * @details 读取全局计数器g_total_us，读取时关闭中断确保数据完整性
 * @param 无
 * @retval uint64_t 当前总us数
 */
static uint64_t _Delay_GetCurrentUs(void)
{
    uint64_t current_us;
    __disable_irq(); // 关闭中断，防止读取时被SysTick中断修改
    current_us = g_total_us;
    __enable_irq(); // 恢复中断
    return current_us;
}

/**
 * @copydoc delay.h::Delay_Init
 */
void Delay_Init(void)
{
    // 配置SysTick时钟源为HCLK（72MHz），使能中断
    SysTick->LOAD = SYSTICK_RELOAD_US;                                     // 设置1us对应的重装载值
    SysTick->VAL = 0x00;                                                   // 清空当前计数值
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk; // 时钟源+中断使能
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;                              // 启动SysTick定时器

    // 确保全局变量初始化为0
    g_total_us = 0;
}

/**
 * @copydoc delay.h::Delay_us
 */
void Delay_us(Delay_Handle *delay_handle, uint32_t target_us)
{
    if (delay_handle == NULL)
    {
        return; // 空指针保护
    }

    // 限制us级最大时长（24位SysTick计数器上限：2^24/72 ≈ 233015us）
    if (target_us > 233015)
    {
        target_us = 233015;
    }

    // 记录起始时间戳和目标时长，启动延时
    delay_handle->start_tick = _Delay_GetCurrentUs();
    delay_handle->target_tick = target_us;
    delay_handle->state = DELAY_NON_BLOCK_RUNNING;
}

/**
 * @copydoc delay.h::Delay_ms
 */
void Delay_ms(Delay_Handle *delay_handle, uint32_t target_ms)
{
    if (delay_handle == NULL)
    {
        return; // 空指针保护
    }

    // 转换ms到us（1ms = 1000us）
    delay_handle->start_tick = _Delay_GetCurrentUs();
    delay_handle->target_tick = (uint64_t)target_ms * 1000;
    delay_handle->state = DELAY_NON_BLOCK_RUNNING;
}

/**
 * @copydoc delay.h::Delay_s
 */
void Delay_s(Delay_Handle *delay_handle, uint32_t target_s)
{
    if (delay_handle == NULL)
    {
        return; // 空指针保护
    }

    // 转换s到us（1s = 1000000us）
    delay_handle->start_tick = _Delay_GetCurrentUs();
    delay_handle->target_tick = (uint64_t)target_s * 1000000;
    delay_handle->state = DELAY_NON_BLOCK_RUNNING;
}

/**
 * @copydoc delay.h::Delay_Check
 */
Delay_StateType Delay_Check(Delay_Handle *delay_handle)
{
    if (delay_handle == NULL)
    {
        return DELAY_NON_BLOCK_IDLE; // 空指针返回未启动状态
    }

    // 仅处理进行中的延时任务
    if (delay_handle->state != DELAY_NON_BLOCK_RUNNING)
    {
        return delay_handle->state;
    }

    // 计算已流逝的时间（64位差值，无溢出风险）
    uint64_t elapsed_us = _Delay_GetCurrentUs() - delay_handle->start_tick;

    // 若达到目标时长，标记为已结束
    if (elapsed_us >= delay_handle->target_tick)
    {
        delay_handle->state = DELAY_NON_BLOCK_FINISHED;
    }

    return delay_handle->state;
}


/**
 * @copydoc delay.h::Delay_us_Blocking
 */
void Delay_us_Blocking(uint32_t us)
{
    // 简易阻塞延时实现，基于无意义循环
    // 注意：此方法受编译器优化影响，精度较低，仅适用于粗略延时
    volatile uint32_t count;
    while (us--)
    {
        count = DELAY_BLOCKING_BASE_COUNT; // 约1us延时（72MHz下）
        while (count--)
            ;
    }
}

/**
 * @copydoc delay.h::Delay_ms_Blocking
 */
void Delay_ms_Blocking(uint32_t ms)
{
    // 简易阻塞延时实现，基于无意义循环
    // 注意：此方法受编译器优化影响，精度较低，仅适用于粗略延时
    volatile uint32_t count;
    while (ms--)
    {
        count = DELAY_BLOCKING_BASE_COUNT * 100; // 约1ms延时（72MHz下）
        while (count--)
            ;
    }
}

/**
 * @copydoc delay.h::Delay_s_Blocking
 */
void Delay_s_Blocking(uint32_t s)
{
    // 简易阻塞延时实现，基于无意义循环
    // 注意：此方法受编译器优化影响，精度较低，仅适用于粗略延时
    volatile uint32_t count;
    while (s--)
    {
        count = DELAY_BLOCKING_BASE_COUNT * 10000; // 约1s延时（72MHz下）
        while (count--)
            ;
    }
}
