#ifndef BMS_STUBS_H
#define BMS_STUBS_H

#include <stddef.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "can.h"
#include "bms_can_manager.h"

void Test_Stubs_Reset(void);

uint32_t Test_GetDispatchRegisterMatchCount(void);

HAL_StatusTypeDef Test_BMS_CAN_ProcessRXMessage(CAN_Message_t *msg);

#endif /* BMS_STUBS_H */
