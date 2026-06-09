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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define SDI1_Pin GPIO_PIN_5
#define SDI1_GPIO_Port GPIOA
#define SCLK1_Pin GPIO_PIN_6
#define SCLK1_GPIO_Port GPIOA
#define LOAD1_Pin GPIO_PIN_7
#define LOAD1_GPIO_Port GPIOA
#define Btn3_Pin GPIO_PIN_0
#define Btn3_GPIO_Port GPIOB
#define Btn4_Pin GPIO_PIN_1
#define Btn4_GPIO_Port GPIOB
#define LED_Blue_Pin GPIO_PIN_13
#define LED_Blue_GPIO_Port GPIOB
#define LED_Red_Pin GPIO_PIN_14
#define LED_Red_GPIO_Port GPIOB
#define LED_Yellow_Pin GPIO_PIN_15
#define LED_Yellow_GPIO_Port GPIOB
#define Manual_Btn_Pin GPIO_PIN_12
#define Manual_Btn_GPIO_Port GPIOA
#define Reset_Btn_Pin GPIO_PIN_15
#define Reset_Btn_GPIO_Port GPIOA
#define Btn1_Pin GPIO_PIN_3
#define Btn1_GPIO_Port GPIOB
#define Btn2_Pin GPIO_PIN_4
#define Btn2_GPIO_Port GPIOB
#define Relay_220_Pin GPIO_PIN_5
#define Relay_220_GPIO_Port GPIOB
#define Relay_24_Pin GPIO_PIN_6
#define Relay_24_GPIO_Port GPIOB
#define SDI2_Pin GPIO_PIN_7
#define SDI2_GPIO_Port GPIOB
#define SCLK2_Pin GPIO_PIN_8
#define SCLK2_GPIO_Port GPIOB
#define LOAD2_Pin GPIO_PIN_9
#define LOAD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
