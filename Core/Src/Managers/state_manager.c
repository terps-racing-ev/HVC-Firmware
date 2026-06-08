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
osEventFlagsId_t floating_input_flag = NULL;

/* Private variables --------------------------------------------------------*/
/* BMB reporting fields, refreshed every cycle by _State_CheckErrors and sent in
   the state CAN message. bmb_fault_module is the index of the module that
   latched a fault (BMB_FAULT_NO_MODULE when none). bmb_reported_state is the
   aggregate live module state, constrained to IDLE / BALANCING / FAULT
   (priority FAULT > BALANCING > IDLE). Only touched from the State_Manager
   task, so no locking is required. */
static uint8_t bmb_fault_module = BMB_FAULT_NO_MODULE;
static uint8_t bmb_reported_state = (uint8_t)BMB_STATE_IDLE;

/* Private functions ---------------------------------------------------------*/
static void _State_PackCanMessage(State state, ErrorMask errors, uint8_t *data, uint8_t *length);
static State _State_Transition(State curr_state, ErrorMask errors, bool charging_requested);
static ErrorMask _State_CheckErrors(void);

/**
  * @brief  Initialize State manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef State_Manager_Init(void) {
    if (State_InitState(&bms_state) != HAL_OK) return HAL_ERROR;

    const osEventFlagsAttr_t floating_input_flag_attr = {
        .name = "Floating_Input_Flag"
    };
    floating_input_flag = osEventFlagsNew(&floating_input_flag_attr);
    if (floating_input_flag == NULL) {
        return HAL_ERROR;
    }

    const osEventFlagsAttr_t charge_flag_attr = {
        .name = "Charge_Flag"
    };
    charge_flag = osEventFlagsNew(&charge_flag_attr);
    if (charge_flag == NULL) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
  * @brief  Main State manager task
  * @param  argument: Not used
  * @retval None
  */
void State_ManagerTask(void *argument) {
    uint8_t data[8];
    uint8_t length;
    ErrorMask errors = 0U;
    State curr_state;

    for (;;) {
        State_GetState(&curr_state);
        errors = _State_CheckErrors();
        State_SetErrorMask(errors);
        bool charging_requested = false;
        
        // Handle charging logic if triggered
        uint32_t charge_events = osEventFlagsWait(charge_flag, CHARGING_EVENT, osFlagsWaitAny, 0U);
        if (((charge_events & osFlagsError) == 0U) && ((charge_events & CHARGING_EVENT) != 0U)) {
            osEventFlagsClear(charge_flag, CHARGING_EVENT);
            charging_requested = true;
        }

        curr_state = _State_Transition(curr_state, errors, charging_requested);

        if (curr_state == ERRORED) {
            IO_SetDigitalIO(&bms_fault, false);
        } else {
            IO_SetDigitalIO(&bms_fault, true);
        }

        _State_PackCanMessage(curr_state, errors, data, &length);
        HVC_CAN_SendMessage(
            CAN_ID_STATE,
            data,
            length,
            CAN_PRIORITY_CRITICAL
        );

        osDelay(STATE_REFRESH_FREQ_MS);
    }
}

/**
 * @brief Evaluate one state-machine transition cycle and return the current state.
 * @retval State Current state for this task cycle.
 */
static State _State_Transition(State curr_state, ErrorMask errors, bool charging_requested) {
    static uint8_t cycles_since_charging_requested = 0;
    
    switch (curr_state) {
        case PRE_INIT:
            //Check if everything else is initialized
            if (io_initialized && bms_can_initialized && lv_can_initialized) {
                if (errors) {
                    curr_state = ERRORED;
                    State_SetState(ERRORED);
                } else {
                    curr_state = RUNNING;
                    State_SetState(RUNNING);
                }   
            }
            break;
        case RUNNING:
            if (errors) {
                curr_state = ERRORED;
                State_SetState(ERRORED);
            } else if (charging_requested) {
                curr_state = CHARGING;
                State_SetState(CHARGING);
                cycles_since_charging_requested = 0;
            }
            break;
        case CHARGING:
            if (
                errors || 
                cycles_since_charging_requested > MAX_CYCLES_WITHOUT_CHARGE_REQUEST
            ) {
                curr_state = ERRORED;
                State_SetState(ERRORED);
            } else if (!charging_requested) {
                cycles_since_charging_requested++;
            } else {
                cycles_since_charging_requested = 0;
            }
            break;
        case ERRORED:
            // TODO: check this works for charging
            if (!errors) {
                curr_state = RUNNING;
                State_SetState(RUNNING);
            }
            break;
        default:
            // Errored state
            break;
    }
    
    return curr_state;
}

static ErrorMask _State_CheckErrors(void) {
    ErrorMask errors = 0;
    uint32_t now  = osKernelGetTickCount();
    uint32_t last_heartbeat;
    FloatingInputMask floating_inputs = 0U;

    // Reset BMB reporting; re-derived below from live module states / faults.
    bmb_fault_module = BMB_FAULT_NO_MODULE;
    bmb_reported_state = (uint8_t)BMB_STATE_IDLE;

    if (floating_input_flag != NULL) {
        floating_inputs = (FloatingInputMask)osEventFlagsGet(floating_input_flag);
    }

    if (CHECK_REF_OVERTEMP) {
        // Ref overtemp
        float temp = IO_GetTemp(&ref_temp);
        if (temp > 60.0f) {
            SET_ERROR(errors, BMS_ERR_REF_OVER_TEMP);
        }
    }

    if (CHECK_MODULE_TIMEOUT) {
        // Heartbeat
        for (int i = 0; i < NUM_ACC_MODULES; i++) {
            Acc_GetHeartbeatLastUpdate(acc[i], &last_heartbeat);
            if (now-last_heartbeat > MODULE_TIMEOUT_CUTOFF_TICKS) {
                SET_ERROR(errors, BMS_ERR_MODULE_TIMEOUT);
            }
        }
    }

    if (CHECK_BATT_FLOATING) {
        if ((floating_inputs & BMS_FLOATING_INPUT_BATT) != 0U) {
            SET_ERROR(errors, BMS_ERR_BATT_FLOATING);
        }
    }

    if (CHECK_CURR_SENSE_FLOATING) {
        if ((floating_inputs & BMS_FLOATING_INPUT_CURR_SENSE) != 0U) {
            SET_ERROR(errors, BMS_ERR_CURR_SENSE_FLOATING);
        }
    }

    if (CHECK_BMB_ERRORS) {
        // Each module runs its own state machine and reports a Fault_Count in
        // its heartbeat. A non-zero Fault_Count means that module is faulting;
        // require it to stay > 0 for BMB_FAULT_GRACE_PERIOD_MS before latching a
        // BMS fault and recording which module tripped it.
        //
        // Separately, aggregate the live module BMS_State into a single reported
        // value constrained to IDLE / BALANCING / FAULT (priority
        // FAULT > BALANCING > IDLE). Any other/transient module state (e.g. INIT
        // at startup) is reported as IDLE.
        static bool fault_pending[NUM_ACC_MODULES] = {0};
        static uint32_t fault_since[NUM_ACC_MODULES] = {0};
        uint16_t fault_count;
        uint8_t module_state;
        uint8_t reported_state = (uint8_t)BMB_STATE_IDLE;

        for (int i = 0; i < NUM_ACC_MODULES; i++) {
            Acc_GetHeartbeatStatus(acc[i], &fault_count, &module_state);

            if (fault_count > 0U) {
                if (!fault_pending[i]) {
                    fault_pending[i] = true;
                    fault_since[i] = now;
                }

                if ((now - fault_since[i]) >= BMB_FAULT_GRACE_PERIOD_MS) {
                    SET_ERROR(errors, BMS_ERR_BMB_ERROR);
                    // Record the first (lowest-index) module that tripped.
                    if (bmb_fault_module == BMB_FAULT_NO_MODULE) {
                        bmb_fault_module = (uint8_t)i;
                    }
                    reported_state = (uint8_t)BMB_STATE_FAULT;
                }
            } else {
                fault_pending[i] = false;
            }

            // Fold this module's live state into the reported aggregate.
            if (module_state == (uint8_t)BMB_STATE_FAULT) {
                reported_state = (uint8_t)BMB_STATE_FAULT;
            } else if ((module_state == (uint8_t)BMB_STATE_BALANCING) &&
                       (reported_state != (uint8_t)BMB_STATE_FAULT)) {
                reported_state = (uint8_t)BMB_STATE_BALANCING;
            }
        }

        bmb_reported_state = reported_state;
    }

    if (CHECK_BMS_CAN_ERRORS) {
        if (BMS_CAN_HasError()) {
            SET_ERROR(errors, BMS_ERR_BMS_CAN_ERROR);
        }
    }

    if (CHECK_LV_CAN_ERRORS) {
        if (LV_CAN_HasError()) {
            SET_ERROR(errors, BMS_ERR_LV_CAN_ERROR);
        }
    }
    
    return errors;
}

static void _State_PackCanMessage(State state, ErrorMask errors, uint8_t *data, uint8_t *length)
{
    if ((data == NULL) || (length == NULL)) {
        return;
    }

    data[0] = (uint8_t)state;
    data[1] = (uint8_t)(errors & 0xFFU);
    data[2] = (uint8_t)((errors >> 8U) & 0xFFU);
    data[3] = (uint8_t)((errors >> 16U) & 0xFFU);
    data[4] = (uint8_t)((errors >> 24U) & 0xFFU);
    // data[5]: index of the module that latched a BMB fault, or
    //          BMB_FAULT_NO_MODULE (0xFF) when none.
    // data[6]: aggregate live BMB state (IDLE / BALANCING / FAULT).
    data[5] = bmb_fault_module;
    data[6] = bmb_reported_state;
    data[7] = 0U;

    *length = 7U;
}
