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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define APP_ADDRESS 0x08020000
#define FIRMWARE_FILE "firmware.bin"
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUTTON_DOWN_Pin GPIO_PIN_15
#define BUTTON_DOWN_GPIO_Port GPIOA
#define BUTTON_RIGHT_Pin GPIO_PIN_0
#define BUTTON_RIGHT_GPIO_Port GPIOD
#define BUTTON_LEFT_Pin GPIO_PIN_1
#define BUTTON_LEFT_GPIO_Port GPIOD
#define BUTTON_UP_Pin GPIO_PIN_2
#define BUTTON_UP_GPIO_Port GPIOD
#define SD_DTCT_Pin GPIO_PIN_3
#define SD_DTCT_GPIO_Port GPIOD
#define SD_CS_Pin GPIO_PIN_6
#define SD_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
