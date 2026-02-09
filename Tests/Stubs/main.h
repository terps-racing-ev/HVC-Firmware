#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

typedef struct {
    int dummy;
} GPIO_TypeDef;

typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

extern GPIO_TypeDef *LD3_GPIO_Port;
#define LD3_Pin (1U << 3)

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#endif
