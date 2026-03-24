/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void run_every_ms(uint32_t *tick_var, uint32_t interval, void (*func)(void));

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define L298N_IN1_Pin GPIO_PIN_4
#define L298N_IN1_GPIO_Port GPIOA
#define L298N_IN2_Pin GPIO_PIN_5
#define L298N_IN2_GPIO_Port GPIOA
#define MOTOR_PWM_CH1_Pin GPIO_PIN_6
#define MOTOR_PWM_CH1_GPIO_Port GPIOA
#define MOTOR_ENC_A_Pin GPIO_PIN_15
#define MOTOR_ENC_A_GPIO_Port GPIOA
#define MOTOR_ENC_B_Pin GPIO_PIN_3
#define MOTOR_ENC_B_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */


/* -------------- magic defines ---------------- */

/**
 * @brief  Macro to run a code block every specified interval in milliseconds.
 * @param  tick_var: A variable to store the last tick time (must be of type
 * uint32_t).
 * @param  interval: The interval in milliseconds to run the code block.
 * @param  code_block: The code block to execute (should be a lambda or function
 * call).
 */
#define RUN_EVERY_MS(tick_var, interval, code_block)                           \
  do                                                                           \
  {                                                                            \
    if (HAL_GetTick() - (tick_var) >= (interval))                              \
    {                                                                          \
      {code_block}(tick_var) = HAL_GetTick();                                  \
    }                                                                          \
  } while (0)

/* --------------------------------------------- */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
