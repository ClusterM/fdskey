/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SD_MOSI_Pin GPIO_PIN_1
#define SD_MOSI_GPIO_Port GPIOA
#define FDS_READY_Pin GPIO_PIN_4
#define FDS_READY_GPIO_Port GPIOA
#define FDS_MEDIA_SET_Pin GPIO_PIN_5
#define FDS_MEDIA_SET_GPIO_Port GPIOA
#define FDS_WRITABLE_MEDIA_Pin GPIO_PIN_6
#define FDS_WRITABLE_MEDIA_GPIO_Port GPIOA
#define FDS_READ_DATA_Pin GPIO_PIN_7
#define FDS_READ_DATA_GPIO_Port GPIOA
#define FDS_MOTOR_ON_Pin GPIO_PIN_0
#define FDS_MOTOR_ON_GPIO_Port GPIOB
#define FDS_STOP_MOTOR_Pin GPIO_PIN_1
#define FDS_STOP_MOTOR_GPIO_Port GPIOB
#define FDS_WRITE_DATA_Pin GPIO_PIN_2
#define FDS_WRITE_DATA_GPIO_Port GPIOB
#define FDS_WRITE_DATA_EXTI_IRQn EXTI2_IRQn
#define FDS_SCAN_MEDIA_Pin GPIO_PIN_10
#define FDS_SCAN_MEDIA_GPIO_Port GPIOB
#define FDS_SCAN_MEDIA_EXTI_IRQn EXTI15_10_IRQn
#define FDS_WRITE_Pin GPIO_PIN_11
#define FDS_WRITE_GPIO_Port GPIOB
#define FDS_WRITE_EXTI_IRQn EXTI15_10_IRQn
#define SD_CLK_Pin GPIO_PIN_12
#define SD_CLK_GPIO_Port GPIOB
#define SD_MISO_Pin GPIO_PIN_14
#define SD_MISO_GPIO_Port GPIOB
#define SD_DTCT_Pin GPIO_PIN_8
#define SD_DTCT_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
