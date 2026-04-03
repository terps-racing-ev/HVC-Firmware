#ifndef __SOC_H
#define __SOC_H

#include "main.h"
#include "cmsis_os2.h"

#define SOC_HIGH_CHANNEL_SWITCH_CURRENT_MA 75000
#define SOC_HIGH_CHANNEL_SWITCH_OFFSET_MA 5000
#define SOC_MA_PER_A 1000

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
 * @brief Gets the accumulated SOC delta.
 *
 * @param out_delta Output pointer for delta in A*ms.
 */
void Soc_GetDelta(int64_t *out_delta);

#endif /* __SOC_H */