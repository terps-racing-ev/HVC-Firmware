#include "stdint.h"

// Given OCV Voltage underestimates max cell ocv
#define NEG_CURR_LIMIT_SAFETY_FACTOR_mA 10000

typedef struct {
    uint16_t ocv_voltage_mV;
} CurrLimitInput_t;

typedef struct {
    uint32_t neg_curr_limit_mA;
    uint32_t pos_curr_limit_mA;
} CurrLimitOutput_t;

void CurrLimit_Calculate(CurrLimitInput_t *in, CurrLimitOutput_t *out);