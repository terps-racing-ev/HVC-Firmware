#include "state.h"

/* Public variables ---------------------------------------------------------*/
Locked_ErrorMask bms_errors = {0};

/**
 * @brief  Init BMS state to PRE_INIT and inits mutex
 * @param  state: Output pointer for the current state.
 * @retval HAL_StatusTypeDef: init status
 */
HAL_StatusTypeDef State_InitState(Locked_State *state) {
    const osMutexAttr_t state_mutex_attr = {
        .name = "State_Mutex"
    };
    state->mutex = osMutexNew(&state_mutex_attr);  
    if (!state->mutex) return HAL_ERROR;

    // Always init to PRE_INIT state
    state->state = PRE_INIT;

    return HAL_OK;
}

/**
 * @brief  Read the current high-level BMS state.
 * @param  state: Output pointer for the current state.
 * @retval None
 */
void State_GetState(State *state)
{
    if (state == NULL) {
        return;
    }

    if (bms_state.mutex != NULL) {
        osMutexAcquire(bms_state.mutex, osWaitForever);
    }

    *state = bms_state.state;

    if (bms_state.mutex != NULL) {
        osMutexRelease(bms_state.mutex);
    }
}

/**
 * @brief  Write the high-level BMS state.
 * @param  new_state: New state value.
 * @retval None
 */
void State_SetState(State new_state)
{
    if (bms_state.mutex != NULL) {
        osMutexAcquire(bms_state.mutex, osWaitForever);
    }

    bms_state.state = new_state;

    if (bms_state.mutex != NULL) {
        osMutexRelease(bms_state.mutex);
    }
}

/**
 * @brief  Overwrite the full error bit mask.
 * @param  mask: Full error mask value.
 * @retval None
 */
void State_SetErrorMask(ErrorMask mask)
{
    if (bms_errors.mutex != NULL) {
        osMutexAcquire(bms_errors.mutex, osWaitForever);
    }

    bms_errors.error_mask = mask;

    if (bms_errors.mutex != NULL) {
        osMutexRelease(bms_errors.mutex);
    }
}

/**
 * @brief  Read the full error bit mask.
 * @param  mask: Output pointer for the full error mask.
 * @retval None
 */
void State_GetErrorMask(ErrorMask *mask)
{
    if (mask == NULL) {
        return;
    }

    if (bms_errors.mutex != NULL) {
        osMutexAcquire(bms_errors.mutex, osWaitForever);
    }

    *mask = bms_errors.error_mask;

    if (bms_errors.mutex != NULL) {
        osMutexRelease(bms_errors.mutex);
    }
}
