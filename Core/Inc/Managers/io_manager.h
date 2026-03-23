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
#include "curr_sense.h"
#include "acc.h"
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

/**
  * @brief  Read ADC channel value
  * @param  channel: ADC channel to read
  * @param  out: Pointer to store the result
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef _IO_ReadADCChannel(uint32_t channel, uint16_t *out);

/**
  * @brief  Pack IO summary data for transmission
  * @param  data: Pointer to output buffer
  * @param  length: Pointer to store data length
  * @param  sdc_val: Shutdown circuit value
  * @param  imd_val: Insulation monitoring value
  * @param  therm_val: Thermistor value
  * @param  bms_fault_val: BMS fault value
  * @param  cs_low_val: Current sense low value
  * @param  cs_high_val: Current sense high value
  * @retval None
  */
void _IO_PackIOSummary(
    uint8_t *data,
    uint8_t *length,
    uint8_t sdc_val,
    uint8_t imd_val,
    uint16_t therm_val,
    uint8_t bms_fault_val,
    uint16_t cs_low_val,
    uint16_t cs_high_val
);
