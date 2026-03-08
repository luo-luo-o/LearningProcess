/**
 * @file Delay.h
 * @brief STM32阻塞式和非阻塞式延时库（支持us/ms/s三级单位）
 * @details 实现了基于SysTick定时器的高精度延时功能，包含非阻塞式延时API，支持多任务并行。
 *          非阻塞模式通过时间戳对比实现，不占用CPU资源，适合多任务场景。
 * @author luo-luo-o
 */
#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"
#include <stddef.h>

/**
 * @defgroup DELAY_GROUP 延时功能模块
 * @brief 包含阻塞式和非阻塞式延时的完整实现
 * @{
 */

/**
 * @defgroup NonBlockingDelay 非阻塞延时
 * @brief 非阻塞式延时
 * @{
 *
 */

/**
 * @enum Delay_StateType
 * @brief 非阻塞延时的状态枚举
 * @details 用于标记非阻塞延时的生命周期，通过状态判断延时是否完成
 */
typedef enum
{
    DELAY_NON_BLOCK_IDLE = 0,    /*!< 延时未启动（初始状态） */
    DELAY_NON_BLOCK_RUNNING = 1, /*!< 延时正在进行中 */
    DELAY_NON_BLOCK_FINISHED = 2 /*!< 延时已结束 */
} Delay_StateType;

/**
 * @struct Delay_Handle
 * @brief 非阻塞延时句柄结构体
 * @details 每个非阻塞延时任务需要独立的句柄，用于存储延时参数和状态，支持多任务并行
 */
typedef struct
{
    volatile uint64_t start_tick;   /*!< 延时起始时间戳（单位：us） */
    volatile uint64_t target_tick;  /*!< 目标延时总时长（单位：us，内部统一转换） */
    volatile Delay_StateType state; /*!< 当前延时状态（见Delay_StateType） */
} Delay_Handle;

/**
 * @brief 初始化非阻塞延时的时间基准（必须先调用）
 * @details 配置SysTick定时器为1us触发一次中断，作为全局时间戳的计数基准，
 *          所有非阻塞延时函数必须在此函数调用后才能正常工作。
 * @note 基于72MHz系统时钟设计，若时钟频率不同，需修改SYSTICK_RELOAD_US宏定义
 * @param 无
 * @retval 无
 */
void Delay_Init(void);

/**
 * @brief 启动us级非阻塞延时
 * @details 初始化非阻塞延时句柄，记录起始时间并设置目标延时时长（us级），
 *          需配合Delay_Check()函数查询延时状态。
 * @warning us级延时受SysTick 24位计数器限制，最大支持233015us（约233ms）
 * @param[in,out] delay_handle 非阻塞延时句柄指针（需提前定义）
 * @param[in] target_us 目标延时时长（单位：us，范围：0~233015）
 * @retval 无
 */
void Delay_us(Delay_Handle *delay_handle, uint32_t target_us);

/**
 * @brief 启动ms级非阻塞延时
 * @details 初始化非阻塞延时句柄，记录起始时间并设置目标延时时长（ms级，内部转为us处理），
 *          需配合Delay_Check()函数查询延时状态。
 * @param[in,out] delay_handle 非阻塞延时句柄指针（需提前定义）
 * @param[in] target_ms 目标延时时长（单位：ms，范围：0~4294967295）
 * @retval 无
 */
void Delay_ms(Delay_Handle *delay_handle, uint32_t target_ms);

/**
 * @brief 启动s级非阻塞延时
 * @details 初始化非阻塞延时句柄，记录起始时间并设置目标延时时长（s级，内部转为us处理），
 *          需配合Delay_Check()函数查询延时状态。
 * @param[in,out] delay_handle 非阻塞延时句柄指针（需提前定义）
 * @param[in] target_s 目标延时时长（单位：s，范围：0~4294967295）
 * @retval 无
 */
void Delay_s(Delay_Handle *delay_handle, uint32_t target_s);

/**
 * @brief 查询非阻塞延时的当前状态
 * @details 通过对比当前时间戳与起始时间戳，判断延时是否达到目标时长，
 *          是实现非阻塞延时的核心函数。
 * @param[in] delay_handle 非阻塞延时句柄指针
 * @retval Delay_StateType 延时状态：
 *         - DELAY_NON_BLOCK_IDLE：延时未启动
 *         - DELAY_NON_BLOCK_RUNNING：延时进行中
 *         - DELAY_NON_BLOCK_FINISHED：延时已结束
 */
Delay_StateType Delay_Check(Delay_Handle *delay_handle);

// 结束 NonBlockingDelay 组
/**
 * @}
 */

/**
 * @defgroup BlockingDelay 阻塞延时
 * @brief 阻塞式延时
 * @{
 */

/**
 * @brief 阻塞式延迟函数（使用无意义循环消耗时间）
 * @param us 延时计数值，值越大延时越长
 * @return 无
 */
void Delay_us_Blocking(uint32_t us);

/**
 * @brief 阻塞式延迟函数（使用无意义循环消耗时间）
 * @param ms 延时计数值，值越大延时越长
 * @return 无
 */
void Delay_ms_Blocking(uint32_t ms);

/**
 * @brief 阻塞式延迟函数（使用无意义循环消耗时间）
 * @param s 延时计数值，值越大延时越长
 * @return 无
 */
void Delay_s_Blocking(uint32_t s);

/**
 * @}
 */


// 结束DELAY_GROUP
/**
 * @}
 */

#endif /* __DELAY_H */
