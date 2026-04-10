/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bmb.h
  * @brief          : Header for bmb message decoder
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

#include "can_id.h"
#include "can.h"
#include <stdint.h>
#include <stdbool.h>
#include "acc.h"

/* Defines  -------------------------------------------------------------------*/

/* Module ID bit shift (bits 12-15) */
#define MODULE_ID_MASK      0x0000F000
#define MODULE_ID(base) ((uint8_t)(((uint32_t)(base) & MODULE_ID_MASK) >> 12))

#define MSG_CMD_MASK        0xFFFF0FFF
#define REMOVE_MODULE_ID(base) (base & MSG_CMD_MASK)

#define BMB_CAN_VOLTAGE_BASE 0x08F00200
#define CELLS_PER_VOLTAGE_MSG 3
#define VOLTAGE_MSG_INDEX_MASK 0x0000000F
#define VOLTAGE_MSG_INDEX(base) (base & VOLTAGE_MSG_INDEX_MASK)
#define VOLTAGE_BASE_MASK 0xFFFFFFF0
#define REMOVE_VOLTAGE_MSG_INDEX(base) (base & VOLTAGE_BASE_MASK) 

#define AMBIENT_TEMP_MSG_INDEX 13

/* Decoders --------------------------------------------------------*/
/**
 * @brief Decodes min and max temps from cell temp summary.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module and message info.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeCellTempSummary(const CAN_Message_t *in, BMS_Message_t *out);

/**
 * @brief Decodes ambient temps from last cell temp message.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module and message info.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeAmbientTemps(const CAN_Message_t *in, BMS_Message_t *out);

/**
 * @brief Decodes cell voltage messages.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module info and updated min/max data.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeCellVoltages(const CAN_Message_t *in, BMS_Message_t *out);

/**
 * @brief Decodes heartbeat message.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded module info and message timestamp.
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeBMSHeartbeat(const CAN_Message_t *in, BMS_Message_t *out);

/* Handlers --------------------------------------------------------*/
/**
 * @brief Updates corresponding ACC module with cell temp info.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleCellTempSummary(const BMS_Message_t *msg);

/**
 * @brief Updates corresponding ACC module with ambient temp info.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleAmbientTemps(const BMS_Message_t *msg);

/**
 * @brief Updates corresponding ACC module with cell voltage info.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleCellVoltages(const BMS_Message_t *msg);

/**
 * @brief Updates corresponding ACC module with heartbeat timestamp.
 * @param msg: Input pointer to decoded module and message info.
 * @retval bool
 */
bool HandleBMSHeartbeat(const BMS_Message_t *msg);