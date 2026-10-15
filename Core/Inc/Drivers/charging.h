/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : charging.h
  * @brief          : Header for charging request CAN handlers
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __CHARGING_H
#define __CHARGING_H

#include "can.h"
#include <stdbool.h>

/**
 * @brief Decodes charging request message from BMS bus.
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded message (UNUSED).
 * @retval bool: True if message matches charging request ID.
 */
bool DecodeChargingHeartbeatBMS(const CAN_Message_t *in, BMS_Message_t *out);

/**
 * @brief Handles charging request by raising charging event flag.
 * @param msg: Input pointer to decoded message (UNUSED).
 * @retval bool: True if event flag set succeeded.
 */
bool HandleChargingHeartbeatBMS(const BMS_Message_t *msg);

#endif /* __CHARGING_H */