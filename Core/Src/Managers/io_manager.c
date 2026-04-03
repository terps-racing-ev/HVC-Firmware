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
static HAL_StatusTypeDef _IO_ReadADCChannel(uint32_t channel, uint16_t *out);
static void _IO_LowPriority(void);
static void _IO_Handle_CurrSense_Fault(void);
static void _IO_PackIOSummary(
    uint8_t *data, 
    uint8_t *length,
    bool sdc_val,
    bool imd_val,
    float temp_val,
    bool bms_fault_val
);
static void _IO_PackCurrentSenseMessage(
    uint8_t *data,
    uint8_t *length,
    int32_t cs_low_val,
    int32_t cs_high_val
);

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
 * @brief Combined IO manager task. Runs at the priority IO rate and
 *        schedules the slower summary work from the same thread.
 * @param argument: Not used
 * @retval None
 */
void IO_ManagerTask(void *argument){
    uint32_t regular_cycle_divider = 0U;
    const uint32_t regular_cycle_period =
        (IO_UPDATE_FREQ_MS + IO_PRIORITY_UPDATE_FREQ_MS - 1U) / IO_PRIORITY_UPDATE_FREQ_MS;

    (void)argument;

    for (;;) {
        _IO_Handle_CurrSense_Fault();

        if (regular_cycle_divider == 0U) {
            _IO_LowPriority();
        }

        regular_cycle_divider++;
        if (regular_cycle_divider >= regular_cycle_period) {
            regular_cycle_divider = 0U;
        }

        osDelay(IO_PRIORITY_UPDATE_FREQ_MS);
    }
}

static void _IO_LowPriority(void)
{
    uint8_t io_summary[8];
    uint8_t io_summary_len;
    float temp;
    bool sdc_raw;
    bool imd_raw;
    uint16_t therm_raw;

    // Read SDC + IMD
    sdc_raw = (HAL_GPIO_ReadPin(SDC_GPIO_Port, SDC_Pin) == GPIO_PIN_SET);
    imd_raw = (HAL_GPIO_ReadPin(IMD_GPIO_Port, IMD_Pin) == GPIO_PIN_SET);
    IO_SetDigitalIO(&sdc, sdc_raw);
    IO_SetDigitalIO(&imd, imd_raw);

    // Read Therm
    // TODO: handle HAL_ERROR return
    _IO_ReadADCChannel(ADC_CHANNEL_15, &therm_raw);
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
        temp,
        IO_GetDigitalIO(&bms_fault)
    );
    LV_CAN_SendMessage(
        CAN_ID_IO_SUMMARY,
        io_summary,
        io_summary_len,
        CAN_PRIORITY_NORMAL
    );
}

static void _IO_Handle_CurrSense_Fault(void)
{
    uint8_t current_summary[8];
    uint8_t current_summary_len;
    uint16_t cs_low_raw_val;
    uint16_t cs_high_raw_val;
    int32_t cs_low_val;
    int32_t cs_high_val;
    bool fault;

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
    cs_low_val = Curr_CalculateCurrentSenseLow(cs_low_raw_val);
    IO_SetCurrent(&cs_low, cs_low_val);
    cs_high_val = Curr_CalculateCurrentSenseHigh(cs_high_raw_val);
    IO_SetCurrent(&cs_high, cs_high_val);

    _IO_PackCurrentSenseMessage(
        current_summary,
        &current_summary_len,
        cs_low_val,
        cs_high_val
    );
    LV_CAN_SendMessage(
        CAN_ID_IO_CURRENT,
        current_summary,
        current_summary_len,
        CAN_PRIORITY_NORMAL
    );
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
static HAL_StatusTypeDef _IO_ReadADCChannel(uint32_t channel, uint16_t *out)
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

static void _IO_PackIOSummary(
    uint8_t *data, 
    uint8_t *length,
    bool sdc_val,
    bool imd_val,
    float temp_val,
    bool bms_fault_val
){
    int16_t temp_c_x100;

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

    // 4) temp (little-endian int16_t, degrees C x100)
    temp_c_x100 = (int16_t)(temp_val * 100.0f);
    data[5] = (uint8_t)(temp_c_x100 & 0xFF);
    data[6] = (uint8_t)(((uint16_t)temp_c_x100 >> 8U) & 0xFFU);
}

static void _IO_PackCurrentSenseMessage(
    uint8_t *data,
    uint8_t *length,
    int32_t cs_low_val,
    int32_t cs_high_val
){
    uint32_t cs_low_u;
    uint32_t cs_high_u;

    if ((data == NULL) || (length == NULL)) {
        return;
    }

    cs_low_u = (uint32_t)cs_low_val;
    cs_high_u = (uint32_t)cs_high_val;

    data[0] = (uint8_t)(cs_low_u & 0xFFU);
    data[1] = (uint8_t)((cs_low_u >> 8U) & 0xFFU);
    data[2] = (uint8_t)((cs_low_u >> 16U) & 0xFFU);
    data[3] = (uint8_t)((cs_low_u >> 24U) & 0xFFU);
    data[4] = (uint8_t)(cs_high_u & 0xFFU);
    data[5] = (uint8_t)((cs_high_u >> 8U) & 0xFFU);
    data[6] = (uint8_t)((cs_high_u >> 16U) & 0xFFU);
    data[7] = (uint8_t)((cs_high_u >> 24U) & 0xFFU);
    *length = 8U;
}

