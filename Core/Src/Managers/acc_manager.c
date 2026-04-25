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
#include "current_limit.h"

static osMessageQueueId_t Acc_CurrSenseQueueHandle = NULL;
uint8_t acc_initialized = 0;

static void _Acc_PackSocMessage(uint8_t *data, uint8_t *length, const SOC_Snapshot_t *snapshot);
static void _Acc_PackSummaryMessage(uint8_t *data, uint8_t *length, const Acc_Summary_t *summary);
static void _Acc_PackCurrentLimitMessage(
  uint8_t *data,
  uint8_t *length,
  const CurrLimitOutput_t *curr_limit_output
);

HAL_StatusTypeDef Acc_Manager_Init(void)
{
  uint32_t i;

  for (i = 0U; i < NUM_ACC_MODULES; i++) {
    *acc[i] = (Acc_Module_t){0};
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

  acc_initialized = 1;

  return HAL_OK;
}

void Acc_ManagerTask(void *argument)
{
  Acc_CurrSenseSample_t sample;
  uint8_t can_data[8];
  uint8_t can_len;
  Acc_Summary_t summary;
  uint8_t modules_checked;
  bool soc_estimated = false;
  SOC_Snapshot_t soc_snapshot;
  CurrLimitInput_t curr_limit_input;
  CurrLimitOutput_t curr_limit_output;

  (void)argument;

  for (;;) {
    while (osMessageQueueGet(Acc_CurrSenseQueueHandle, &sample, NULL, 0U) == osOK) {
      Soc_UpdateDeltaFromCurrSample(sample.cs_low, sample.cs_high, sample.timestamp);
    }

    HAL_StatusTypeDef summary_status = Acc_CalculateSummary(&modules_checked);

    if ((summary_status == HAL_OK) && (Acc_GetSummary(&summary) == HAL_OK)) {
      // Estimate SOC
      // TODO: another check for waiting for resting + read from flash
      if (!soc_estimated && modules_checked == NUM_ACC_MODULES) {
        SOC_UpdateStartingCapacityFromVolt(summary.volt_min_mV);
        soc_estimated = true;
      }

      SOC_GetSnapshot(&soc_snapshot);

      if (soc_estimated) {
        curr_limit_input.ocv_voltage_mV = SOC_GetOcvFromSoc(soc_snapshot.soc_pctx100);
        CurrLimit_Calculate(&curr_limit_input, &curr_limit_output);
        _Acc_PackCurrentLimitMessage(can_data, &can_len, &curr_limit_output);
        (void)BMS_CAN_SendMessage(
          CAN_ID_CURRENT_LIMIT,
          can_data,
          can_len,
          CAN_PRIORITY_NORMAL
        );
        (void)LV_CAN_SendMessage(
          CAN_ID_CURRENT_LIMIT,
          can_data,
          can_len,
          CAN_PRIORITY_NORMAL
        );
      }

      _Acc_PackSocMessage(can_data, &can_len, &soc_snapshot);
      (void)LV_CAN_SendMessage(
        CAN_ID_SOC,
        can_data,
        can_len,
        CAN_PRIORITY_NORMAL
      );


      _Acc_PackSummaryMessage(can_data, &can_len, &summary);
      (void)LV_CAN_SendMessage(
        CAN_ID_ACC_SUMMARY,
        can_data,
        can_len,
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

static void _Acc_PackSocMessage(uint8_t *data, uint8_t *length, const SOC_Snapshot_t *snapshot)
{
  uint32_t soc_delta_u;

  if ((data == NULL) || (length == NULL) || (snapshot == NULL)) {
    return;
  }

  soc_delta_u = (uint32_t)snapshot->soc_delta_As;

  data[0] = (uint8_t)(snapshot->soc_pctx100 & 0xFFU);
  data[1] = (uint8_t)((snapshot->soc_pctx100 >> 8U) & 0xFFU);
  data[2] = (uint8_t)(snapshot->soc_capacity_As & 0xFFU);
  data[3] = (uint8_t)((snapshot->soc_capacity_As >> 8U) & 0xFFU);
  data[4] = (uint8_t)(soc_delta_u & 0xFFU);
  data[5] = (uint8_t)((soc_delta_u >> 8U) & 0xFFU);
  data[6] = (uint8_t)((soc_delta_u >> 16U) & 0xFFU);
  data[7] = (uint8_t)((soc_delta_u >> 24U) & 0xFFU);
  *length = 8U;
}

static void _Acc_PackSummaryMessage(uint8_t *data, uint8_t *length, const Acc_Summary_t *summary)
{
  if ((data == NULL) || (length == NULL) || (summary == NULL)) {
    return;
  }

  // Byte layout (little-endian): v_min, v_max, t_min_dC, t_max_dC
  data[0] = (uint8_t)(summary->volt_min_mV & 0xFFU);
  data[1] = (uint8_t)((summary->volt_min_mV >> 8U) & 0xFFU);
  data[2] = (uint8_t)(summary->volt_max_mV & 0xFFU);
  data[3] = (uint8_t)((summary->volt_max_mV >> 8U) & 0xFFU);
  data[4] = (uint8_t)((uint16_t)summary->temp_min_Cx10 & 0xFFU);
  data[5] = (uint8_t)(((uint16_t)summary->temp_min_Cx10 >> 8U) & 0xFFU);
  data[6] = (uint8_t)((uint16_t)summary->temp_max_Cx10 & 0xFFU);
  data[7] = (uint8_t)(((uint16_t)summary->temp_max_Cx10 >> 8U) & 0xFFU);
  *length = 8U;
}

static void _Acc_PackCurrentLimitMessage(
  uint8_t *data,
  uint8_t *length,
  const CurrLimitOutput_t *curr_limit_output
)
{
  if ((data == NULL) || (length == NULL) || (curr_limit_output == NULL)) {
    return;
  }

  data[0] = (uint8_t)(curr_limit_output->neg_curr_limit_mA & 0xFFU);
  data[1] = (uint8_t)((curr_limit_output->neg_curr_limit_mA >> 8U) & 0xFFU);
  data[2] = (uint8_t)((curr_limit_output->neg_curr_limit_mA >> 16U) & 0xFFU);
  data[3] = (uint8_t)((curr_limit_output->neg_curr_limit_mA >> 24U) & 0xFFU);
  data[4] = (uint8_t)(curr_limit_output->pos_curr_limit_mA & 0xFFU);
  data[5] = (uint8_t)((curr_limit_output->pos_curr_limit_mA >> 8U) & 0xFFU);
  data[6] = (uint8_t)((curr_limit_output->pos_curr_limit_mA >> 16U) & 0xFFU);
  data[7] = (uint8_t)((curr_limit_output->pos_curr_limit_mA >> 24U) & 0xFFU);
  *length = 8U;
}
