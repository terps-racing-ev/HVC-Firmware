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
#include "acc.h"
#include "vsense.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

uint8_t io_initialized = 0;
bool batt_floating = true; // When batt voltage is floating (<50V)

// Private function prototypes
static HAL_StatusTypeDef _IO_ConfigADCChannel(uint32_t channel);
static HAL_StatusTypeDef _IO_ReadADCChannel(uint32_t channel, uint16_t *out);
static uint32_t _IO_CalculateVREF(void);
static void _IO_LowPriority(void);
static void _IO_HighPriority(void);
static void _IO_HandleCompEvent(void);
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
static void _IO_PackVSenseMessage(
    uint8_t *data,
    uint8_t *length,
    uint32_t batt_val,
    uint32_t inv_val
);

HAL_StatusTypeDef IO_Manager_Init(void){
    const osEventFlagsAttr_t comp_flag_attr = {
        .name = "Comp_Flag"
    };

    comp_flag = osEventFlagsNew(&comp_flag_attr);
    if (comp_flag == NULL) {
        return HAL_ERROR;
    }

    // Calibrate ADC
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Delay to fix random ADC offsets
    for (int i = 0; i < 10000; i++) {};

    // Start comparator
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
    if (IO_InitAnalogIO(&batt_raw, "Batt_Raw_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitAnalogIO(&inv_raw, "Inv_Raw_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitTemp(&ref_temp, "Ref_Temp_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitCurrent(&cs_low, "CS_Low_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitCurrent(&cs_high, "CS_High_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitVSense(&batt, "Batt_Mutex") != HAL_OK) return HAL_ERROR;
    if (IO_InitVSense(&inv, "Inv_Mutex") != HAL_OK) return HAL_ERROR;

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
        _IO_HighPriority();

        // Handle comparator logic if triggered
        uint32_t comp_events = osEventFlagsWait(comp_flag, IO_COMP_EVENT, osFlagsWaitAny, 0U);
        if (((comp_events & osFlagsError) == 0U) && ((comp_events & IO_COMP_EVENT) != 0U)) {
            osEventFlagsClear(comp_flag, IO_COMP_EVENT);
            _IO_HandleCompEvent();
        }

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

static void _IO_HandleCompEvent(void)
{
    
    uint32_t comp_state = HAL_COMP_GetOutputLevel(&hcomp2);
    State bms_state; State_GetState(&bms_state);
    uint32_t batt_volt = IO_GetVSense(&batt);

    if (batt_floating && batt_volt >= IO_MAX_BATT_FLOATING_VOLTAGE_MV) {
        batt_floating = false;
    } else if (!batt_floating && batt_volt <= IO_MIN_BATT_FLOATING_VOLTAGE_MV) {
        batt_floating = true;
    }

    // Conditions to check before allowing AIRTOP to close
    if (
        bms_state == ERRORED || 
        batt_floating
    ) {
        HAL_GPIO_WritePin(PL_SIGNAL_GPIO_Port, PL_SIGNAL_Pin, GPIO_PIN_RESET);
    } else if (bms_state == CHARGING) { // If charging ignore normal comp signal
        HAL_GPIO_WritePin(PL_SIGNAL_GPIO_Port, PL_SIGNAL_Pin, GPIO_PIN_SET);
    } else { // Normal operation
        if (comp_state == COMP_OUTPUT_LEVEL_LOW) {
            HAL_GPIO_WritePin(PL_SIGNAL_GPIO_Port, PL_SIGNAL_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(PL_SIGNAL_GPIO_Port, PL_SIGNAL_Pin, GPIO_PIN_SET);
        }
    }

}

static void _IO_LowPriority(void)
{
    uint8_t can_data[8];
    uint8_t can_data_len;
    float temp;
    bool sdc_raw;
    bool imd_raw;
    uint16_t therm_raw = IO_GetAnalogIO(&therm);

    // Read SDC + IMD
    sdc_raw = (HAL_GPIO_ReadPin(SDC_GPIO_Port, SDC_Pin) == GPIO_PIN_SET);
    imd_raw = (HAL_GPIO_ReadPin(IMD_GPIO_Port, IMD_Pin) == GPIO_PIN_SET);
    IO_SetDigitalIO(&sdc, sdc_raw);
    IO_SetDigitalIO(&imd, imd_raw);

    // Read Therm
    if (_IO_ReadADCChannel(ADC_CHANNEL_15, &therm_raw) == HAL_OK) {
        IO_SetAnalogIO(&therm, therm_raw);
    }

    // Calculate and update temp value
    temp = Therm_CalculateTemperature(therm_raw);
    IO_SetTemp(&ref_temp, temp);

    // TODO: Emeter temps?

    // Send summary message
    _IO_PackIOSummary(
        can_data,
        &can_data_len,
        sdc_raw,
        imd_raw,
        temp,
        IO_GetDigitalIO(&bms_fault)
    );
    LV_CAN_SendMessage(
        CAN_ID_IO_SUMMARY,
        can_data,
        can_data_len,
        CAN_PRIORITY_NORMAL
    );

    // TODO: make this max current in an interval
    _IO_PackCurrentSenseMessage(
        can_data,
        &can_data_len,
        IO_GetCurrent(&cs_low),
        IO_GetCurrent(&cs_high)
    );
    LV_CAN_SendMessage(
        CAN_ID_IO_CURRENT,
        can_data,
        can_data_len,
        CAN_PRIORITY_NORMAL
    );
    BMS_CAN_SendMessage(
        CAN_ID_IO_CURRENT,
        can_data,
        can_data_len,
        CAN_PRIORITY_NORMAL
    );
}

static void _IO_HighPriority(void)
{
    uint8_t current_summary[8], current_summary_len;
    uint8_t vsense_summary[8], vsense_summary_len;
    uint16_t cs_low_raw_val = IO_GetAnalogIO(&cs_low_raw);
    uint16_t cs_high_raw_val = IO_GetAnalogIO(&cs_high_raw);
    uint16_t batt_raw_val = IO_GetAnalogIO(&batt_raw);
    uint16_t inv_raw_val = IO_GetAnalogIO(&inv_raw);
    int32_t cs_low_val, cs_high_val, cs_low_filt_val, cs_high_filt_val, timestamp;
    uint32_t vref;
    uint32_t batt_voltage;
    uint32_t inv_voltage;
    uint32_t batt_voltage_filt;
    uint32_t inv_voltage_filt;
    bool fault;

    // Write BMS Fault (1 is good)
    fault = IO_GetDigitalIO(&bms_fault);
    if (fault) HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(BMS_Fault_GPIO_Port, BMS_Fault_Pin, GPIO_PIN_RESET);
    
    // Calculate ADC reference voltage
    vref = _IO_CalculateVREF();

    // Keep the previous sample on conversion failure so bad startup reads
    // cannot inject uninitialized stack values into filters.
    if (_IO_ReadADCChannel(ADC_CHANNEL_BATT, &batt_raw_val) == HAL_OK) {
        IO_SetAnalogIO(&batt_raw, batt_raw_val);
    }
    if (_IO_ReadADCChannel(ADC_CHANNEL_INV, &inv_raw_val) == HAL_OK) {
        IO_SetAnalogIO(&inv_raw, inv_raw_val);
    }
    if (_IO_ReadADCChannel(ADC_CHANNEL_CS_LC, &cs_low_raw_val) == HAL_OK) {
        IO_SetAnalogIO(&cs_low_raw, cs_low_raw_val);
    }
    if (_IO_ReadADCChannel(ADC_CHANNEL_CS_HC, &cs_high_raw_val) == HAL_OK) {
        IO_SetAnalogIO(&cs_high_raw, cs_high_raw_val);
    }

    // Calculate batt value
    // TODO: Use vref here also
    // Scale 
    batt_voltage = (uint32_t)(VSense_CalculateVoltage(batt_raw_val) * INVERSE_PRECHARGE_FACTOR);
    inv_voltage = VSense_CalculateVoltage(inv_raw_val);
    // Calculate cs values
    cs_low_val = Curr_CalculateCurrentSenseLow(cs_low_raw_val, vref);
    cs_high_val = Curr_CalculateCurrentSenseHigh(cs_high_raw_val, vref);

    // Update moving average
    cs_low_filt_val = MovingAverage_Update(&cs_low.ma, cs_low_val);
    cs_high_filt_val = MovingAverage_Update(&cs_high.ma, cs_high_val);
    batt_voltage_filt = MovingAverage_Update(&batt.ma, batt_voltage);
    inv_voltage_filt = MovingAverage_Update(&inv.ma, inv_voltage);
    
    // Set cs values
    IO_SetCurrent(&cs_low, cs_low_filt_val);
    IO_SetCurrent(&cs_high, cs_high_filt_val);
    IO_SetVSense(&batt, batt_voltage_filt);
    IO_SetVSense(&inv, inv_voltage_filt);

    timestamp = osKernelGetTickCount();
    (void)Acc_CurrSenseQueue_Push(cs_low_filt_val, cs_high_filt_val, timestamp, 0U);

    _IO_PackVSenseMessage(
        vsense_summary,
        &vsense_summary_len,
        batt_voltage_filt,
        inv_voltage_filt
    );
    LV_CAN_SendMessage(
        CAN_ID_IO_VSENSE,
        vsense_summary,
        vsense_summary_len,
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
    sConfig.SamplingTime = ADC_SAMPLE_TIME; // Longer cycle time = higher impedance
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
    uint32_t adc_value;

    if (out == NULL) {
        return HAL_ERROR;
    }

    if (_IO_ConfigADCChannel(channel) != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return HAL_ERROR;
    }
    
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        (void)HAL_ADC_Stop(&hadc1);
        return HAL_ERROR;
    }

    adc_value = HAL_ADC_GetValue(&hadc1);

    if (HAL_ADC_Stop(&hadc1) != HAL_OK) {
        return HAL_ERROR;
    }

    *out = (uint16_t)adc_value;
    return HAL_OK;
}

/**
  * @brief  Calculate ADC reference voltage
  * @retval Reference voltage in mV
  */
static uint32_t _IO_CalculateVREF(void)
{
    uint16_t vref_adc_value = 0U;
    static uint32_t vref_voltage = 3300U;

    if (_IO_ReadADCChannel(ADC_CHANNEL_VREFINT, &vref_adc_value) == HAL_OK) {
        uint32_t measured_vref = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vref_adc_value, ADC_RESOLUTION_12B);

        // Reject implausible Vref readings and keep last-known-good value.
        if ((measured_vref >= 2800U) && (measured_vref <= 3600U)) {
            vref_voltage = measured_vref;
        }
    }

    return vref_voltage;

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

static void _IO_PackVSenseMessage(
    uint8_t *data,
    uint8_t *length,
    uint32_t batt_val,
    uint32_t inv_val
){
    if ((data == NULL) || (length == NULL)) {
        return;
    }

    data[0] = (uint8_t)(batt_val & 0xFFU);
    data[1] = (uint8_t)((batt_val >> 8U) & 0xFFU);
    data[2] = (uint8_t)((batt_val >> 16U) & 0xFFU);
    data[3] = (uint8_t)((batt_val >> 24U) & 0xFFU);
    data[4] = (uint8_t)(inv_val & 0xFFU);
    data[5] = (uint8_t)((inv_val >> 8U) & 0xFFU);
    data[6] = (uint8_t)((inv_val >> 16U) & 0xFFU);
    data[7] = (uint8_t)((inv_val >> 24U) & 0xFFU);
    *length = 8U;
}

