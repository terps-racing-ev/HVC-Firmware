/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : curr_sense.h
  * @brief          : Header for curr sense driver
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
#include "stm32l4xx_hal.h"

// https://www.lem.com/sites/default/files/products_datasheets/dhab_s124_v5.pdf

// TODO: implement offset based on what voltage is actually 0A on current sensor

/**
 * @brief Convert current sense voltage from high channel to current in mA
 * @param adc_value: Raw reading from adc
 * @param vref: ADC reference voltage in mV
 */
int32_t Curr_CalculateCurrentSenseHigh(uint32_t adc_value, uint32_t vref);

/**
 * @brief Convert current sense voltage from low channel to current in mA
 * @param adc_value: Raw reading from adc
 * @param vref: ADC reference voltage in mV
 */
int32_t Curr_CalculateCurrentSenseLow(uint32_t adc_value, uint32_t vref);
