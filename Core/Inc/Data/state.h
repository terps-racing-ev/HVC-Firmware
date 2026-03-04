#ifndef __STATE_H
#define __STATE_H

#include <stdbool.h>
#include "cmsis_os.h"

typedef enum {
    PRE_INIT = 0,
    OK,
    ERRORED
} State;

typedef struct {
    osMutexId_t mutex;
    State state;
} Locked_State;

extern Locked_State bms_state;

typedef uint32_t ErrorMask;

typedef struct {
    osMutexId_t mutex;
    ErrorMask error_mask;
} Locked_ErrorMask;

typedef enum {
    BMS_ERR_REF_OVER_TEMP = 0,
    BMS_ERR_MODULE_TIMEOUT,

    // Keep this last so = # errors added
    BMS_ERR_COUNT 
} ErrorBit;

extern Locked_ErrorMask bms_errors;

// True after initialiation
extern uint8_t lv_can_initialized;
extern uint8_t bms_can_initialized;
extern uint8_t io_initialized;

void State_GetState(State *state);
void State_SetState(State new_state);
void State_SetErrorMask(ErrorMask mask);
void State_GetErrorMask(ErrorMask *mask);

/**
 * @brief Set one error bit in ErrorMask
 */
#define SET_ERROR(mask, bit) (mask) |= ((ErrorMask)1U << (uint32_t)(bit))                                

#endif /* __STATE_H */
