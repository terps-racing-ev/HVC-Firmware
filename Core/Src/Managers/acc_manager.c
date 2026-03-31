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
#include "can.h"
#include "managers_config.h"

Acc_Module_t module_0;
Acc_Module_t module_1;
Acc_Module_t module_2;
Acc_Module_t module_3;
Acc_Module_t module_4;
Acc_Module_t module_5;

Acc_Module_t *acc[6] = {
    &module_0,
    &module_1,
    &module_2,
    &module_3,
    &module_4,
    &module_5
};
// For initializing mutexes
static const char *acc_mutex_names[NUM_ACC_MODULES] = {
  "module_0_mutex",
  "module_1_mutex",
  "module_2_mutex",
  "module_3_mutex",
  "module_4_mutex",
  "module_5_mutex"
};

/* Private Defines ----------------------------------*/
static osMessageQueueId_t Acc_SOC_CurrSenseQueueHandle = NULL;

/**
  * @brief  Initialize acc manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef Acc_Manager_Init(void) {
  for (int i = 0; i < NUM_ACC_MODULES; i++) {
    if (Acc_InitModule(acc[i], acc_mutex_names[i]) != HAL_OK) {
      return HAL_ERROR;
    }
  }

  // Create soc curr sense queue
  Acc_SOC_CurrSenseQueueHandle = osMessageQueueNew(
    SOC_CS_QUEUE_SIZE,
    sizeof(Curr_Sense_Reading_t),
    NULL
  );

  return HAL_OK;
}

/**
  * @brief  Main acc manager task
  * @param  argument: Not used
  * @retval None
  */
void Acc_ManagerTask(void *argument) {
  Curr_Sense_Reading_t r;
  Cell_Voltages_t cv;
  uint8_t num_modules_updated;
  uint16_t min_cell_voltage;
  SOC_State soc_state = SOC_PRE_ESTIMATE;
  capacity_pct_t capacity;

  for (;;) {

#ifdef ACC_MANAGER_ENABLED
    // Handle curr sense readings in batches
    while (osMessageQueueGet(Acc_SOC_CurrSenseQueueHandle, &r, NULL, 0)) {
      SOC_UpdateDeltaCapacity(&r);
    }

    switch (soc_state) {
      case SOC_PRE_ESTIMATE:  // Estimate start SOC by using lookup at the start

        // Wait for starting min voltage
        if (num_modules_updated != NUM_ACC_MODULES) {
            num_modules_updated = _Acc_GetMinCellVoltage(&min_cell_voltage);
        } else {
          // Use starting min voltage to calculate capacity
          soc_start_pct = SOC_CalculateCapacityPct(min_cell_voltage);
          soc_start_a_ms = SOC_CapacityPctToAms(soc_start_pct);
          
          soc_state = SOC_PRE_RESTING_LOOKUP;
        }
        break;
      case SOC_PRE_RESTING_LOOKUP:  // Wait for resting lookup
        // TODO: implement RTC checking
        soc_state = SOC_SET;
        break;
      default:  // Do nothing on SOC_SET
        break;
    }
#endif

    osDelay(ACC_UPDATE_FREQ_MS);
  }
}

/**
 * @brief Adds curr sense reading to soc queue. Non-blocking
 *
 * @param cs_low_val Curr Sense low channel reading
 * @param cs_high_val Curr Sense high channel reading
 */
HAL_StatusTypeDef Acc_AddSocCurrSense(int32_t cs_low_val, int32_t cs_high_val, uint32_t timestamp) {
  Curr_Sense_Reading_t r;

  r.cs_low_val_mA = cs_low_val;
  r.cs_high_val_mA = cs_high_val;
  r.timestamp_ticks = timestamp;

  // All messages have priority 0
  if (osMessageQueuePut(Acc_SOC_CurrSenseQueueHandle, &r, 0, 0) != osOK) {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
 * @brief Get the min cell voltage from all modules
 * @return Number of modules checked
 */
uint8_t _Acc_GetMinCellVoltage(uint16_t *min_cell_voltage) {
  uint8_t num_modules_checked = 0;
  Cell_Voltages_t cv;
  *min_cell_voltage = 0; 
  
  for (int i = 0; i < NUM_ACC_MODULES; i++) {
    Acc_GetCellVoltages(acc[i], &cv);
    if (cv.volt_min != 0) { // Has been updated
      num_modules_checked++;
      if (cv.volt_min < *min_cell_voltage) *min_cell_voltage = cv.volt_min; // Update min
    }
  }

  return num_modules_checked;
}