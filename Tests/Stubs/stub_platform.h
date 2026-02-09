#ifndef STUB_PLATFORM_H
#define STUB_PLATFORM_H

#include "cmsis_os.h"
#include "can.h"
#include "mcp2515.h"

extern osMessageQueueId_t stub_queue_new_returns[2];
extern int stub_queue_new_call_count;
extern osStatus_t stub_queue_get_status;
extern osStatus_t stub_queue_put_status;

extern HAL_StatusTypeDef stub_can_activate_status;
extern HAL_StatusTypeDef stub_can_add_tx_status;
extern HAL_StatusTypeDef stub_can_get_rx_status;

extern uint32_t stub_tick;

extern int stub_gpio_write_count;
extern int stub_gpio_toggle_count;

extern CAN_Error stub_mcp_send_status;

void stub_platform_reset(void);

#endif
