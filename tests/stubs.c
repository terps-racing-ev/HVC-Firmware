#include "test_stubs.h"
#include "CANSPI.h"

static osMessageQueueId_t queue_new_results[8];
static size_t queue_new_result_count = 0;
static size_t queue_new_result_index = 0;
static uint32_t queue_new_call_count = 0;

static HAL_StatusTypeDef can_activate_notification_result = HAL_OK;
static uint32_t can_activate_notification_call_count = 0;

static HAL_StatusTypeDef can_add_tx_result = HAL_OK;
static HAL_StatusTypeDef can_get_rx_result = HAL_OK;
static uint32_t can_get_rx_ext_id = 0;
static uint8_t can_get_rx_dlc = 0;

static uint32_t kernel_tick = 0;

CAN_HandleTypeDef hcan1;

void Test_Stubs_Reset(void)
{
    queue_new_result_count = 0;
    queue_new_result_index = 0;
    queue_new_call_count = 0;
    can_activate_notification_result = HAL_OK;
    can_activate_notification_call_count = 0;
    can_add_tx_result = HAL_OK;
    can_get_rx_result = HAL_OK;
    can_get_rx_ext_id = 0;
    can_get_rx_dlc = 0;
    kernel_tick = 0;
}

void Test_SetQueueNewResults(const osMessageQueueId_t *results, size_t count)
{
    size_t i = 0;
    queue_new_result_count = (count > 8) ? 8 : count;
    for (i = 0; i < queue_new_result_count; i++) {
        queue_new_results[i] = results[i];
    }
    queue_new_result_index = 0;
}

uint32_t Test_GetQueueNewCallCount(void)
{
    return queue_new_call_count;
}

void Test_SetCanActivateNotificationResult(HAL_StatusTypeDef result)
{
    can_activate_notification_result = result;
}

uint32_t Test_GetCanActivateNotificationCallCount(void)
{
    return can_activate_notification_call_count;
}

void Test_SetCanAddTxResult(HAL_StatusTypeDef result)
{
    can_add_tx_result = result;
}

void Test_SetCanGetRxResult(HAL_StatusTypeDef result, uint32_t ext_id, uint8_t dlc)
{
    can_get_rx_result = result;
    can_get_rx_ext_id = ext_id;
    can_get_rx_dlc = dlc;
}

void Test_SetKernelTick(uint32_t tick)
{
    kernel_tick = tick;
}

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const void *attr)
{
    (void)msg_count;
    (void)msg_size;
    (void)attr;

    queue_new_call_count++;
    if (queue_new_result_index < queue_new_result_count) {
        return queue_new_results[queue_new_result_index++];
    }

    return (osMessageQueueId_t)0x1;
}

osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    (void)mq_id;
    (void)msg_ptr;
    (void)msg_prio;
    (void)timeout;
    return osOK;
}

void osDelay(uint32_t ticks)
{
    (void)ticks;
}

uint32_t osKernelGetTickCount(void)
{
    return kernel_tick;
}

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t active_it)
{
    (void)hcan;
    (void)active_it;
    can_activate_notification_call_count++;
    return can_activate_notification_result;
}

HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t *pData, uint32_t *pTxMailbox)
{
    (void)hcan;
    (void)pHeader;
    (void)pData;
    (void)pTxMailbox;
    return can_add_tx_result;
}

HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t fifo, CAN_RxHeaderTypeDef *pHeader, uint8_t *aData)
{
    (void)hcan;
    (void)fifo;
    (void)aData;
    if (pHeader != NULL) {
        pHeader->ExtId = can_get_rx_ext_id;
        pHeader->DLC = can_get_rx_dlc;
    }
    return can_get_rx_result;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    (void)GPIOx;
    (void)GPIO_Pin;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    (void)GPIOx;
    (void)GPIO_Pin;
    (void)PinState;
}

int CANSPI_Receive(uCAN_MSG *msg)
{
    (void)msg;
    return 0;
}

int CANSPI_isRxErrorPassive(void)
{
    return 0;
}
