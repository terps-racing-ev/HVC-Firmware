#ifndef CMSIS_OS_H
#define CMSIS_OS_H

#include <stdint.h>

typedef void *osMessageQueueId_t;

typedef uint32_t osStatus_t;

#define osOK 0U

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, void *attr);
osStatus_t osMessageQueueGet(osMessageQueueId_t queue_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout);
osStatus_t osMessageQueuePut(osMessageQueueId_t queue_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);

void osDelay(uint32_t ms);
uint32_t osKernelGetTickCount(void);

#endif
