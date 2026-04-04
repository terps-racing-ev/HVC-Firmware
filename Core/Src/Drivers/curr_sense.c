/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : curr_sense.c
  * @brief          : Implementation for curr sense driver
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

int32_t Curr_CalculateCurrentSenseHigh(uint32_t adc_value, uint32_t vref) {
  // Off datasheet
  // I = (Uout - U0) * 1/S
  // Uout = output voltage, which is voltage divider scale factor * (adc_value / 4096 * vref/1000)
  // Uo = 2.5
  // 1/S = 1/(0.004) = 250 (given by datasheet)
  // Measured voltage divider scale factor = 2.00200401

  return (int32_t)(122.19263 * vref * adc_value / 1000.0  - 625000);
}

int32_t Curr_CalculateCurrentSenseLow(uint32_t adc_value, uint32_t vref) {
  // Also off datasheet
  // I = (Uout - U0) * 1/S
  // Uout = output voltage, which from adc_value is voltage divider scale factor * (adc_value / 4096 * vref/1000)
  // Uo = 2.5
  // 1/S = 1/(0.0267) = 37.45318 (given by datasheet)
  // Measured voltage divider scale factor = 2.00100301

  return (int32_t)(18.2968586 * vref * adc_value / 1000.0 - 93632.9588);
}