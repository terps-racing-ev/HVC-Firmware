#include "soc.h"
#include "ocv_lookup_table.h"
#include "cmsis_os2.h"
#include <string.h>
#include "stdbool.h"

static int32_t soc_delta_remainder_mAms = 0;
static uint32_t soc_last_timestamp = 0U;
static uint8_t soc_has_timestamp = 0U;
static osMutexId_t soc_mutex = NULL;
static SOC_Snapshot_t soc_snapshot = {0};
static uint16_t starting_capacity = 0;
static bool starting_capacity_set = false;

/**
 * @brief Initialize SOC runtime state and mutex.
 */
HAL_StatusTypeDef Soc_Init(void)
{
    soc_delta_remainder_mAms = 0;
    soc_last_timestamp = 0U;
    soc_has_timestamp = 0U;
    starting_capacity = 0;
    starting_capacity_set = false;

    if (soc_mutex == NULL) {
        soc_mutex = osMutexNew(NULL);
        if (soc_mutex == NULL) {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

/**
 * @brief Integrate current readings into SOC delta (A*s).
 */
void Soc_UpdateDeltaFromCurrSample(int32_t cs_low, int32_t cs_high, uint32_t timestamp)
{
    int32_t abs_low;
    int32_t threshold;
    int32_t selected_current;
    uint32_t dt_ms;
    int64_t delta_mAms;

    if (soc_mutex == NULL) {
        return;
    }

    osMutexAcquire(soc_mutex, osWaitForever);

    // Skip calculation for first reading
    if (soc_has_timestamp == 0U) {
        soc_last_timestamp = timestamp;
        soc_has_timestamp = 1U;
        osMutexRelease(soc_mutex);
        return;
    }

    // Choose sensor channel based on current level
    abs_low = (cs_low < 0) ? -cs_low : cs_low;
    threshold = SOC_HIGH_CHANNEL_SWITCH_CURRENT_MA - SOC_HIGH_CHANNEL_SWITCH_OFFSET_MA;
    selected_current = (abs_low >= threshold) ? cs_high : cs_low;

    if (selected_current < IGNORED_CURR_CUTOFF_MA && selected_current > -IGNORED_CURR_CUTOFF_MA) {
        selected_current = 0;
    }

    dt_ms = timestamp - soc_last_timestamp;
    delta_mAms = ((int64_t)selected_current * (int64_t)dt_ms) + soc_delta_remainder_mAms;
    soc_snapshot.soc_delta_As += (delta_mAms / SOC_MA_MS_PER_A_S);
    soc_delta_remainder_mAms = (int32_t)(delta_mAms % SOC_MA_MS_PER_A_S);
    soc_last_timestamp = timestamp;

    // Has been updated
    if (starting_capacity_set) {
        // Calculate pct and capacity
        soc_snapshot.soc_capacity_As = starting_capacity - soc_snapshot.soc_delta_As;
        soc_snapshot.soc_pctx100 = (uint16_t)(((float)soc_snapshot.soc_capacity_As / (float)SOC_MAX_CAPACITY_A_S) * 10000);
    }


    osMutexRelease(soc_mutex);
}

/**
 * @brief Set starting capacity from OCV-derived SOC at the given voltage.
 */
void SOC_UpdateStartingCapacityFromVolt(uint32_t voltage_mv)
{
    size_t i;
    uint16_t starting_capacity_local;
    capacity_pct_t capacity_pctx100;

    if (OCV_Lookup_Table_Size == 0U) {
        return;
    }

    // Clamp outside the table range.
    if (voltage_mv >= OCV_Lookup_Table[0U].voltage_mv) {
        capacity_pctx100 = OCV_Lookup_Table[0U].capacity_pctx100;
    } else if (voltage_mv <= OCV_Lookup_Table[OCV_Lookup_Table_Size - 1U].voltage_mv) {
        capacity_pctx100 = OCV_Lookup_Table[OCV_Lookup_Table_Size - 1U].capacity_pctx100;
    } else {
        capacity_pctx100 = OCV_Lookup_Table[OCV_Lookup_Table_Size - 1U].capacity_pctx100;

        for (i = 1U; i < OCV_Lookup_Table_Size; i++) {
            uint32_t v_high = OCV_Lookup_Table[i - 1U].voltage_mv;
            uint32_t v_low = OCV_Lookup_Table[i].voltage_mv;

            if ((voltage_mv <= v_high) && (voltage_mv >= v_low)) {
                uint32_t c_high = OCV_Lookup_Table[i - 1U].capacity_pctx100;
                uint32_t c_low = OCV_Lookup_Table[i].capacity_pctx100;

                if (v_high == v_low) {
                    capacity_pctx100 = (capacity_pct_t)c_low;
                } else {
                    uint32_t dv = v_high - v_low;
                    uint32_t dv_meas = voltage_mv - v_low;
                    uint32_t interp = c_low + (((c_high - c_low) * dv_meas) / dv);
                    capacity_pctx100 = (capacity_pct_t)interp;
                }
                break;
            }
        }
    }

    starting_capacity_local = (uint16_t)(((uint32_t)capacity_pctx100 * SOC_MAX_CAPACITY_A_S) / 10000U);

    if (soc_mutex == NULL) {
        return;
    }
    osMutexAcquire(soc_mutex, osWaitForever);
    starting_capacity = starting_capacity_local;
    starting_capacity_set = true;
    osMutexRelease(soc_mutex);
}

/**
 * @brief Copy current SOC snapshot in a thread-safe way.
 */
void SOC_GetSnapshot(SOC_Snapshot_t* snapshot) {
    if ((snapshot == NULL) || (soc_mutex == NULL)) return;

    osMutexAcquire(soc_mutex, osWaitForever);
    memcpy(snapshot, &soc_snapshot, sizeof(SOC_Snapshot_t));
    osMutexRelease(soc_mutex);
}

/**
 * @brief Reset integrated delta and restore snapshot from starting capacity.
 */
void SOC_Reset(void) {
    if (soc_mutex == NULL) return;

    osMutexAcquire(soc_mutex, osWaitForever);
    soc_snapshot.soc_delta_As = 0;
    soc_snapshot.soc_pctx100 = (uint16_t)(((uint32_t)starting_capacity * 10000U) / SOC_MAX_CAPACITY_A_S);
    soc_snapshot.soc_capacity_As = starting_capacity;
    osMutexRelease(soc_mutex);
}
