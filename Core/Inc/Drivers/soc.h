/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : soc.h
  * @brief          : Header for soc handler
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
#ifndef __SOC_H
#define __SOC_H

#include "acc.h"

#define DEFAULT_ELAPSED_TICKS (pdMS_TO_TICKS(1)) // 1 ms in ticks
#define CS_MAX_LOW_CHANNEL_READING_MV 75000
#define CS_CHANNEL_SWITCHING_BUFFER_MV 5000 // How soon before max reading we switch to high current

#define TICKS_TO_MS(ticks) ((ticks * 1000U) / configTICK_RATE_HZ)

// TODO: See if we can use uint16
typedef uint32_t voltage_mv_t;
typedef uint32_t capacity_pct_t;

typedef enum {
    SOC_PRE_ESTIMATE,
    SOC_PRE_RESTING_LOOKUP,
    SOC_SET
} SOC_State;

typedef struct {
    voltage_mv_t voltage_mv;
    capacity_pct_t capacity_pct;  // Percent scaled by 100
} OCV_Voltage_Lookup;

// Defined in soc.c
extern OCV_Voltage_Lookup ocv_lookup_table[];
extern size_t ocv_lookup_table_size;

extern uint16_t soc_start_pct; // Start SOC value
extern uint32_t soc_start_a_ms; // Start SOC converted to A*ms
extern uint16_t soc_pct;   // SOC as % of total possible
extern int32_t delta_capacity_a_ms; // A * ms, Total acc capacity that's entered since init in, - means out

void SOC_Init(void);

void SOC_UpdateDeltaCapacity(Curr_Sense_Reading_t *cs_reading);

capacity_pct_t SOC_CalculateCapacityPct(uint32_t cell_voltage_mV);

/**
 * @brief Convert percent capacity to A * ms. Blocks while waiting for flash mutex
 * @param capacity_pct: Percent capacity * 100 (100% would be 10000)
 * @return Capacity in A*ms
 */
uint32_t SOC_CapacityPctToAms(capacity_pct_t capacity_pct);

#endif // __SOC_H