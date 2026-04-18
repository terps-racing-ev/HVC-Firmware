/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lv_can_manager.h
  * @brief          : LV CAN bus manager header
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

#ifndef __LV_CAN_MANAGER_H
#define __LV_CAN_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "can.h"
#include "state.h"
#include "spi_can.h"
#include "debug.h"

typedef bool (*LV_DecodeFunc)(const CAN_Message_t *in, LV_Message_t *out);
typedef bool (*LV_HandleFunc)(const LV_Message_t *msg);

typedef struct {
  LV_DecodeFunc decode;
  LV_HandleFunc handle;
} LV_CanDispatchEntry;

static const LV_CanDispatchEntry LV_DispatchRegister[] = {
  {DecodeResetLV, HandleResetLV}
};
#define LV_DispatchRegisterCount (sizeof(LV_DispatchRegister)/sizeof(LV_CanDispatchEntry))

/**
  * @brief  Initialize LV CAN manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef LV_CAN_Manager_Init(void);

/**
  * @brief  Main LV CAN manager task
  * @param  argument: Not used
  * @retval None
  */
void LV_CAN_ManagerTask(void *argument);

/**
  * @brief  SPI Int pending callback
  * @param  argument: Not used
  * @retval None
  */
void SPI_IntCallbackTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __LV_CAN_MANAGER_H */