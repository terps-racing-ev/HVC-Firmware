#ifndef __SOC_H
#define __SOC_H

#include "main.h"

#define SOC_HIGH_CHANNEL_SWITCH_CURRENT_MA 75000
#define SOC_HIGH_CHANNEL_SWITCH_OFFSET_MA 20000
#define SOC_MA_PER_A 1000
#define SOC_MS_PER_S 1000
#define SOC_MA_MS_PER_A_S (SOC_MA_PER_A * SOC_MS_PER_S)
#define SOC_MAX_CAPACITY_A_S 54000 // TODO: use flash to store dynamic, pls

#define IGNORED_CURR_CUTOFF_MA 200 // Minimum current value that isn't ignored in delta calc

typedef struct {
    uint16_t soc_pctx100;
    uint16_t soc_capacity_As;
    int32_t soc_delta_As;
} SOC_Snapshot_t;

/**
 * @brief Initializes SOC module state.
 *
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef Soc_Init(void);

/**
 * @brief Updates SOC delta from one current-sense sample.
 *
 * Uses the high channel when |low current| is at or above
 * (SOC_HIGH_CHANNEL_SWITCH_CURRENT_MA - SOC_HIGH_CHANNEL_SWITCH_OFFSET_MA),
 * otherwise uses the low channel.
 *
 * @param cs_low Low channel current in mA.
 * @param cs_high High channel current in mA.
 * @param timestamp Tick timestamp in ms.
 */
void Soc_UpdateDeltaFromCurrSample(int32_t cs_low, int32_t cs_high, uint32_t timestamp);

/**
 * @brief Converts pack voltage to SOC using the OCV lookup table.
 *
 * The lookup table is expected to be sorted by descending voltage. This
 * function returns the SOC of the first table entry whose voltage is less than
 * or equal to voltage_mv.
 *
 * @param voltage_mv Input voltage in mV.
 */
void SOC_UpdateStartingCapacityFromVolt(uint32_t voltage_mv);

/**
 * @brief Reverse lookup from SOC to estimated OCV using the OCV table.
 *
 * @param soc_pctx100 Input SOC in percent scaled by 100.
 *
 * @return Estimated OCV voltage in mV.
 */
uint16_t SOC_GetOcvFromSoc(uint16_t soc_pctx100);

/**
 * @brief Gets a thread-safe snapshot of current SOC tracking values.
 *
 * @param snapshot Output pointer for SOC snapshot values.
 */
void SOC_GetSnapshot(SOC_Snapshot_t* snapshot);

/**
 * @brief Resets integrated SOC delta and restores snapshot from starting capacity.
 */
void SOC_Reset(void);

#endif /* __SOC_H */
