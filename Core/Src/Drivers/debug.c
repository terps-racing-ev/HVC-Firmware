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

#define BMB_PASSTHROUGH_ID_PREFIX 0x08F00000u
#define BMB_PASSTHROUGH_ID_MASK   0xFFF00000u

static bool IsBMBPassthroughId(uint32_t id)
{
  return (id & BMB_PASSTHROUGH_ID_MASK) == BMB_PASSTHROUGH_ID_PREFIX;
}

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
bool DecodeResetLV(const uCAN_MSG *in) {
  return in->frame.id == CAN_ID_RESET;
}

/**
 * @brief Reset HVC
 * @param msg: Input pointer to decoded msg (UNUSED)
 * @retval bool
 */
bool HandleResetLV(const uCAN_MSG *msg) {
  NVIC_SystemReset();
  return true;  // Never reached
}

bool DecodeBMBPassthroughLV(const uCAN_MSG *msg) {
  if (msg == NULL) {
    return false;
  }

  return IsBMBPassthroughId(msg->frame.id);
}

bool HandleBMBPassthroughLV(const uCAN_MSG *msg) {
  uint8_t data[8];

  if ((msg == NULL) || !IsBMBPassthroughId(msg->frame.id)) {
    return false;
  }

  data[0] = msg->frame.data0;
  data[1] = msg->frame.data1;
  data[2] = msg->frame.data2;
  data[3] = msg->frame.data3;
  data[4] = msg->frame.data4;
  data[5] = msg->frame.data5;
  data[6] = msg->frame.data6;
  data[7] = msg->frame.data7;

  return BMS_CAN_SendMessageWithTimeout(
    msg->frame.id,
    data,
    msg->frame.dlc,
    CAN_PRIORITY_CRITICAL,
    0
  ) == HAL_OK;
}