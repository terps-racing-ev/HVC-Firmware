#ifndef CAN_H
#define CAN_H

#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

/* CAN Defaults */
#define CAN_TX_QUEUE_SIZE 64        // Number of messages that can be queued
#define CAN_RX_QUEUE_SIZE 32        // Number of received messages to buffer
#define CAN_TX_TIMEOUT_MS 100       // Timeout for adding message to queue
#define CAN_MAX_RETRIES 3           // Maximum transmission retry attempts
#define CAN_HEARTBEAT_INTERVAL_MS 1000  // Heartbeat message interval (1 second)

/* Message Priority Levels */
#define CAN_PRIORITY_CRITICAL 0     // Safety-critical messages (highest)
#define CAN_PRIORITY_HIGH 1         // Important operational messages
#define CAN_PRIORITY_NORMAL 2       // Standard telemetry messages
#define CAN_PRIORITY_LOW 3          // Debug/diagnostic messages (lowest)

HAL_StatusTypeDef LV_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority);

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
} CAN_Statistics_t;

static void CAN_ResetStatistics(CAN_Statistics_t *stats) {
    stats->bus_off_count = 0;
    stats->rx_message_count = 0;
    stats->rx_queue_full_count = 0;
    stats->tx_error_count = 0;
    stats->tx_queue_full_count = 0;
    stats->tx_success_count = 0;
}

#endif /* CAN_H */