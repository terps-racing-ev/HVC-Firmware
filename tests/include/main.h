#ifndef MAIN_H
#define MAIN_H

#include "stm32l4xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern COMP_HandleTypeDef hcomp2;

#define IMD_Pin GPIO_PIN_9
#define IMD_GPIO_Port GPIOA
#define SDC_Pin GPIO_PIN_10
#define SDC_GPIO_Port GPIOA
#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOB
#define BMS_Fault_Pin GPIO_PIN_6
#define BMS_Fault_GPIO_Port GPIOB

#endif /* MAIN_H */
