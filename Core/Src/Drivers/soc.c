/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : soc.c
  * @brief          : Implementation for soc handler
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

#include "soc.h"
#include "flash.h"

static uint32_t last_processed_timestamp = 0;

#define SOC_WAIT_FOR_OCV_RESTING 1 // Implement when RTC is alive

// TODO: fill in
OCV_Voltage_Lookup ocv_lookup_table[] = {
    {4200, 10000},
    {3200, 0}
};
size_t ocv_lookup_table_size = sizeof(ocv_lookup_table) / sizeof(OCV_Voltage_Lookup);    // Computed at compile time

/**
 * @brief Initializes charged/discharged capacity
 */
void SOC_Init(void) {
    delta_capacity_a_ms = 0; // Initialize capacity used/gained to 0
}

void SOC_UpdateDeltaCapacity(Curr_Sense_Reading_t *cs_reading) {
    // 1. Calculate how much capacity changed by
    uint32_t elapsed_ticks = (last_processed_timestamp == 0)
        ? DEFAULT_ELAPSED_TICKS // On first reading use default elapsed time
        : (cs_reading->timestamp_ticks - last_processed_timestamp);
    
    last_processed_timestamp = cs_reading->timestamp_ticks;

    uint32_t elapsed_ms = TICKS_TO_MS(elapsed_ticks);

    if (
        // Ugly way to check if we're in bounds for reading from low current channel of current sensor
        // Low current channel has better accuracy so try to use as much as possible
        (cs_reading->cs_low_val_mA <= CS_MAX_LOW_CHANNEL_READING_MV - CS_CHANNEL_SWITCHING_BUFFER_MV && cs_reading->cs_low_val_mA >= 0)
        || (cs_reading->cs_low_val_mA >= CS_CHANNEL_SWITCHING_BUFFER_MV - CS_MAX_LOW_CHANNEL_READING_MV  && cs_reading->cs_low_val_mA < 0)
    ) {
        delta_capacity_a_ms -= (cs_reading->cs_low_val_mA * elapsed_ms)/1000;
    } else {
        // Read from high channel after passing threshold
        delta_capacity_a_ms -= (cs_reading->cs_high_val_mA * elapsed_ms)/1000;
    }
}

capacity_pct_t SOC_CalculateCapacityPct(uint32_t cell_voltage_mV) {
    // Rounds down to nearest defined OCV capacity %
    for (int i = 0; i < ocv_lookup_table_size; i++) {
        if (cell_voltage_mV >= ocv_lookup_table[i].voltage_mv) {
            return ocv_lookup_table[i].capacity_pct;
        }
    }
}

/**
 * @brief Convert percent capacity to A * ms. Blocks while waiting for flash mutex
 */
uint32_t SOC_CapacityPctToAms(capacity_pct_t capacity_pct) {
    Flash_SOC_Data_t flash_data;

    if (Flash_ReadSOCData(&flash_data) != HAL_OK) {
        return 0;
    }

    // TODO: cap total_capacity at a reasonable value so we don't overflow ever

    // Cast to uint64 so no overflow :D
    return (uint32_t)(((uint64_t)flash_data.soc_total_capacity * capacity_pct) / 100);
}