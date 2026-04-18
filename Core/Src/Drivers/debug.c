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

#include "debug.h"

/**
 * @brief Decodes reset message
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded msg (UNUSED)
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeResetBMS(const CAN_Message_t *in, BMS_Message_t *out) {
    return in->id == CAN_ID_RESET;
}

/**
 * @brief Reset HVC
 * @param msg: Input pointer to decoded msg (UNUSED)
 * @retval bool
 */
bool HandleResetBMS(const BMS_Message_t *msg) {
  NVIC_SystemReset();
  return true;  // Never reached
}

/**
 * @brief Decodes reset message
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded msg (UNUSED)
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeResetLV(const CAN_Message_t *in, LV_Message_t *out) {
  return in->id == CAN_ID_RESET;
}

/**
 * @brief Reset HVC
 * @param msg: Input pointer to decoded msg (UNUSED)
 * @retval bool
 */
bool HandleResetLV(const LV_Message_t *msg) {
  NVIC_SystemReset();
  return true;  // Never reached
}