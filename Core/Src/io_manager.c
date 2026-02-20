/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : io_manager.c
  * @brief          : IO manager implementation
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

#include "io_manager.h"
#include "cmsis_os2.h"
#include "stm32l4xx_hal.h"

// External ADC handle (from main.c)
extern ADC_HandleTypeDef hadc1;

// IO structs
DigitalIO sdc = {0};
DigitalIO imd = {0};
DigitalIO bms_fault = {0};
AnalogIO cs_low = {0};
AnalogIO cs_high = {0};

// Private function prototypes
static HAL_StatusTypeDef _IO_ConfigADCChannel(uint32_t channel);
static uint16_t _IO_ReadADCChannel(void);
static void _IO_UpdateDigitalIO(void);
static void _IO_UpdateAnalogIO(void);

HAL_StatusTypeDef IO_Manager_Init(void){
    // Start hardware
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_COMP_Start(&hcomp2) != HAL_OK) {
        return HAL_ERROR;
    }

    // Mutex init
    const osMutexAttr_t sdc_mutex_attr = {
        .name = "SDC_Mutex"
    };
    sdc.mutex = osMutexNew(&sdc_mutex_attr);
    if (!sdc.mutex) return HAL_ERROR;

    const osMutexAttr_t imd_mutex_attr = {
        .name = "IMD_Mutex"
    };
    imd.mutex = osMutexNew(&imd_mutex_attr);
    if (!imd.mutex) return HAL_ERROR;

    const osMutexAttr_t bms_fault_mutex_attr = {
        .name = "BMS_Fault_Mutex"
    };
    bms_fault.mutex = osMutexNew(&bms_fault_mutex_attr);
    if (!bms_fault.mutex) return HAL_ERROR;

    const osMutexAttr_t cs_low_mutex_attr = {
        .name = "CS_Low_Mutex"
    };
    cs_low.mutex = osMutexNew(&cs_low_mutex_attr);
    if (!cs_low.mutex) return HAL_ERROR;

    const osMutexAttr_t cs_high_mutex_attr = {
        .name = "CS_High_Mutex"
    };
    cs_high.mutex = osMutexNew(&cs_high_mutex_attr);
    if (!cs_high.mutex) return HAL_ERROR;

    // Initialize all IO values
    sdc.value = 0;
    sdc.last_updated = 0;

    imd.value = 0;
    imd.last_updated = 0;

    bms_fault.value = 0;    // Start faulted
    bms_fault.last_updated = 0;

    cs_low.value = 0;
    cs_low.last_updated = 0;

    cs_high.value = 0;
    cs_high.last_updated = 0;

    return HAL_OK;
}

/**
 * @brief Main IO manager task
 * @param argument: Not used
 * @retval None
 */
void IO_ManagerTask(void *argument){
    for (;;) {
        // Read SDC + IMD, write BMS Fault
        _IO_UpdateDigitalIO();
        // Read Curr Sense high and low
        _IO_UpdateAnalogIO();

        // Todo: Add values to LV can

        osDelay(10);  // 10ms loop
    }
}

/**
 * @brief Configure ADC for a specific channel
 * @param channel ADC channel (ADC_CHANNEL_5, ADC_CHANNEL_6, etc.)
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef _IO_ConfigADCChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    
    return HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/**
 * @brief Read ADC value from currently configured channel
 * @retval ADC converted value (0-4095)
 */
static uint16_t _IO_ReadADCChannel(void)
{
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return 0;
    }
    
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        return 0;
    }
    
    return HAL_ADC_GetValue(&hadc1);
}

/**
 * @brief Read and write all digital IO values
 */
static void _IO_UpdateDigitalIO(void)
{
    GPIO_PinState sdc_raw = HAL_GPIO_ReadPin(SDC_GPIO_Port, SDC_Pin);
    GPIO_PinState imd_raw = HAL_GPIO_ReadPin(IMD_GPIO_Port, IMD_Pin);
    uint32_t now = osKernelGetTickCount();
    
    // Read SDC
    osMutexAcquire(sdc.mutex, osWaitForever);
    sdc.value = sdc_raw;
    sdc.last_updated = now;
    osMutexRelease(sdc.mutex);
    
    // Read IMD
    osMutexAcquire(imd.mutex, osWaitForever);
    imd.value = imd_raw;
    imd.last_updated = now;
    osMutexRelease(imd.mutex);

    // Write BMS Fault
    osMutexAcquire(bms_fault.mutex, osWaitForever);
    if (bms_fault.value) HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_RESET);
    osMutexRelease(bms_fault.mutex);
}

/**
 * @brief Read and update all analog IO values
 */
static void _IO_UpdateAnalogIO(void)
{
    uint32_t now = osKernelGetTickCount();
    
    // Read CS_LOW (Channel 5 - PA0)
    _IO_ConfigADCChannel(ADC_CHANNEL_5);
    uint16_t cs_low_raw = _IO_ReadADCChannel();
    
    osMutexAcquire(cs_low.mutex, osWaitForever);
    cs_low.value = cs_low_raw;
    cs_low.last_updated = now;
    osMutexRelease(cs_low.mutex);
    
    // Read CS_HIGH (Channel 6 - PA1)
    _IO_ConfigADCChannel(ADC_CHANNEL_6);
    uint16_t cs_high_raw = _IO_ReadADCChannel();
    
    osMutexAcquire(cs_high.mutex, osWaitForever);
    cs_high.value = cs_high_raw;
    cs_high.last_updated = now;
    osMutexRelease(cs_high.mutex);
}