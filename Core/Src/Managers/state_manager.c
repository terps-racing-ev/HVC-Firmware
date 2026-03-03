/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : state_manager.c
  * @brief          : State manager implementation
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

#include "state_manager.h"

/* Public variables ---------------------------------------------------------*/
Locked_State bms_state = {0};

/* Private functions ---------------------------------------------------------*/
static void _State_PackCanMessage(State state, uint8_t *data, uint8_t *length);
static State _State_Transition(void);

/**
  * @brief  Initialize State manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef State_Manager_Init(void) {
    const osMutexAttr_t bms_state_mutex_attr = {
        .name = "BMS_State_Mutex"
    };
    bms_state.mutex = osMutexNew(&bms_state_mutex_attr);
    if (!bms_state.mutex) return HAL_ERROR;

    bms_state.state = PRE_INIT;

    return HAL_OK;
}

/**
  * @brief  Main State manager task
  * @param  argument: Not used
  * @retval None
  */
void State_ManagerTask(void *argument) {
    // TODO: Implement state machine
    uint8_t data[8];
    uint8_t length;
    
    State curr_state = _State_Transition();

    _State_PackCanMessage(curr_state, data, &length);
    BMS_CAN_SendMessage(
        CAN_ID_STATE,
        data,
        length,
        CAN_PRIORITY_CRITICAL
    );
    LV_CAN_SendMessage(
        CAN_ID_STATE,
        data,
        length,
        CAN_PRIORITY_CRITICAL
    );

    osDelay(STATE_REFRESH_FREQ_MS);
}

/**
 * @brief Evaluate one state-machine transition cycle and return the current state.
 * @retval State Current state for this task cycle.
 */
static State _State_Transition(void) {
    State curr_state;
    uint32_t now  = osKernelGetTickCount();
    uint32_t last_heartbeat;

    State_GetState(&curr_state);

    switch (curr_state) {
        case PRE_INIT:
            //Check if everything else is initialized
            if (io_initialized && bms_can_initialized && lv_can_initialized) {
                State_SetState(OK);
            }
            break;
        case OK:
            // TODO: Create error tracking variable
            // Right now, only overtemp will be logged (by state)
            // if both overtemp and timeout errors occur
            if (CHECK_REF_OVERTEMP) {
                // Ref overtemp
                float temp = IO_GetTemp(&ref_temp);
                if (temp > 60.0f) {
                    State_SetState(ERRORED_REF_OVER_TEMP);
                }
            }
            if (CHECK_MODULE_TIMEOUT) {
                // Heartbeat
                for (int i = 0; i < NUM_ACC_MODULES; i++) {
                    Acc_GetHeartbeatLastUpdate(acc[i], &last_heartbeat);
                    if (now-last_heartbeat > MODULE_TIMEOUT_CUTOFF_TICKS) {
                        State_SetState(ERRORED_MODULE_TIMEOUT);
                    }
                }
            }
            // Todo: 
            break;
        default:
            // Errored state
            break;
    }
    
    return curr_state;
}

static void _State_PackCanMessage(State state, uint8_t *data, uint8_t *length)
{
    if ((data == NULL) || (length == NULL)) {
        return;
    }

    data[0] = (uint8_t)state;
    data[1] = 0U;
    data[2] = 0U;
    data[3] = 0U;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;

    *length = 1U;
}
