/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : can_manager.h
  * @brief          : CAN bus manager header
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

#ifndef __CAN_MANAGER_H
#define __CAN_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdbool.h>
#include "can.h"

/* External Variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan1;

/* Function Prototypes -------------------------------------------------------*/

/**
  * @brief  Initialize CAN manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BMS_CAN_Manager_Init(void);

/**
  * @brief  Main CAN manager task
  * @param  argument: Not used
  * @retval None
  */
void BMS_CAN_ManagerTask(void *argument);

/**
  * @brief  Send CAN message (non-blocking, queues message)
  * @param  id: CAN message ID (29-bit extended, max 0x1FFFFFFF)
  * @param  data: Pointer to data buffer (up to 8 bytes)
  * @param  length: Data length (0-8 bytes)
  * @param  priority: Message priority (0 = highest, 3 = lowest)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BMS_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_MANAGER_H */