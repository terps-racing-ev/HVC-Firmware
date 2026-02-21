#ifndef CMSIS_OS_H
#define CMSIS_OS_H

#include <stdint.h>

typedef void* osMessageQueueId_t;
typedef uint32_t osStatus_t;

#define osOK 0U
#define osErrorTimeout 1U

typedef void* osMutexId_t;

typedef uint32_t osWaitForever_t;
#define osWaitForever 0xFFFFFFFFU

#define osFlagsWaitAny 0x00000000U

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const void *attr);
osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);
osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout);

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout);
uint32_t osThreadFlagsClear(uint32_t flags);

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout);
uint32_t osThreadFlagsClear(uint32_t flags);

void osDelay(uint32_t ticks);
uint32_t osKernelGetTickCount(void);

#endif /* CMSIS_OS_H */
