#include <luoluo_LIB_stm32f10x.h>

/**
 * @brief 自动重置的延时处理函数
 * @param process 业务处理函数
 * @param delay_handle 延时句柄指针
 * @param delay_time 延时时长(ms)
 * @return uint8_t 1-本次执行了处理函数，0-未执行
 */
uint8_t do_Process_with_delay_ms(Process_ptr process, Delay_Handle *delay_handle, uint32_t delay_time)
{
    if (process == NULL || delay_handle == NULL)
    {
        return 0;
    }

    // 检查延时状态
    Delay_StateType state = Delay_Check(delay_handle);

    if (state == DELAY_NON_BLOCK_FINISHED)
    {
        // 执行业务函数
        process();
        // 自动重启延时
        Delay_ms(delay_handle, delay_time);
        return 1;
    }
    else if (state == DELAY_NON_BLOCK_IDLE)
    {
        // 首次启动延时
        Delay_ms(delay_handle, delay_time);
    }

    return 0;
}


