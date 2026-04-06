/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : acc_manager.c
  * @brief          : Acc manager implementation
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
/* USER CODE END Header */

#include "acc_manager.h"
#include "soc.h"
#include "can.h"
#include "can_id.h"

static osMessageQueueId_t Acc_CurrSenseQueueHandle = NULL;

static void _Acc_PackSocDeltaMessage(uint8_t *data, uint8_t *length, int64_t soc_delta);
static void _Acc_PackSummaryMessage(uint8_t *data, uint8_t *length, const Acc_Summary_t *summary);

HAL_StatusTypeDef Acc_Manager_Init(void)
{
  uint32_t i;

  for (i = 0U; i < NUM_ACC_MODULES; i++) {
    *acc[i] = (Acc_Module){0};
    acc[i]->mutex = osMutexNew(NULL);
    if (acc[i]->mutex == NULL) {
      return HAL_ERROR;
    }
  }

  acc_summary = (Acc_Summary_t){0};
  acc_summary.mutex = osMutexNew(NULL);
  if (acc_summary.mutex == NULL) {
    return HAL_ERROR;
  }

  Acc_CurrSenseQueueHandle = osMessageQueueNew(
    ACC_CURR_SENSE_QUEUE_SIZE,
    sizeof(Acc_CurrSenseSample_t),
    NULL
  );
  if (Acc_CurrSenseQueueHandle == NULL) {
    return HAL_ERROR;
  }

  if (Soc_Init() != HAL_OK) {
    return HAL_ERROR;
  }

  return HAL_OK;
}

void Acc_ManagerTask(void *argument)
{
  Acc_CurrSenseSample_t sample;
  int64_t soc_delta;
  uint8_t soc_data[8];
  uint8_t soc_len;
  Acc_Summary_t summary;
  uint8_t summary_data[8];
  uint8_t summary_len;

  (void)argument;

  for (;;) {
    while (osMessageQueueGet(Acc_CurrSenseQueueHandle, &sample, NULL, 0U) == osOK) {
      Soc_UpdateDeltaFromCurrSample(sample.cs_low, sample.cs_high, sample.timestamp);
    }

    // Soc_GetDelta(&soc_delta);
    // _Acc_PackSocDeltaMessage(soc_data, &soc_len, soc_delta);
    // (void)LV_CAN_SendMessage(
    //   CAN_ID_SOC,
    //   soc_data,
    //   soc_len,
    //   CAN_PRIORITY_NORMAL
    // );

    if ((Acc_CalculateSummary() == HAL_OK) && (Acc_GetSummary(&summary) == HAL_OK)) {
      _Acc_PackSummaryMessage(summary_data, &summary_len, &summary);
      (void)LV_CAN_SendMessage(
        CAN_ID_ACC_SUMMARY,
        summary_data,
        summary_len,
        CAN_PRIORITY_NORMAL
      );
    }
    


    osDelay(ACC_UPDATE_FREQ_MS);
  }
}

HAL_StatusTypeDef Acc_CurrSenseQueue_Push(
  int32_t cs_low,
  int32_t cs_high,
  uint32_t timestamp,
  uint32_t timeout_ms
)
{
  Acc_CurrSenseSample_t sample;
  osStatus_t status;

  if (Acc_CurrSenseQueueHandle == NULL) {
    return HAL_ERROR;
  }

  sample.cs_low = cs_low;
  sample.cs_high = cs_high;
  sample.timestamp = timestamp;

  status = osMessageQueuePut(Acc_CurrSenseQueueHandle, &sample, 0U, timeout_ms);
  if (status == osOK) {
    return HAL_OK;
  }
  if (status == osErrorTimeout) {
    return HAL_TIMEOUT;
  }
  if (status == osErrorResource) {
    return HAL_BUSY;
  }

  return HAL_ERROR;
}

static void _Acc_PackSocDeltaMessage(uint8_t *data, uint8_t *length, int64_t soc_delta)
{
  uint64_t soc_u;

  if ((data == NULL) || (length == NULL)) {
    return;
  }

  soc_u = (uint64_t)soc_delta;

  data[0] = (uint8_t)(soc_u & 0xFFU);
  data[1] = (uint8_t)((soc_u >> 8U) & 0xFFU);
  data[2] = (uint8_t)((soc_u >> 16U) & 0xFFU);
  data[3] = (uint8_t)((soc_u >> 24U) & 0xFFU);
  data[4] = (uint8_t)((soc_u >> 32U) & 0xFFU);
  data[5] = (uint8_t)((soc_u >> 40U) & 0xFFU);
  data[6] = (uint8_t)((soc_u >> 48U) & 0xFFU);
  data[7] = (uint8_t)((soc_u >> 56U) & 0xFFU);
  *length = 8U;
}

static void _Acc_PackSummaryMessage(uint8_t *data, uint8_t *length, const Acc_Summary_t *summary)
{
  int16_t temp_min_deci_c;
  int16_t temp_max_deci_c;
  float scaled;

  if ((data == NULL) || (length == NULL) || (summary == NULL)) {
    return;
  }

  scaled = summary->temp_min * 10.0f;
  if (scaled > 32767.0f) {
    temp_min_deci_c = 32767;
  } else if (scaled < -32768.0f) {
    temp_min_deci_c = -32768;
  } else {
    temp_min_deci_c = (scaled >= 0.0f) ? (int16_t)(scaled + 0.5f) : (int16_t)(scaled - 0.5f);
  }

  scaled = summary->temp_max * 10.0f;
  if (scaled > 32767.0f) {
    temp_max_deci_c = 32767;
  } else if (scaled < -32768.0f) {
    temp_max_deci_c = -32768;
  } else {
    temp_max_deci_c = (scaled >= 0.0f) ? (int16_t)(scaled + 0.5f) : (int16_t)(scaled - 0.5f);
  }

  // Byte layout (little-endian): v_min, v_max, t_min_dC, t_max_dC
  data[0] = (uint8_t)(summary->volt_min & 0xFFU);
  data[1] = (uint8_t)((summary->volt_min >> 8U) & 0xFFU);
  data[2] = (uint8_t)(summary->volt_max & 0xFFU);
  data[3] = (uint8_t)((summary->volt_max >> 8U) & 0xFFU);
  data[4] = (uint8_t)((uint16_t)temp_min_deci_c & 0xFFU);
  data[5] = (uint8_t)(((uint16_t)temp_min_deci_c >> 8U) & 0xFFU);
  data[6] = (uint8_t)((uint16_t)temp_max_deci_c & 0xFFU);
  data[7] = (uint8_t)(((uint16_t)temp_max_deci_c >> 8U) & 0xFFU);
  *length = 8U;
}
