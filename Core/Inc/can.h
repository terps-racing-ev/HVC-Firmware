#ifndef __CAN_H
#define __CAN_H

/* Defines -------------------------------------------------------------------*/
#define CAN_TX_QUEUE_SIZE 64        // Number of messages that can be queued
#define CAN_RX_QUEUE_SIZE 32        // Number of received messages to buffer
#define CAN_TX_TIMEOUT_MS 100       // Timeout for adding message to queue
#define CAN_MAX_RETRIES 3           // Maximum transmission retry attempts
#define CAN_HEARTBEAT_INTERVAL_MS 1000  // Heartbeat message interval (1 second)

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

/* External Variables --------------------------------------------------------*/
extern osMessageQueueId_t LV_CAN_TxQueueHandle;

#endif /* CAN_H_ */