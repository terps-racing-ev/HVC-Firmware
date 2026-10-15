#include "vsense.h"

uint32_t VSense_CalculateVoltage(uint16_t adc_value) {
    return (uint32_t)((adc_value * 3.3f/4095.0) * READ_VOLT_TO_INPUT_VOLT * 1000.0);
}