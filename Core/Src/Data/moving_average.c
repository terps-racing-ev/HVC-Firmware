#include "moving_average.h"

void MovingAverage_Init(MovingAverage_Data_t* ma, uint8_t window_size)
{

    if (ma == NULL || window_size == 0) {
        return;
    }

    ma->weight = 2.0 / (window_size + 1.0);
    ma->filtered_val = 0;

}

int32_t MovingAverage_Update(MovingAverage_Data_t* ma, int32_t new_val)
{
    if (ma == NULL) {
        return new_val;
    }

    ma->filtered_val = (1.0 - ma->weight) * ma->filtered_val + (ma->weight) * new_val;

    return (int32_t) ma->filtered_val;

}