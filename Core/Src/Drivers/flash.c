/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash.c
  * @brief          : Implementation for flash driver
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

#include "flash.h"
#include "stdbool.h"

/**
 * @brief Initialize the magic value in flash if not already. Use only in init or after acquiring flash_mutex
 */
HAL_StatusTypeDef _Flash_InitMagic(void);
/**
 * @brief Read one uint32 from flash at given address
 * @param addr: Address in flash to read
 * @param data: Data output
 * @return Status of read
 */
HAL_StatusTypeDef _Flash_ReadFromFlash(uint32_t addr, uint32_t *data);
/**
 * @brief (Non-blocking) write data in array to flash starting from given address. Use only in init or after acquiring flash_mutex
 * @param start_addr: Start address in flash of data to write
 * @param data_arr: Array of data to write, in 64 bit doublewords
 * @param data_length: Number of elements in array
 */
HAL_StatusTypeDef _Flash_WriteToFlash(Flash_Address_t start_addr, uint64_t *data_arr, uint16_t data_length);

// Store flash data in RAM after first read
Flash_SOC_Data_t flash_data;
bool flash_read = false;
osMutexId_t flash_mutex;

HAL_StatusTypeDef Flash_Init(void) {
    // Init mutex
    const osMutexAttr_t flash_mutex_attr = {
        .name = "Flash_Mutex"
    };
    flash_mutex = osMutexNew(&flash_mutex_attr);
    if (flash_mutex == NULL) return HAL_ERROR;

    // Initialize flash magic value
    if (_Flash_InitMagic() != HAL_OK) {
        return HAL_ERROR;
    }

    // Read in data from flash
    

    return HAL_OK;
}

/**
 * @brief Initialize the magic value in flash if not already. Use only in init or after acquiring flash_mutex
 */
HAL_StatusTypeDef _Flash_InitMagic(void) {
    // If already initted don't do anything
    uint32_t flash_magic = *(__IO uint32_t*)FLASH_START_ADDRESS;
    if (flash_magic == FLASH_MAGIC) {
        return HAL_OK;
    }

    uint64_t data = ((uint64_t)FLASH_MAGIC << 32) | ((uint64_t)FLASH_MAGIC);
    return _Flash_WriteToFlash(FLASH_START_ADDRESS, &data, 1);
}

/**
 * @brief Read one uint32 from flash at given address. Checks magic value before reading. Use only in init or after acquiring flash_mutex
 * @param addr: Address in flash to read
 * @param data: Data output
 * @return Status of read
 */
HAL_StatusTypeDef _Flash_ReadFromFlash(uint32_t addr, uint32_t *data) {
    uint32_t flash_magic = *(__IO uint32_t*)FLASH_START_ADDRESS;

    // Flash hasn't been initialized
    if (flash_magic != FLASH_MAGIC) {
        return HAL_ERROR;
    }

    if (data == NULL) {
        return HAL_ERROR;
    }

    *data = *(__IO uint32_t*)addr;

    return HAL_OK;
}

/**
 * @brief (Non-blocking) write data in array to flash starting from given address. Use only in init or after acquiring flash_mutex
 * @param start_addr: Start address in flash of data to write
 * @param data_arr: Array of data to write, in 64 bit doublewords
 * @param data_length: Number of elements in array
 */
HAL_StatusTypeDef _Flash_WriteToFlash(Flash_Address_t start_addr, uint64_t *data_arr, uint16_t data_length) {
    // Unlock flash for writing
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return HAL_ERROR;
    }

    // Erase page containing data
    // STM32 requires erasing before write
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;

    uint32_t page_number = ((uint32_t)start_addr - FLASH_BASE) / FLASH_PAGE_SIZE;

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Page = page_number;
    erase_init.NbPages = 1;

    if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK) {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    for (uint16_t i = 0; i < data_length; i++) {
        uint32_t write_addr = (uint32_t)start_addr + (uint32_t)i * sizeof(uint64_t);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, write_addr, data_arr[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}
