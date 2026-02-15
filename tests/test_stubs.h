#ifndef TEST_STUBS_H
#define TEST_STUBS_H

#include <stddef.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

void Test_Stubs_Reset(void);

void Test_SetQueueNewResults(const osMessageQueueId_t *results, size_t count);
uint32_t Test_GetQueueNewCallCount(void);

void Test_SetCanActivateNotificationResult(HAL_StatusTypeDef result);
uint32_t Test_GetCanActivateNotificationCallCount(void);

void Test_SetCanAddTxResult(HAL_StatusTypeDef result);
void Test_SetCanGetRxResult(HAL_StatusTypeDef result, uint32_t ext_id, uint8_t dlc);

void Test_SetKernelTick(uint32_t tick);

#endif /* TEST_STUBS_H */
