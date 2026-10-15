/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : therm.h
  * @brief          : Header for calculating thermistor temps
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

// Calculated from resistor divider
#define READ_VOLT_TO_INPUT_VOLT 509.692307692f

/**
  * @brief  Calculate voltage from vsense ADC reading
  * @param  adc_value: Raw ADC value (0-4095)
  * @retval Voltage in mV
  */
uint32_t VSense_CalculateVoltage(uint16_t adc_value);

