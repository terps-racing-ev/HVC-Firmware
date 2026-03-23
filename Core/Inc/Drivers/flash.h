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
#ifndef FLASH_H
#define FLASH_H

#include "stm32l4xx_hal.h"
#include "cmsis_os2.h"

#define FLASH_MAGIC 0xBEEF0000u         // Generic flash marker for this driver
#define FLASH_SOC_RECORD_MAGIC 0x534F4331u  // "SOC1"
#define FLASH_SOC_COMMIT_MAGIC 0x434F4D4Du  // "COMM"
/* Storage layout ------------------------------------------------------------*/
#define FLASH_START_ADDRESS 0x0803C000u  // First word of storage region
#define FLASH_SOC_PAGE_2_ADDRESS (FLASH_START_ADDRESS + FLASH_PAGE_SIZE)
#define FLASH_SOC_PAGE_3_ADDRESS (FLASH_START_ADDRESS + 2 * FLASH_PAGE_SIZE)
typedef enum {
  FLASH_SOC_PAGE_2 = 0,
  FLASH_SOC_PAGE_3 = 1
} Flash_SOC_Page_t;

typedef struct {
  uint32_t soc_total_capacity;         // Total capacity in A*ms
  uint32_t soc_last_load_time;         // RTC time of last load
  uint32_t soc_last_calculated;        // Last calculated SOC
  uint32_t soc_last_calculated_time;   // Time SOC was calculated last
} Flash_SOC_Data_t;

typedef struct {
  uint32_t sequence;
  Flash_SOC_Data_t payload;
} Flash_SOC_Record_t;

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

#endif  // FLASH_H