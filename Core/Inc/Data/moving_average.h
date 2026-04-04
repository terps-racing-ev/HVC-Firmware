#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include <stdint.h>
#include <stddef.h>

#define MA_WINDOW_SIZE 8

typedef struct {

    float weight;
    float filtered_val;

} MovingAverage_Data_t;

void MovingAverage_Init(MovingAverage_Data_t* ma, uint8_t window_size);

int32_t MovingAverage_Update(MovingAverage_Data_t* ma, int32_t new_val);

#endif // MOVING_AVERAGE_H