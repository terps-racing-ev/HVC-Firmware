/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : therm.c
  * @brief          : Handler for calculating thermistor temps
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

#include "therm.h"

/**
  * @brief  Calculate temperature from thermistor ADC reading using B-parameter equation
  * @param  adc_value: Raw ADC value (0-4095)
  * @retval Temperature in degrees Celsius
  */
float Therm_CalculateTemperature(uint16_t adc_value)
{
    // Convert ADC value to voltage
    float voltage = ((float)adc_value / ADC_RESOLUTION) * ADC_VREF;
    
    // Check for disconnected thermistor (very low ADC reading means low voltage = open circuit)
    if (adc_value < 10) {  // Less than ~8mV indicates disconnected sensor
        return -127.0f;  // Return obvious error value
    }
    
    // Calculate thermistor resistance using voltage divider
    // Circuit: 3.3V -> 10kΩ pullup -> ADC_input -> Thermistor -> GND
    // Voltage divider: V_adc = 3.3V * (R_thermistor / (R_pullup + R_thermistor))
    // Solving for R_thermistor: R_thermistor = (V_adc * R_pullup) / (3.3V - V_adc)
    
    float r_thermistor;
    
    if (voltage >= 3.29f) {  // Close to 3.3V limit, very high resistance (cold)
        r_thermistor = PULLUP_RESISTOR * 100.0f;  // Assume very high resistance
    } else {
        // Normal calculation with 3.3V supply
        r_thermistor = (voltage * PULLUP_RESISTOR) / (ADC_VREF - voltage);
    }
    
    // Handle edge cases
    if (r_thermistor <= 0) {
        return 125.0f; // Return maximum temperature for very low resistance
    }
    
    // Calculate temperature using B-parameter equation (derived from Steinhart-Hart)
    // 1/T = 1/T0 + (1/B) * ln(R/R0)
    // Where T0 = 298.15K (25°C), R0 = resistance at 25°C, B = B-value
    float ln_ratio = logf(r_thermistor / THERMISTOR_R25);
    float temp_kelvin = 1.0f / ((1.0f / REFERENCE_TEMP_K) + (ln_ratio / THERMISTOR_B_VALUE));
    
    // Convert to Celsius
    float temp_celsius = temp_kelvin - 273.15f;
    
    // Clamp to reasonable range
    if (temp_celsius < -40.0f) {
        temp_celsius = -40.0f;
    } else if (temp_celsius > 125.0f) {
        temp_celsius = 125.0f;
    }
    
    return temp_celsius;
}