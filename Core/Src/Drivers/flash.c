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
#include <stdbool.h>
#include <string.h>

typedef struct {
    uint32_t record_magic;
    uint32_t sequence;
    Flash_SOC_Data_t payload;
    uint32_t reserved;
    uint32_t commit_magic;
} Flash_SOC_RawRecord_t;

#define FLASH_RECORD_DOUBLEWORDS (sizeof(Flash_SOC_RawRecord_t) / sizeof(uint64_t))

static osMutexId_t flash_mutex = NULL;

static uint32_t _Flash_PageToAddress(Flash_SOC_Page_t page);
static bool _Flash_IsSequenceNewer(uint32_t left, uint32_t right);
static HAL_StatusTypeDef _Flash_ReadPageRecord(uint32_t page_addr, Flash_SOC_Record_t *record, bool *is_valid);
static HAL_StatusTypeDef _Flash_FindLatestRecord(Flash_SOC_Record_t *record, bool *is_valid, Flash_SOC_Page_t *page);
static HAL_StatusTypeDef _Flash_ErasePage(uint32_t page_addr);
static HAL_StatusTypeDef _Flash_ProgramDoubleWords(uint32_t start_addr, const uint64_t *data_arr, uint16_t data_length);
static HAL_StatusTypeDef _Flash_WriteRecordToPage(Flash_SOC_Page_t target_page, const Flash_SOC_Record_t *record);

// Lets unit tests simualte different flash values
uint32_t __attribute__((weak)) Flash_ReadWord(uint32_t addr) {
    return *(__IO uint32_t *)(uintptr_t)addr;
}

HAL_StatusTypeDef Flash_Init(void) {
    if (flash_mutex != NULL) {
        return HAL_OK;
    }

    const osMutexAttr_t flash_mutex_attr = {
        .name = "Flash_Mutex"
    };

    flash_mutex = osMutexNew(&flash_mutex_attr);
    if (flash_mutex == NULL) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef Flash_ReadSOCData(Flash_SOC_Data_t *data) {
    Flash_SOC_Record_t latest_record;
    bool has_record = false;

    if (data == NULL || flash_mutex == NULL) {
        return HAL_ERROR;
    }

    if (osMutexAcquire(flash_mutex, osWaitForever) != osOK) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = _Flash_FindLatestRecord(&latest_record, &has_record, NULL);

    if (status == HAL_OK && has_record) {
        *data = latest_record.payload;
    } else {
        status = HAL_ERROR;
    }

    (void)osMutexRelease(flash_mutex);
    return status;
}

HAL_StatusTypeDef Flash_WriteSOCData(Flash_SOC_Data_t *data) {
    Flash_SOC_Record_t latest_record;
    bool has_latest_record = false;
    Flash_SOC_Page_t latest_page = FLASH_SOC_PAGE_2;
    Flash_SOC_Record_t new_record;
    Flash_SOC_Page_t target_page;

    if (data == NULL || flash_mutex == NULL) {
        return HAL_ERROR;
    }

    if (osMutexAcquire(flash_mutex, osWaitForever) != osOK) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = _Flash_FindLatestRecord(&latest_record, &has_latest_record, &latest_page);
    if (status != HAL_OK) {
        (void)osMutexRelease(flash_mutex);
        return HAL_ERROR;
    }

    new_record.sequence = has_latest_record ? (latest_record.sequence + 1u) : 0u;
    new_record.payload = *data;

    if (!has_latest_record || latest_page == FLASH_SOC_PAGE_3) {
        target_page = FLASH_SOC_PAGE_2;
    } else {
        target_page = FLASH_SOC_PAGE_3;
    }

    status = _Flash_WriteRecordToPage(target_page, &new_record);

    (void)osMutexRelease(flash_mutex);
    return status;
}

static uint32_t _Flash_PageToAddress(Flash_SOC_Page_t page) {
    return (page == FLASH_SOC_PAGE_2) ? FLASH_SOC_PAGE_2_ADDRESS : FLASH_SOC_PAGE_3_ADDRESS;
}

static bool _Flash_IsSequenceNewer(uint32_t left, uint32_t right) {
    return left > right;
}

static HAL_StatusTypeDef _Flash_ReadPageRecord(uint32_t page_addr, Flash_SOC_Record_t *record, bool *is_valid) {
    Flash_SOC_RawRecord_t raw_record;
    uint32_t *record_words = (uint32_t *)&raw_record;
    const uint32_t total_words = (uint32_t)(sizeof(Flash_SOC_RawRecord_t) / sizeof(uint32_t));

    if (record == NULL || is_valid == NULL) {
        return HAL_ERROR;
    }

    for (uint32_t i = 0; i < total_words; i++) {
        record_words[i] = Flash_ReadWord(page_addr + (i * sizeof(uint32_t)));
    }

    if (raw_record.record_magic != FLASH_SOC_RECORD_MAGIC || raw_record.commit_magic != FLASH_SOC_COMMIT_MAGIC) {
        *is_valid = false;
        return HAL_OK;
    }

    record->sequence = raw_record.sequence;
    record->payload = raw_record.payload;
    *is_valid = true;

    return HAL_OK;
}

static HAL_StatusTypeDef _Flash_FindLatestRecord(Flash_SOC_Record_t *record, bool *is_valid, Flash_SOC_Page_t *page) {
    Flash_SOC_Record_t page2_record;
    Flash_SOC_Record_t page3_record;
    bool page2_valid = false;
    bool page3_valid = false;

    if (record == NULL || is_valid == NULL) {
        return HAL_ERROR;
    }

    if (_Flash_ReadPageRecord(FLASH_SOC_PAGE_2_ADDRESS, &page2_record, &page2_valid) != HAL_OK) {
        return HAL_ERROR;
    }
    if (_Flash_ReadPageRecord(FLASH_SOC_PAGE_3_ADDRESS, &page3_record, &page3_valid) != HAL_OK) {
        return HAL_ERROR;
    }

    if (!page2_valid && !page3_valid) {
        *is_valid = false;
        return HAL_OK;
    }

    if (!page3_valid || (page2_valid && _Flash_IsSequenceNewer(page2_record.sequence, page3_record.sequence))) {
        *record = page2_record;
        *is_valid = true;
        if (page != NULL) {
            *page = FLASH_SOC_PAGE_2;
        }
        return HAL_OK;
    }

    *record = page3_record;
    *is_valid = true;
    if (page != NULL) {
        *page = FLASH_SOC_PAGE_3;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef _Flash_ErasePage(uint32_t page_addr) {
    uint32_t page_number = (page_addr - FLASH_BASE) / FLASH_PAGE_SIZE;
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Page = page_number;
    erase_init.NbPages = 1;

    if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef _Flash_ProgramDoubleWords(uint32_t start_addr, const uint64_t *data_arr, uint16_t data_length) {
    if (data_arr == NULL) {
        return HAL_ERROR;
    }

    for (uint16_t i = 0; i < data_length; i++) {
        uint32_t write_addr = start_addr + ((uint32_t)i * sizeof(uint64_t));
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, write_addr, data_arr[i]) != HAL_OK) {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef _Flash_WriteRecordToPage(Flash_SOC_Page_t target_page, const Flash_SOC_Record_t *record) {
    Flash_SOC_RawRecord_t raw_record;
    uint64_t dwords[FLASH_RECORD_DOUBLEWORDS];
    uint32_t page_addr = _Flash_PageToAddress(target_page);

    if (record == NULL) {
        return HAL_ERROR;
    }

    memset(&raw_record, 0xFF, sizeof(raw_record));
    raw_record.record_magic = FLASH_SOC_RECORD_MAGIC;
    raw_record.sequence = record->sequence;
    raw_record.payload = record->payload;
    raw_record.commit_magic = FLASH_SOC_COMMIT_MAGIC;

    memcpy(dwords, &raw_record, sizeof(raw_record));

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = _Flash_ErasePage(page_addr);
    if (status == HAL_OK) {
        // Program all but the final doubleword first; commit marker is written last.
        status = _Flash_ProgramDoubleWords(page_addr, dwords, (uint16_t)(FLASH_RECORD_DOUBLEWORDS - 1u));
    }
    if (status == HAL_OK) {
        status = _Flash_ProgramDoubleWords(
            page_addr + (uint32_t)((FLASH_RECORD_DOUBLEWORDS - 1u) * sizeof(uint64_t)),
            &dwords[FLASH_RECORD_DOUBLEWORDS - 1u],
            1u
        );
    }

    HAL_FLASH_Lock();
    return status;
}

