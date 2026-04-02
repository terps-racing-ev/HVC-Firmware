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

int32_t Curr_CalculateCurrentSenseHigh(uint32_t adc_value) {
  // Off datasheet
  // I = (Uout - U0) * 1/S
  // Uout = output voltage, which from adc_value is 2 * (adc_value / 4095 * 3.3)
  // Uo = 2.5
  // 1/S = 1/(0.0267) = 250 (given by datasheet)

  return (int32_t)((2 * 250 * (adc_value / 4095.0 * 3.3) - 625) * 1000);
}

int32_t Curr_CalculateCurrentSenseLow(uint32_t adc_value) {
  // Also off datasheet
  // I = (Uout - U0) * 1/S
  // Uout = output voltage, which from adc_value is 2 * (adc_value / 4095 * 3.3)
  // Uo = 2.5
  // 1/S = 1/(0.0267) = 37.45318 (given by datasheet)

  return (int32_t)((2 * 37.45318 * (adc_value / 4095.0 * 3.3) - 93.63295) * 1000);
}