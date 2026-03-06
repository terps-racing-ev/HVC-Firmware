#ifndef TEST_STUBS_H
#define TEST_STUBS_H

#include <stddef.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

typedef void (*Test_DelayHook)(uint32_t ticks);

void Test_Stubs_Reset(void);

void Test_SetQueueNewResults(const osMessageQueueId_t *results, size_t count);
uint32_t Test_GetQueueNewCallCount(void);

void Test_SetMutexNewResults(const osMutexId_t *results, size_t count);
uint32_t Test_GetMutexNewCallCount(void);

void Test_SetCanActivateNotificationResult(HAL_StatusTypeDef result);
uint32_t Test_GetCanActivateNotificationCallCount(void);

void Test_SetCanAddTxResult(HAL_StatusTypeDef result);
void Test_SetCanGetRxResult(HAL_StatusTypeDef result, uint32_t ext_id, uint8_t dlc);

void Test_SetKernelTick(uint32_t tick);
void Test_SetDelayHook(Test_DelayHook hook);
void Test_SetAdcStartResult(HAL_StatusTypeDef result);
void Test_SetCompStartResult(HAL_StatusTypeDef result);

uint32_t Test_GetBmsCanSendCallCount(void);
uint32_t Test_GetLvCanSendCallCount(void);
uint32_t Test_GetLastBmsCanId(void);
uint8_t Test_GetLastBmsCanLength(void);
uint8_t Test_GetLastBmsCanDataByte(uint8_t index);
uint32_t Test_GetLastLvCanId(void);
uint8_t Test_GetLastLvCanLength(void);
uint8_t Test_GetLastLvCanDataByte(uint8_t index);
uint32_t Test_GetDelayCallCount(void);
uint32_t Test_GetLastDelayTicks(void);

#endif /* TEST_STUBS_H */
