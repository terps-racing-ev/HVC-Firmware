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
bool DecodeCellTempSummary(const CAN_Message_t *in, BMS_Message_t *out)
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
    // Next 16 bits = avg_temp (deci-degC)
    int16_t avg_raw = (int16_t)((uint16_t)in->data[4] | ((uint16_t)in->data[5] << 8));

    uint8_t min_cell_id = in->data[6];
    uint8_t max_cell_id = in->data[7];

    out->cell_temps.temp_min_Cx10 = min_raw;
    out->cell_temps.temp_max_Cx10 = max_raw;
    out->cell_temps.temp_avg_Cx10 = avg_raw;
    out->cell_temps.temp_min_cell_id = min_cell_id;
    out->cell_temps.temp_max_cell_id = max_cell_id;

    return true;
}

/**
 * @brief Decodes ambient temps from last cell temp message.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module and message info.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeAmbientTemps(const CAN_Message_t *in, BMS_Message_t *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 8U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_TEMP + AMBIENT_TEMP_MSG_INDEX) return false;

    out->module = MODULE_ID(in->id);
   
    // Ambient temp 1 in bytes 4-5, ambient temp 2 in bytes 6-7 (Cx10)
    int16_t temp_1_raw = (int16_t)((uint16_t)in->data[4] | ((uint16_t)in->data[5] << 8));
    int16_t temp_2_raw = (int16_t)((uint16_t)in->data[6] | ((uint16_t)in->data[7] << 8));

    out->amb_temps.amb_temp_1_Cx10 = temp_1_raw;
    out->amb_temps.amb_temp_2_Cx10 = temp_2_raw;

    return true;
}

/**
 * @brief Decodes heartbeat message.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module info and message timestamp.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeBMSHeartbeat(const CAN_Message_t *in, BMS_Message_t *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 8U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_BMS_HEARTBEAT) return false;

    out->module = MODULE_ID(in->id);
   
    out->heartbeat.heartbeat_timestamp = in->timestamp;
    // 5th byte = 0xFF if any errors
    out->heartbeat.errored = (bool)(in->data[5]);

    return true;
}

/**
 * @brief Decodes voltage summary message
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded voltage summary info and message timestamp.
 * @retval bool: Returns trye if message is meant for this decoder, false if not.
 */
bool DecodeVoltageSummary(const CAN_Message_t *in, BMS_Message_t *out) {
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 8U) return false;

    cmd = REMOVE_MODULE_ID(in->id);
    if ((cmd != BMB_CAN_BMS1_VOLTAGE_SUMMARY) && (cmd != BMB_CAN_BMS2_VOLTAGE_SUMMARY)) {
        return false;
    }

    out->module = MODULE_ID(in->id);
    out->is_bms1 = (cmd == BMB_CAN_BMS1_VOLTAGE_SUMMARY);

    // Byte layout (little-endian): avg_mV, min_mV, max_mV, min_id, max_id
    out->cell_voltages.volt_avg_mV = (uint16_t)in->data[0] | ((uint16_t)in->data[1] << 8);
    out->cell_voltages.volt_min_mV = (uint16_t)in->data[2] | ((uint16_t)in->data[3] << 8);
    out->cell_voltages.volt_max_mV = (uint16_t)in->data[4] | ((uint16_t)in->data[5] << 8);
    out->cell_voltages.volt_min_cell_id = in->data[6];
    out->cell_voltages.volt_max_cell_id = in->data[7];

    return true;
}

/**
 * @brief Updates corresponding ACC module with BMS1/BMS2 voltage summary.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleVoltageSummary(const BMS_Message_t *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;

    Acc_SetCellVoltages(acc[msg->module], &msg->cell_voltages, msg->is_bms1);

    return true;
}

/**
 * @brief Updates corresponding ACC module with cell temp info.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleCellTempSummary(const BMS_Message_t *msg){
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
bool HandleAmbientTemps(const BMS_Message_t *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetAmbientTemps(acc[msg->module], &msg->amb_temps);

    return true;
}

/**
 * @brief Updates corresponding ACC module with heartbeat timestamp.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleBMSHeartbeat(const BMS_Message_t *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetHeartbeat(
        acc[msg->module], 
        &msg->heartbeat
    );

    return true;
}

