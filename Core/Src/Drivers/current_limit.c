#include "current_limit.h"
#include "cells.h"

#include "stddef.h"

static void CurrLimit_CellVoltage(CurrLimitInput_t *in, CurrLimitOutput_t *out);

typedef void (*CurrLimit_Func)(CurrLimitInput_t *in, CurrLimitOutput_t *out);
CurrLimit_Func CurrLimitPipeline[] = {
    {CurrLimit_CellVoltage}
};
uint8_t PipelineSize = sizeof(CurrLimitPipeline)/sizeof(CurrLimit_Func);

void CurrLimit_Calculate(CurrLimitInput_t *in, CurrLimitOutput_t *out) {
    if ((in == NULL) || (out == NULL)) {
        return;
    }

    // Start from the loosest limits so pipeline stages can only tighten them.
    out->pos_curr_limit_mA = UINT32_MAX;
    out->neg_curr_limit_mA = UINT32_MAX;

    for (uint8_t i = 0; i < PipelineSize; i++) {
        CurrLimitPipeline[i](in, out);
    }
}

static void CurrLimit_CellVoltage(CurrLimitInput_t *in, CurrLimitOutput_t *out) {
    // Calculate using Ohm's Law
    uint32_t neg_curr_voltage_window_mV =
        (in->ocv_voltage_mV < MAX_SAFE_CELL_VOLTAGE_mV)
        ? (uint32_t)(MAX_SAFE_CELL_VOLTAGE_mV - in->ocv_voltage_mV)
        : 0U;
    uint32_t pos_curr_voltage_window_mV =
        (in->ocv_voltage_mV > MIN_SAFE_CELL_VOLTAGE_mV)
        ? (uint32_t)(in->ocv_voltage_mV - MIN_SAFE_CELL_VOLTAGE_mV)
        : 0U;
    
    uint32_t pos_curr_limit_mA = 
        (uint32_t)(((pos_curr_voltage_window_mV * 1000U)/ CELL_INT_RESISTANCE_mOhm)
        * PHYSICAL_CELLS_PER_CELL);
    uint32_t neg_curr_limit_mA = 
        (uint32_t)(((neg_curr_voltage_window_mV * 1000U)/ CELL_INT_RESISTANCE_mOhm)
        * PHYSICAL_CELLS_PER_CELL)
        - NEG_CURR_LIMIT_SAFETY_FACTOR_mA;

    out->pos_curr_limit_mA = 
        (pos_curr_limit_mA < out->pos_curr_limit_mA) ?
        pos_curr_limit_mA :
        out->pos_curr_limit_mA;
    
    out->neg_curr_limit_mA = 
        (neg_curr_limit_mA < out->neg_curr_limit_mA) ?
        neg_curr_limit_mA :
        out->neg_curr_limit_mA;
}



