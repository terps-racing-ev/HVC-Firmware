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
#include "soc.h"

#define ACC_UPDATE_FREQ_MS 10

#define SOC_CS_QUEUE_SIZE 20

// Queue of curr sense readings for soc
// Instead of acc_manager task reading curr sense
// value whenever it updates, it just reads what the 
// IO priority manager puts on the queue. This means 
// the acc manager can run on a slower update freq 
// since it can just batch process all things on queue
osMessageQueueId_t soc_curr_sense_queue;

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
