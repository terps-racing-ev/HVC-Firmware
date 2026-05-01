#ifndef CAN_H
#define CAN_H

#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>

#define BMS_ECHO_MSGS 1

/* CAN Defaults */
#define CAN_TX_QUEUE_SIZE 64        // Number of messages that can be queued
#define CAN_RX_QUEUE_SIZE 32        // Number of received messages to buffer
#define CAN_TX_TIMEOUT_MS 100       // Timeout for adding message to queue
#define CAN_MAX_RETRIES 3           // Maximum transmission retry attempts
#define CAN_HEARTBEAT_INTERVAL_MS 1000  // Heartbeat message interval (1 second)
#define CAN_TASK_MAX_RX_PER_CYCLE 32 // Max RX messages to process each task cycle
#define CAN_TASK_MAX_TX_PER_CYCLE 32 // Max TX messages to process each task cycle
#define CAN_RECOVERY_BACKOFF_BASE_MS 10 // Initial CAN recovery backoff delay
#define CAN_RECOVERY_BACKOFF_MAX_MS 200 // Max CAN recovery backoff delay
#define CAN_ERROR_HOLD_TICKS 500 // Keep CAN error flag active for this many ticks

/* Message Priority Levels */
#define CAN_PRIORITY_CRITICAL 0     // Safety-critical messages (highest)
#define CAN_PRIORITY_HIGH 1         // Important operational messages
#define CAN_PRIORITY_NORMAL 2       // Standard telemetry messages
#define CAN_PRIORITY_LOW 3          // Debug/diagnostic messages (lowest)

HAL_StatusTypeDef LV_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority);
HAL_StatusTypeDef BMS_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority);
bool BMS_CAN_HasError(void);
bool LV_CAN_HasError(void);

typedef struct {
	uint32_t id;
	uint8_t data[8];
	uint8_t length;
	uint8_t priority;
	uint32_t timestamp;
} CAN_Message_t;

typedef struct {
	uint32_t tx_success_count;
	uint32_t tx_error_count;
	uint32_t tx_queue_full_count;
	uint32_t rx_message_count;
	uint32_t rx_queue_full_count;
	uint32_t bus_off_count;
	uint32_t error_passive_count;
	uint32_t recovery_count;
	uint32_t rx_invalid_count;
} CAN_Statistics_t;

static void CAN_ResetStatistics(CAN_Statistics_t *stats) {
    stats->bus_off_count = 0;
    stats->error_passive_count = 0;
    stats->recovery_count = 0;
    stats->rx_invalid_count = 0;
    stats->rx_message_count = 0;
    stats->rx_queue_full_count = 0;
    stats->tx_error_count = 0;
    stats->tx_queue_full_count = 0;
    stats->tx_success_count = 0;
}

#endif /* CAN_H */