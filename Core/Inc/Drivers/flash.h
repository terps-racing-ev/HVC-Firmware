/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash.h
  * @brief          : Header for reading/writing to flash
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
#include "cmsis_os2.h"

#define FLASH_MAGIC 0xBEEF0000  // Magic value that shows flash is initialized

/* Addresses ----------------------------------------------------------------*/
#define FLASH_START_ADDRESS               0x0803C000  // First word of storage region

typedef enum {
  FLASH_SOC_TOTAL_CAPACITY_ADDRESS = (FLASH_START_ADDRESS + 4), // Total acc capacity (calculated during charging)
  FLASH_SOC_LAST_LOAD_TIME_ADDRESS = (FLASH_START_ADDRESS + 8) // Time of last acc load
} Flash_Address_t;

typedef struct {
  uint32_t soc_total_capacity;        // Total capacity in A*ms
  uint32_t soc_last_load_time;        // RTC time of last load
  uint32_t soc_last_calculated;       // Last calculated SOC
  uint32_t soc_last_calculated_time;  // Time SOC was calculated last
} Flash_SOC_Data_t;

/**
 * @brief Inits flash and reads data into RAM. If flash hasn't been written to yet, inits
 */
HAL_StatusTypeDef Flash_Init(void);

/**
 * @brief (Blocking) Read SOC data.
 * @param data Data output
 */
HAL_StatusTypeDef Flash_ReadSOCData(Flash_SOC_Data_t *data);

/**
 * @brief (Blocking) Write SOC data to flash.
 * @param data Data to write
 */
HAL_StatusTypeDef Flash_WriteSOCData(Flash_SOC_Data_t *data);