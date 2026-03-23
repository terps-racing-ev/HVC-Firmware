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

uint8_t io_initialized = 0;

// External ADC handle (from main.c)
extern ADC_HandleTypeDef hadc1;

// IO structs
DigitalIO sdc = {0};
DigitalIO imd = {0};
DigitalIO bms_fault = {0};
AnalogIO cs_low_raw = {0};
AnalogIO cs_high_raw = {0};
AnalogIO therm = {0};
Temp ref_temp = {0};
Current cs_low = {0};
Current cs_high = {0};

// Private function prototypes
static HAL_StatusTypeDef _IO_ConfigADCChannel(uint32_t channel);

HAL_StatusTypeDef IO_Manager_Init(void){
    // Start hardware
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_COMP_Start(&hcomp2) != HAL_OK) {
        return HAL_ERROR;
    }

    // Data init
    if (IO_InitDigitalIO(&sdc, "SDC_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitDigitalIO(&imd, "IMD_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitDigitalIO(&bms_fault, "BMS_Fault_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitAnalogIO(&cs_low_raw, "CS_Low_Raw_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitAnalogIO(&cs_high_raw, "CS_High_Raw_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitAnalogIO(&therm, "Therm_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitTemp(&ref_temp, "Ref_Temp_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitCurrent(&cs_low, "CS_Low_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitCurrent(&cs_high, "CS_High_Mutex") != HAL_OK) return HAL_ERROR;

    io_initialized = 1;

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
    GPIO_PinState sdc_raw;
    GPIO_PinState imd_raw;
    uint16_t therm_raw;

    for (;;) {
        // Read SDC + IMD
        sdc_raw = HAL_GPIO_ReadPin(SDC_GPIO_Port, SDC_Pin);
        imd_raw = HAL_GPIO_ReadPin(IMD_GPIO_Port, IMD_Pin);
        IO_SetDigitalIO(&sdc, sdc_raw);
        IO_SetDigitalIO(&imd, imd_raw);
        
        // Read Therm
        // TODO: handle HAL_ERROR return
        _IO_ReadADCChannel(ADC_CHANNEL_8, &therm_raw);
        IO_SetAnalogIO(&therm, therm_raw);

        // Calculate and update temp value
        temp = Therm_CalculateTemperature(therm_raw);
        IO_SetTemp(&ref_temp, temp);

        // TODO: Emeter temps?

        // Send summary message
        _IO_PackIOSummary(
            io_summary, 
            &io_summary_len,
            sdc_raw,
            imd_raw,
            therm_raw,
            IO_GetDigitalIO(&bms_fault),    // These values are set in the priority task
            IO_GetAnalogIO(&cs_low_raw),
            IO_GetAnalogIO(&cs_high_raw)
        );
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
  * @brief  Main IO manager task (for priority io values). Runs at faster rate.
  * @param  argument: Not used
  * @retval None
  */
void IO_PriorityManagerTask(void *argument) {
    uint16_t cs_low_raw_val;
    uint16_t cs_high_raw_val;
    int32_t cs_low_val_mA;
    int32_t cs_high_val_mA;
    uint8_t fault;
    uint32_t now;

    for (;;) {
        now = osKernelGetTickCount();

        // Write BMS Fault (1 is good)
        fault = IO_GetDigitalIO(&bms_fault);
        if (fault) HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_RESET);

        // Read CS_LOW (Channel 5 - PA0)
        _IO_ReadADCChannel(ADC_CHANNEL_5, &cs_low_raw_val);
        IO_SetAnalogIO(&cs_low_raw, cs_low_raw_val);
        // Read CS_HIGH (Channel 6 - PA1)
        _IO_ReadADCChannel(ADC_CHANNEL_6, &cs_high_raw_val);
        IO_SetAnalogIO(&cs_high_raw, cs_high_raw_val);

        // Set cs values
        cs_low_val_mA = Curr_CalculateCurrentSenseLow(cs_low_raw_val);
        IO_SetCurrent(&cs_low, cs_low_val_mA);
        cs_high_val_mA = Curr_CalculateCurrentSenseHigh(cs_high_raw_val);
        IO_SetCurrent(&cs_high, cs_high_val_mA);


        // // Use a queue so curr sense reading is dependent on how fast
        // // this task reads adc instead of how fast acc manager
        // // processes values
        // Acc_AddSocCurrSense(cs_low_val_mA, cs_high_val_mA, now);
        
        osDelay(IO_PRIORITY_UPDATE_FREQ_MS);
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
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5; // Longer cycle time = higher impedance
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    
    return HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/**
 * @brief Read ADC value from currently configured channel
 * @retval ADC converted value (0-4095)
 */
HAL_StatusTypeDef _IO_ReadADCChannel(uint32_t channel, uint16_t *out)
{   
    if (_IO_ConfigADCChannel(channel) != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return HAL_ERROR;
    }
    
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        return HAL_ERROR;
    }
    
    *out = HAL_ADC_GetValue(&hadc1);
    return HAL_OK;
}

void _IO_PackIOSummary(
    uint8_t *data, 
    uint8_t *length,
    uint8_t sdc_val,
    uint8_t imd_val,
    uint16_t therm_val,
    uint8_t bms_fault_val,
    uint16_t cs_low_val,
    uint16_t cs_high_val
){

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
    data[0] |= (uint8_t)(sdc_val & 0x01U);

    // 2) imd (bit 1)
    data[0] |= (uint8_t)((imd_val & 0x01U) << 1U);

    // 3) bms_fault (bit 2)
    data[0] |= (uint8_t)((bms_fault_val & 0x01U) << 2U);

    // 4) cs_low (little-endian uint16_t)
    data[1] = (uint8_t)(cs_low_val & 0xFFU);
    data[2] = (uint8_t)((cs_low_val >> 8U) & 0xFFU);

    // 5) cs_high (little-endian uint16_t)
    data[3] = (uint8_t)(cs_high_val & 0xFFU);
    data[4] = (uint8_t)((cs_high_val >> 8U) & 0xFFU);

    // 6) therm (little-endian uint16_t)
    data[5] = (uint8_t)(therm_val & 0xFFU);
    data[6] = (uint8_t)((therm_val >> 8U) & 0xFFU);
}

