/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
ADC_HandleTypeDef hadc1;
COMP_HandleTypeDef hcomp2;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_HC_Pin GPIO_PIN_0
#define CS_HC_GPIO_Port GPIOA
#define CS_LC_Pin GPIO_PIN_1
#define CS_LC_GPIO_Port GPIOA
#define Therm_Pin GPIO_PIN_3
#define Therm_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define EMeter_Sig_Pin GPIO_PIN_0
#define EMeter_Sig_GPIO_Port GPIOB
#define LV_CAN_INT_Pin GPIO_PIN_1
#define LV_CAN_INT_GPIO_Port GPIOB
#define LV_CAN_INT_EXTI_IRQn EXTI1_IRQn
#define IMD_Pin GPIO_PIN_9
#define IMD_GPIO_Port GPIOA
#define SDC_Pin GPIO_PIN_10
#define SDC_GPIO_Port GPIOA
#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOB
#define Batt_Pin GPIO_PIN_4
#define Batt_GPIO_Port GPIOB
#define PL_Signal_Pin GPIO_PIN_5
#define PL_Signal_GPIO_Port GPIOB
#define BMS_Fault_Pin GPIO_PIN_6
#define BMS_Fault_GPIO_Port GPIOB
#define Inv_Pin GPIO_PIN_7
#define Inv_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
