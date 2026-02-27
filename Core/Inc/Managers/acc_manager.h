/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : acc_manager.h
  * @brief          : Acc manager header
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

#include "stm32l4xx_hal.h"
#include "acc.h"

/**
  * @brief  Initialize acc manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef Acc_Manager_Init(void);

/**
  * @brief  Main acc manager task
  * @param  argument: Not used
  * @retval None
  */
void Acc_ManagerTask(void *argument);
