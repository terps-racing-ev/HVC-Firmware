#ifndef CMSIS_OS_H
#define CMSIS_OS_H

#include <stdint.h>

typedef void* osMessageQueueId_t;
typedef uint32_t osStatus_t;

#define osOK 0U

typedef void* osMutexId_t;

typedef uint32_t osWaitForever_t;
#define osWaitForever 0xFFFFFFFFU

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const void *attr);
osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);

void osDelay(uint32_t ticks);
uint32_t osKernelGetTickCount(void);

#endif /* CMSIS_OS_H */
