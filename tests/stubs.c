#include "test_stubs.h"
#include "spi_can.h"
#include "io.h"
#include "therm.h"
#include "can.h"

static osMessageQueueId_t queue_new_results[8];
static size_t queue_new_result_count = 0;
static size_t queue_new_result_index = 0;
static uint32_t queue_new_call_count = 0;

static osMutexId_t mutex_new_results[8];
static size_t mutex_new_result_count = 0;
static size_t mutex_new_result_index = 0;
static uint32_t mutex_new_call_count = 0;

static HAL_StatusTypeDef can_activate_notification_result = HAL_OK;
static uint32_t can_activate_notification_call_count = 0;

static HAL_StatusTypeDef can_add_tx_result = HAL_OK;
static HAL_StatusTypeDef can_get_rx_result = HAL_OK;
static uint32_t can_get_rx_ext_id = 0;
static uint8_t can_get_rx_dlc = 0;

static HAL_StatusTypeDef adc_start_result = HAL_OK;
static HAL_StatusTypeDef comp_start_result = HAL_OK;

static uint32_t kernel_tick = 0;

CAN_HandleTypeDef hcan1;
ADC_HandleTypeDef hadc1;
COMP_HandleTypeDef hcomp2;

void Test_Stubs_Reset(void)
{
    queue_new_result_count = 0;
    queue_new_result_index = 0;
    queue_new_call_count = 0;
    mutex_new_result_count = 0;
    mutex_new_result_index = 0;
    mutex_new_call_count = 0;
    can_activate_notification_result = HAL_OK;
    can_activate_notification_call_count = 0;
    can_add_tx_result = HAL_OK;
    can_get_rx_result = HAL_OK;
    can_get_rx_ext_id = 0;
    can_get_rx_dlc = 0;
    adc_start_result = HAL_OK;
    comp_start_result = HAL_OK;
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

void Test_SetMutexNewResults(const osMutexId_t *results, size_t count)
{
    size_t i = 0;
    mutex_new_result_count = (count > 8) ? 8 : count;
    for (i = 0; i < mutex_new_result_count; i++) {
        mutex_new_results[i] = results[i];
    }
    mutex_new_result_index = 0;
}

uint32_t Test_GetMutexNewCallCount(void)
{
    return mutex_new_call_count;
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

void Test_SetAdcStartResult(HAL_StatusTypeDef result)
{
    adc_start_result = result;
}

void Test_SetCompStartResult(HAL_StatusTypeDef result)
{
    comp_start_result = result;
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

osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    (void)mq_id;
    (void)msg_ptr;
    (void)msg_prio;
    (void)timeout;
    return osErrorTimeout;
}

osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
    (void)attr;

    mutex_new_call_count++;
    if (mutex_new_result_index < mutex_new_result_count) {
        return mutex_new_results[mutex_new_result_index++];
    }

    return (osMutexId_t)0x1;
}

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    (void)mutex_id;
    (void)timeout;
    return osOK;
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    (void)mutex_id;
    return osOK;
}

void osDelay(uint32_t ticks)
{
    (void)ticks;
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
    (void)flags;
    (void)options;
    (void)timeout;
    return 0;
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    return flags;
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

HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    return adc_start_result;
}

HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *sConfig)
{
    (void)hadc;
    (void)sConfig;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc, uint32_t timeout)
{
    (void)hadc;
    (void)timeout;
    return HAL_OK;
}

uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    return 0;
}

HAL_StatusTypeDef HAL_COMP_Start(COMP_HandleTypeDef *hcomp)
{
    (void)hcomp;
    return comp_start_result;
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

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    (void)GPIOx;
    (void)GPIO_Pin;
    return GPIO_PIN_RESET;
}

uint8_t CANSPI_Receive(uCAN_MSG *msg)
{
    (void)msg;
    return 0;
}

uint8_t CANSPI_Transmit(uCAN_MSG *tempCanMsg)
{
    (void)tempCanMsg;
    return 1;
}

uint8_t CANSPI_isRxErrorPassive(void)
{
    return 0;
}

bool CANSPI_Initialize(void)
{
    return true;
}

uint8_t IO_GetDigitalIO(DigitalIO *dio)
{
    return dio->value;
}

uint16_t IO_GetAnalogIO(AnalogIO *aio)
{
    return aio->value;
}

float IO_GetTemp(Temp *t)
{
    return t->value;
}

void IO_SetDigitalIO(DigitalIO *dio, uint16_t value)
{
    dio->value = (uint8_t)value;
    dio->last_updated = osKernelGetTickCount();
}

void IO_SetAnalogIO(AnalogIO *aio, uint16_t value)
{
    aio->value = value;
    aio->last_updated = osKernelGetTickCount();
}

void IO_SetTemp(Temp *t, float value)
{
    t->value = value;
    t->last_updated = osKernelGetTickCount();
}

float Therm_CalculateTemperature(uint16_t adc_value)
{
    (void)adc_value;
    return 25.0f;
}

__attribute__((weak)) HAL_StatusTypeDef LV_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority)
{
    (void)id;
    (void)data;
    (void)length;
    (void)priority;
    return HAL_OK;
}
