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

typedef struct {
    uint8_t module;
    union {
        CellTemps cell_temps;
        CellVoltages cell_voltages;
        uint8_t heartbeat;
    };
} BMS_Message;

bool DecodeCellTempSummary(const CAN_Message_t *in, BMS_Message *out);
bool HandleCellTempSummary(const BMS_Message *msg);