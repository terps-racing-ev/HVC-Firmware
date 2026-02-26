#include "state.h"

State State_GetState(){
    State state;

    osMutexAcquire(bms_state.mutex, osWaitForever);
    state = bms_state.state;
    osMutexRelease(bms_state.mutex);

    return state;
}


void State_SetState(State new_state) {
    osMutexAcquire(bms_state.mutex, osWaitForever);
    bms_state.state = new_state;
    osMutexRelease(bms_state.mutex);
}