#include "soc.h"

static int64_t soc_delta = 0;
static int32_t soc_delta_remainder_mAms = 0;
static uint32_t soc_last_timestamp = 0U;
static uint8_t soc_has_timestamp = 0U;
static osMutexId_t soc_mutex = NULL;

HAL_StatusTypeDef Soc_Init(void)
{
    soc_delta = 0;
    soc_delta_remainder_mAms = 0;
    soc_last_timestamp = 0U;
    soc_has_timestamp = 0U;

    if (soc_mutex == NULL) {
        soc_mutex = osMutexNew(NULL);
        if (soc_mutex == NULL) {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

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

    if (soc_has_timestamp == 0U) {
        soc_last_timestamp = timestamp;
        soc_has_timestamp = 1U;
        osMutexRelease(soc_mutex);
        return;
    }

    abs_low = (cs_low < 0) ? -cs_low : cs_low;
    threshold = SOC_HIGH_CHANNEL_SWITCH_CURRENT_MA - SOC_HIGH_CHANNEL_SWITCH_OFFSET_MA;
    selected_current = (abs_low >= threshold) ? cs_high : cs_low;

    dt_ms = timestamp - soc_last_timestamp;
    delta_mAms = ((int64_t)selected_current * (int64_t)dt_ms) + soc_delta_remainder_mAms;
    soc_delta += (delta_mAms / SOC_MA_PER_A);
    soc_delta_remainder_mAms = (int32_t)(delta_mAms % SOC_MA_PER_A);
    soc_last_timestamp = timestamp;

    osMutexRelease(soc_mutex);
}

void Soc_GetDelta(int64_t *out_delta)
{
    if ((soc_mutex == NULL) || (out_delta == NULL)) {
        return;
    }

    osMutexAcquire(soc_mutex, osWaitForever);
    *out_delta = soc_delta;
    osMutexRelease(soc_mutex);
}