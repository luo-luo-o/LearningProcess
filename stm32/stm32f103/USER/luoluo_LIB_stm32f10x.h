/**
 ******************************************************************************
 * @file   luoluoLIB_stm32f103.h
 * @author luo_luo
 * @brief  stm32f103二次封装
 ******************************************************************************
 *
 */
#ifndef LUOLUOLIB_STM32F103_H
#define LUOLUOLIB_STM32F103_H

/*  Include necessary headers  */
#include <stm32f10x.h>
#include <Delay.h>
/*  End Include  */


/*  Typedef  */
typedef void (*Process_ptr)(void); // 业务处理函数指针类型
/*  End Typedef  */


/*  GPIO   */
#define GPIO_PIN_(num) (1 << num)
/*  End GPIO  */

/*  Delay  */

/**
 * @brief 自动重置的延时处理函数
 * @param process 业务处理函数
 * @param delay_handle 延时句柄指针
 * @param delay_time 延时时长(ms)
 * @return uint8_t 1-本次执行了处理函数，0-未执行
 */
uint8_t do_Process_with_delay_ms(Process_ptr process, Delay_Handle *delay_handle, uint32_t delay_time);

/*  End of Delay  */

#endif // LUOLUOLIB_H

// End of file
