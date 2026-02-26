#ifndef __STATE_H
#define __STATE_H

#include "cmsis_os.h"

typedef enum {
    PRE_INIT = 0,
    OK,
    ERRORED,
    ERRORED_REF_OVER_TEMP, // Add more errored states for better debugging
} State;

typedef struct {
    osMutexId_t mutex;
    State state;
} Locked_State;

extern Locked_State bms_state;

// True after initialiation
extern uint8_t lv_can_initialized;
extern uint8_t bms_can_initialized;
extern uint8_t io_initialized;

State State_GetState();
void State_SetState(State new_state);

#endif /* __STATE_H */