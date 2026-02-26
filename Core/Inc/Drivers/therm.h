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
// TODO: Implement logf directly since thats the only function in this used
#include <math.h>

// ADC parameters
#define ADC_RESOLUTION 4095.0f      // 12-bit ADC resolution (0-4095)
#define ADC_VREF 3.3f               // ADC reference voltage (3.3V)

// Thermistor parameters (NTC 10k ohm @ 25°C, B-value 4300K)
#define THERMISTOR_R25 10000.0f     // Resistance at 25°C in ohms
#define THERMISTOR_B_VALUE 3435.0f  // B25/85 value of thermistor
#define PULLUP_RESISTOR 10000.0f    // Pull-up resistor value in ohms (to 3.3V)
#define REFERENCE_TEMP_K 298.15f    // Reference temperature in Kelvin (25°C)

float Therm_CalculateTemperature(uint16_t adc_value);