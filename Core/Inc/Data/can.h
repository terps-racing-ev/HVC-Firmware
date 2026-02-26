#ifndef __CAN_H
#define __CAN_H

#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

// TODO: replace this include with int types ?
#include <stdint.h>

/* Defines -------------------------------------------------------------------*/

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

/* CAN Message Structure */
typedef struct {
    uint32_t id;                    // CAN message ID (29-bit extended)
    uint8_t data[8];                // Message data (up to 8 bytes)
    uint8_t length;                 // Data length (0-8)
    uint8_t priority;               // Message priority (0 = highest)
    uint32_t timestamp;             // Timestamp when message was queued
} CAN_Message_t;

/* CAN Statistics Structure */
typedef struct {
    uint32_t tx_success_count;      // Successfully transmitted messages
    uint32_t tx_error_count;        // Failed transmissions
    uint32_t tx_queue_full_count;   // Times TX queue was full
    uint32_t rx_message_count;      // Received messages
    uint32_t rx_queue_full_count;   // Times RX queue was full
    uint32_t bus_off_count;         // CAN bus-off events
} CAN_Statistics_t;

/* Public Functions --------------------------------------------------------*/

/**
  * @brief  Send CAN message on LV bus (non-blocking, queues message)
  * @param  id: CAN message ID (29-bit extended, max 0x1FFFFFFF)
  * @param  data: Pointer to data buffer (up to 8 bytes)
  * @param  length: Data length (0-8 bytes)
  * @param  priority: Message priority (0 = highest, 3 = lowest)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef LV_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority);

/**
  * @brief  Send CAN message (non-blocking, queues message)
  * @param  id: CAN message ID (29-bit extended, max 0x1FFFFFFF)
  * @param  data: Pointer to data buffer (up to 8 bytes)
  * @param  length: Data length (0-8 bytes)
  * @param  priority: Message priority (0 = highest, 3 = lowest)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BMS_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority);

/* Shared Functions  --------------------------------------------------------*/

/**
  * @brief  Reset CAN statistics (by zeroing out)
  * @param  stats: Can statistics struct
  * @retval none
  */
void CAN_ResetStatistics(CAN_Statistics_t *stats);

#endif /* CAN_H_ */