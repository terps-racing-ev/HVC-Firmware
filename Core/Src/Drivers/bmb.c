/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bmb.c
  * @brief          : BMB message decoder
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

/**
 * @brief Decodes min and max temps from cell temp summary.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module and message info.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
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

/**
 * @brief Decodes ambient temps from last cell temp message.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module and message info.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeAmbientTemps(const CAN_Message_t *in, BMS_Message *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 8U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_TEMP + AMBIENT_TEMP_MSG_INDEX) return false;

    out->module = MODULE_ID(in->id);
   
    // Ambient temp 1 in bytes 4-5, ambient temp 2 in bytes 6-7 (0.1C)
    int16_t temp_1_raw = (int16_t)((uint16_t)in->data[4] | ((uint16_t)in->data[5] << 8));
    int16_t temp_2_raw = (int16_t)((uint16_t)in->data[6] | ((uint16_t)in->data[7] << 8));

    out->amb_temps.amb_temp_1 = ((float)temp_1_raw) / 10.0f;
    out->amb_temps.amb_temp_2 = ((float)temp_2_raw) / 10.0f;

    return true;
}

/**
 * @brief Decodes cell voltage messages.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module info and updated min/max data.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeCellVoltages(const CAN_Message_t *in, BMS_Message *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 6U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (REMOVE_VOLTAGE_MSG_INDEX(cmd) != BMB_CAN_VOLTAGE_BASE) return false;

    out->module = MODULE_ID(in->id);
   
    Acc_GetCellVoltages(acc[out->module], &out->cell_voltages);

    uint8_t cell_id[CELLS_PER_VOLTAGE_MSG];
    float volt[CELLS_PER_VOLTAGE_MSG];
    
    for (int i = 0; i < CELLS_PER_VOLTAGE_MSG; i++) {
        // cells 1-3 in message 0, 4-6 in message 1, 7-9 in message 2, and so on
        cell_id[i] = 3 * VOLTAGE_MSG_INDEX(cmd) + i + 1;
        // first voltage in bytes 0-1, second voltage in bytes 2-3, third voltage in bytes 4-5 (mV)
        volt[i] = (float)((uint16_t)in->data[2 * i] | ((uint16_t)in->data[2 * i + 1] << 8)) / 1000.0f;
        
        if ((out->cell_voltages.volt_min == 0) || (volt[i] < out->cell_voltages.volt_min)) {
            out->cell_voltages.volt_min_cell_id = cell_id[i];
            out->cell_voltages.volt_min = volt[i];
        } else if (volt[i] > out->cell_voltages.volt_max) {
            out->cell_voltages.volt_max_cell_id = cell_id[i];
            out->cell_voltages.volt_max = volt[i];
        }
    }

    return true;
}

/**
 * @brief Decodes heartbeat message.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module info and message timestamp.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeBMSHeartbeat(const CAN_Message_t *in, BMS_Message *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 8U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_BMS_HEARTBEAT) return false;

    out->module = MODULE_ID(in->id);
   
    out->heartbeat_timestamp = in->timestamp;

    return true;
}

/**
 * @brief Updates corresponding ACC module with cell temp info.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleCellTempSummary(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetCellTemps(acc[msg->module], &msg->cell_temps);

    return true;
}

/**
 * @brief Updates corresponding ACC module with ambient temp info.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleAmbientTemps(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetAmbientTemps(acc[msg->module], &msg->amb_temps);

    return true;
}

/**
 * @brief Updates corresponding ACC module with cell voltage info.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleCellVoltages(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetCellVoltages(acc[msg->module], &msg->cell_voltages);

    return true;

}

/**
 * @brief Updates corresponding ACC module with heartbeat timestamp.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleBMSHeartbeat(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetHeartbeatLastUpdate(acc[msg->module], &msg->heartbeat_timestamp);

    return true;
}