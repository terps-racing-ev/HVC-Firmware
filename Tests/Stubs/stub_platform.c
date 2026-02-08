#include "stub_platform.h"
#include "main.h"

static GPIO_TypeDef stub_gpio_port;
GPIO_TypeDef *LD3_GPIO_Port = &stub_gpio_port;

CAN_HandleTypeDef hcan1;

osMessageQueueId_t stub_queue_new_returns[2] = {0};
int stub_queue_new_call_count = 0;
osStatus_t stub_queue_get_status = osOK;
osStatus_t stub_queue_put_status = osOK;

HAL_StatusTypeDef stub_can_activate_status = HAL_OK;
HAL_StatusTypeDef stub_can_add_tx_status = HAL_OK;
HAL_StatusTypeDef stub_can_get_rx_status = HAL_OK;

uint32_t stub_tick = 0;

int stub_gpio_write_count = 0;
int stub_gpio_toggle_count = 0;

CAN_Error stub_mcp_send_status = CAN_OK;

void stub_platform_reset(void)
{
    stub_queue_new_returns[0] = (osMessageQueueId_t)0x1;
    stub_queue_new_returns[1] = (osMessageQueueId_t)0x1;
    stub_queue_new_call_count = 0;
    stub_queue_get_status = osOK;
    stub_queue_put_status = osOK;
    stub_can_activate_status = HAL_OK;
    stub_can_add_tx_status = HAL_OK;
    stub_can_get_rx_status = HAL_OK;
    stub_tick = 0;
    stub_gpio_write_count = 0;
    stub_gpio_toggle_count = 0;
    stub_mcp_send_status = CAN_OK;
}

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, void *attr)
{
    (void)msg_count;
    (void)msg_size;
    (void)attr;
    if (stub_queue_new_call_count < 2) {
        return stub_queue_new_returns[stub_queue_new_call_count++];
    }
    return stub_queue_new_returns[1];
}

osStatus_t osMessageQueueGet(osMessageQueueId_t queue_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    (void)queue_id;
    (void)msg_ptr;
    (void)msg_prio;
    (void)timeout;
    return stub_queue_get_status;
}

osStatus_t osMessageQueuePut(osMessageQueueId_t queue_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    (void)queue_id;
    (void)msg_ptr;
    (void)msg_prio;
    (void)timeout;
    return stub_queue_put_status;
}

void osDelay(uint32_t ms)
{
    (void)ms;
}

uint32_t osKernelGetTickCount(void)
{
    return stub_tick;
}

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs)
{
    (void)hcan;
    (void)ActiveITs;
    return stub_can_activate_status;
}

HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t *aData, uint32_t *pTxMailbox)
{
    (void)hcan;
    (void)pHeader;
    (void)aData;
    (void)pTxMailbox;
    return stub_can_add_tx_status;
}

HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *pHeader, uint8_t aData[])
{
    (void)hcan;
    (void)RxFifo;
    (void)pHeader;
    (void)aData;
    return stub_can_get_rx_status;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    (void)GPIOx;
    (void)GPIO_Pin;
    (void)PinState;
    stub_gpio_write_count++;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    (void)GPIOx;
    (void)GPIO_Pin;
    stub_gpio_toggle_count++;
}

CAN_Error MCP_sendMessage(can_frame *frame)
{
    (void)frame;
    return stub_mcp_send_status;
}
