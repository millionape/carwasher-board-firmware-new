/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_SEL_Pin GPIO_PIN_1
#define USER_SEL_GPIO_Port GPIOA
#define MODE_SW_Pin GPIO_PIN_2
#define MODE_SW_GPIO_Port GPIOA
#define FRONT_SW_1_Pin GPIO_PIN_3
#define FRONT_SW_1_GPIO_Port GPIOA
#define FRONT_SW_2_Pin GPIO_PIN_4
#define FRONT_SW_2_GPIO_Port GPIOA
#define FRONT_SW_3_Pin GPIO_PIN_5
#define FRONT_SW_3_GPIO_Port GPIOA
#define FRONT_SW_4_Pin GPIO_PIN_6
#define FRONT_SW_4_GPIO_Port GPIOA
#define FRONT_SW_5_Pin GPIO_PIN_7
#define FRONT_SW_5_GPIO_Port GPIOA
#define OUT_1_Pin GPIO_PIN_0
#define OUT_1_GPIO_Port GPIOB
#define OUT_2_Pin GPIO_PIN_1
#define OUT_2_GPIO_Port GPIOB
#define OUT_6_Pin GPIO_PIN_12
#define OUT_6_GPIO_Port GPIOB
#define MAX7219_CS_Pin GPIO_PIN_14
#define MAX7219_CS_GPIO_Port GPIOB
#define OUT_3_Pin GPIO_PIN_8
#define OUT_3_GPIO_Port GPIOA
#define ALIVE_LED_Pin GPIO_PIN_11
#define ALIVE_LED_GPIO_Port GPIOA
#define CREDIT_RESET_Pin GPIO_PIN_15
#define CREDIT_RESET_GPIO_Port GPIOA
#define COIN_IT_Pin GPIO_PIN_4
#define COIN_IT_GPIO_Port GPIOB
#define COIN_IT_EXTI_IRQn EXTI4_IRQn
#define BANK_IT_Pin GPIO_PIN_5
#define BANK_IT_GPIO_Port GPIOB
#define BANK_IT_EXTI_IRQn EXTI9_5_IRQn
#define OUT_4_Pin GPIO_PIN_8
#define OUT_4_GPIO_Port GPIOB
#define OUT_5_Pin GPIO_PIN_9
#define OUT_5_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
