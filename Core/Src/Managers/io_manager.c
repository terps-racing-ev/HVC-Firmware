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
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

// External ADC handle (from main.c)
extern ADC_HandleTypeDef hadc1;

// IO structs
DigitalIO sdc = {0};
DigitalIO imd = {0};
DigitalIO bms_fault = {0};
AnalogIO cs_low = {0};
AnalogIO cs_high = {0};
AnalogIO therm = {0};
Temp ref_temp = {0};

// Private function prototypes
static HAL_StatusTypeDef _IO_ConfigADCChannel(uint32_t channel);
static uint16_t _IO_ReadADCChannel(void);
static void _IO_UpdateDigitalIO(void);
static void _IO_UpdateAnalogIO(void);
static void _IO_UpdateTemps(void);
static void _IO_PackIOSummary(uint8_t *data, uint8_t *length);

HAL_StatusTypeDef IO_Manager_Init(void){
    // Start hardware
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_COMP_Start(&hcomp2) != HAL_OK) {
        return HAL_ERROR;
    }

    // Mutex init
    // TODO: move declarations into io.h
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

    const osMutexAttr_t therm_mutex_attr = {
        .name = "Therm_Mutex"
    };
    therm.mutex = osMutexNew(&therm_mutex_attr);
    if (!therm.mutex) return HAL_ERROR;

    const osMutexAttr_t ref_temp_mutex_attr = {
        .name = "Ref_Temp_Mutex"
    };
    ref_temp.mutex = osMutexNew(&ref_temp_mutex_attr);
    if (!ref_temp.mutex) return HAL_ERROR;

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

    therm.value = 0;
    therm.last_updated = 0;

    ref_temp.value = 0.0f;
    ref_temp.last_updated = 0;

    return HAL_OK;
}

/**
 * @brief Main IO manager task
 * @param argument: Not used
 * @retval None
 */
void IO_ManagerTask(void *argument){
    uint8_t io_summary[8];
    uint8_t io_summary_len;
    float temp;

    for (;;) {
        // Read SDC + IMD, write BMS Fault
        _IO_UpdateDigitalIO();
        // Read Curr Senses and Therm
        _IO_UpdateAnalogIO();
        // Calculate and update temp values
        _IO_UpdateTemps();

        // TODO: Emeter temps?

        // Send summary message
        _IO_PackIOSummary(io_summary, &io_summary_len);
        LV_CAN_SendMessage(
            CAN_ID_IO_SUMMARY,
            io_summary,
            io_summary_len,
            CAN_PRIORITY_NORMAL
        );

        osDelay(IO_UPDATE_FREQ_MS);
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
    IO_SetDigitalIO(&sdc, sdc_raw);

    GPIO_PinState imd_raw = HAL_GPIO_ReadPin(IMD_GPIO_Port, IMD_Pin);
    IO_SetDigitalIO(&imd, imd_raw);

    uint8_t fault = IO_GetDigitalIO(&bms_fault);
    if (bms_fault.value) HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Read and update all analog IO values
 */
static void _IO_UpdateAnalogIO(void)
{
    // Read CS_LOW (Channel 5 - PA0)
    _IO_ConfigADCChannel(ADC_CHANNEL_5);
    uint16_t cs_low_raw = _IO_ReadADCChannel();
    IO_SetAnalogIO(&cs_low, cs_low_raw);
    
    // Read CS_HIGH (Channel 6 - PA1)
    _IO_ConfigADCChannel(ADC_CHANNEL_6);
    uint16_t cs_high_raw = _IO_ReadADCChannel();
    IO_SetAnalogIO(&cs_high, cs_high_raw);

    _IO_ConfigADCChannel(ADC_CHANNEL_8);
    uint16_t therm_raw = _IO_ReadADCChannel();
    IO_SetAnalogIO(&therm, therm_raw);
}

static void _IO_PackIOSummary(uint8_t *data, uint8_t *length)
{
    if ((data == NULL) || (length == NULL)) {
        return;
    }

    // Default payload and fixed packed length
    data[0] = 0U;
    data[1] = 0U;
    data[2] = 0U;
    data[3] = 0U;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;
    *length = 7U;

    // 1) sdc (bit 0)
    osMutexAcquire(sdc.mutex, osWaitForever);
    data[0] |= (uint8_t)(sdc.value & 0x01U);
    osMutexRelease(sdc.mutex);

    // 2) imd (bit 1)
    osMutexAcquire(imd.mutex, osWaitForever);
    data[0] |= (uint8_t)((imd.value & 0x01U) << 1U);
    osMutexRelease(imd.mutex);

    // 3) bms_fault (bit 2)
    osMutexAcquire(bms_fault.mutex, osWaitForever);
    data[0] |= (uint8_t)((bms_fault.value & 0x01U) << 2U);
    osMutexRelease(bms_fault.mutex);

    // 4) cs_low (little-endian uint16_t)
    osMutexAcquire(cs_low.mutex, osWaitForever);
    data[1] = (uint8_t)(cs_low.value & 0xFFU);
    data[2] = (uint8_t)((cs_low.value >> 8U) & 0xFFU);
    osMutexRelease(cs_low.mutex);

    // 5) cs_high (little-endian uint16_t)
    osMutexAcquire(cs_high.mutex, osWaitForever);
    data[3] = (uint8_t)(cs_high.value & 0xFFU);
    data[4] = (uint8_t)((cs_high.value >> 8U) & 0xFFU);
    osMutexRelease(cs_high.mutex);

    // 6) therm (little-endian uint16_t)
    osMutexAcquire(therm.mutex, osWaitForever);
    data[5] = (uint8_t)(therm.value & 0xFFU);
    data[6] = (uint8_t)((therm.value >> 8U) & 0xFFU);
    osMutexRelease(therm.mutex);
}

/**
 * @brief Update temp values
 */
static void _IO_UpdateTemps(void) {
    uint16_t therm_adc_val;
    float temp;
    
    therm_adc_val = IO_GetAnalogIO(&therm);
    temp = Therm_CalculateTemperature(therm_adc_val);
    IO_SetTemp(&ref_temp, temp);
}