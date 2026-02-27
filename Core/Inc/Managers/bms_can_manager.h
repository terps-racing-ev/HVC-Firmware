/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bms_can_manager.h
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
#include "can.h"
#include "state.h"
#include "bmb.h"

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

typedef bool (*DecodeFunc)(const CAN_Message_t *in, BMS_Message *out);
typedef bool (*HandleFunc)(const BMS_Message *msg);

typedef struct {
    uint32_t can_id;
    DecodeFunc decode;
    HandleFunc handle;
} CanDispatchEntry;

static const CanDispatchEntry DispatchRegister[] = {
    {BMB_CAN_TEMP_SUMMARY, DecodeCellTempSummary, HandleCellTempSummary}
    
};
extern const uint8_t DispatchRegisterCount; // Computed at runtime in bms can manager

#ifdef __cplusplus
}
#endif

#endif /* __CAN_MANAGER_H */