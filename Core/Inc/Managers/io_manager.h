/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : io_manager.h
  * @brief          : IO manager header
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

#include "main.h"
#include "io.h"
#include "can.h"
#include "state.h"
#include "can_id.h"
#include "therm.h"
#include "stm32l4xx_hal.h"

#define IO_UPDATE_FREQ_MS 10
#define IO_PRIORITY_UPDATE_FREQ_MS 1

/**
  * @brief  Initialize IO manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef IO_Manager_Init(void);

/**
  * @brief  Main IO manager task
  * @param  argument: Not used
  * @retval None
  */
void IO_ManagerTask(void *argument);

/**
  * @brief  Main IO manager task (for priority io values). Runs at faster rate.
  * @param  argument: Not used
  * @retval None
  */
void IO_PriorityManagerTask(void *argument);