#ifndef MCP2515_H
#define MCP2515_H

#include <stdint.h>

typedef struct {
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[8];
} can_frame;

typedef enum {
    CAN_OK = 0,
    CAN_FAIL = 1
} CAN_Error;

CAN_Error MCP_sendMessage(can_frame *frame);

#endif
