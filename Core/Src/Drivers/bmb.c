/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bmb.c
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

  #include "bmb.h"

bool DecodeCellTempSummary(const CAN_Message_t *in, BMS_Message *out)
{
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 4U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_TEMP_SUMMARY) return false;

    out->module = MODULE_ID(in->id);
   
    // First 16 bits = min_temp (deci-degC), next 16 bits = max_temp (deci-degC)
    int16_t min_raw = (int16_t)((uint16_t)in->data[0] | ((uint16_t)in->data[1] << 8));
    int16_t max_raw = (int16_t)((uint16_t)in->data[2] | ((uint16_t)in->data[3] << 8));

    out->cell_temps.temp_min = ((float)min_raw) / 10.0f; // 636 -> 63.6 C
    out->cell_temps.temp_max = ((float)max_raw) / 10.0f; // 636 -> 63.6 C

    return true;
}

bool HandleCellTempSummary(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetCellTemps(acc[msg->module], &msg->cell_temps);

    return true;
}
