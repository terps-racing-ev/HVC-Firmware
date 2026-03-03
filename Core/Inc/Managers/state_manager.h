/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : state_manager.h
  * @brief          : State manager header
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
#include "state.h"
#include "main.h"
#include "io.h"
#include "can.h"
#include "can_id.h"
#include "acc.h"

#define STATE_REFRESH_FREQ_MS 10

#define MODULE_TIMEOUT_CUTOFF_TICKS 100

#define CHECK_MODULE_TIMEOUT 1
#define CHECK_REF_OVERTEMP 1

/**
  * @brief  Initialize State manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef State_Manager_Init(void);

/**
  * @brief  Main State manager task
  * @param  argument: Not used
  * @retval None
  */
void State_ManagerTask(void *argument);