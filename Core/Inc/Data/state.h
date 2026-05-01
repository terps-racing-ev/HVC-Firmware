#ifndef __STATE_H
#define __STATE_H

#include <stdbool.h>
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

typedef enum {
    PRE_INIT = 0,
    RUNNING,
    CHARGING,
    BALANCING,
    ERRORED
} State;

typedef struct {
    osMutexId_t mutex;
    State state;
} Locked_State;

extern Locked_State bms_state;

typedef uint32_t ErrorMask;
typedef uint32_t FloatingInputMask;

typedef struct {
    osMutexId_t mutex;
    ErrorMask error_mask;
} Locked_ErrorMask;

typedef enum {
    BMS_ERR_REF_OVER_TEMP = 0,
    BMS_ERR_MODULE_TIMEOUT = 1,
    BMS_ERR_BATT_FLOATING = 2,
    BMS_ERR_CURR_SENSE_FLOATING = 3,
    BMS_ERR_BMB_ERROR = 4,
    BMS_ERR_BMS_CAN_ERROR = 5,
    BMS_ERR_LV_CAN_ERROR = 6,
    BMS_ERR_DEFAULT
} ErrorBit;

typedef enum {
    BMS_FLOATING_INPUT_BATT = 1U << 0,
    BMS_FLOATING_INPUT_CURR_SENSE = 1U << 1,
} FloatingInputBit;

extern Locked_ErrorMask bms_errors;
extern osEventFlagsId_t floating_input_flag;

// True after initialiation
extern uint8_t bms_can_initialized;
extern uint8_t lv_can_initialized;
extern uint8_t acc_initialized;
extern uint8_t io_initialized;

// Flag for charging message
#define CHARGING_EVENT 1
extern osEventFlagsId_t charge_flag;

HAL_StatusTypeDef State_InitState(Locked_State *state);
void State_GetState(State *state);
void State_SetState(State new_state);
void State_SetErrorMask(ErrorMask mask);
void State_GetErrorMask(ErrorMask *mask);

/**
 * @brief Set one error bit in ErrorMask
 */
#define SET_ERROR(mask, bit) (mask) |= ((ErrorMask)1U << (uint32_t)(bit))                                

#endif /* __STATE_H */
