#include "bms_can_stubs.h"

static uint32_t dispatch_register_match_count = 0;

CAN_HandleTypeDef hcan1;

Acc_Module module_0 = {0};
Acc_Module module_1 = {0};
Acc_Module module_2 = {0};
Acc_Module module_3 = {0};
Acc_Module module_4 = {0};
Acc_Module module_5 = {0};

Acc_Module *acc[6] = {
    &module_0,
    &module_1,
    &module_2,
    &module_3,
    &module_4,
    &module_5
};

void Test_Stubs_Reset(void)
{
    dispatch_register_match_count = 0;
    memset(&module_0, 0, sizeof(module_0));
    memset(&module_1, 0, sizeof(module_0));
    memset(&module_2, 0, sizeof(module_0));
    memset(&module_3, 0, sizeof(module_0));
    memset(&module_4, 0, sizeof(module_0));
    memset(&module_5, 0, sizeof(module_0));
}

uint32_t Test_GetDispatchRegisterMatchCount(void)
{
    return dispatch_register_match_count;
}

HAL_StatusTypeDef Test_BMS_CAN_ProcessRXMessage(CAN_Message_t *msg) {
    // Prefetch for faster loops
    uint32_t msg_id = msg->id;
    BMS_Message decoded_msg;
    
    /*
    if (BMS_ECHO_MSGS) {
        LV_CAN_SendMessage(msg->id, msg->data, msg->length, msg->priority);
    }
    */

    for (int i = 0; i < DispatchRegisterCount; i++) {
        if ((msg_id & 0xFFF) == (DispatchRegister[i].can_id & 0xFFF)) { // match only bits 0-11 to allow for module offset
            dispatch_register_match_count++;
            if (DispatchRegister[i].decode(msg, &decoded_msg)) {
                DispatchRegister[i].handle(&decoded_msg);
            }
        }
    }

    return HAL_OK;
}

bool DecodeCellTempSummary(const CAN_Message_t *in, BMS_Message *out)
{
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 4U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_TEMP_SUMMARY) return false;

    out->module = MODULE_ID(in->id);
   
    // First 16 bits = min_temp (deci-degC), next 16 bits = max_temp (deci-degC)
    int16_t min_raw = (int16_t)((uint16_t)in->data[0] | ((uint16_t)in->data[1] << 8));
    int16_t max_raw = (int16_t)((uint16_t)in->data[2] | ((uint16_t)in->data[3] << 8));

    out->cell_temps.temp_min = ((float)min_raw) / 10.0f; // 636 -> 63.6 C
    out->cell_temps.temp_max = ((float)max_raw) / 10.0f; // 636 -> 63.6 C

    return true;
}

bool HandleCellTempSummary(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetCellTemps(acc[msg->module], &msg->cell_temps);

    return true;
}

void Acc_SetCellTemps(Acc_Module *acc, const CellTemps *cell_temps){
    osMutexAcquire(acc->mutex, osWaitForever);
    if (cell_temps != NULL) {
        acc->cell_temps = *cell_temps;
    }
    osMutexRelease(acc->mutex);
}

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const void *attr)
{
    (void)msg_count;
    (void)msg_size;
    (void)attr;
    return (osMessageQueueId_t)0x1;
}

HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t active_it)
{
    (void)hcan;
    (void)active_it;
    return HAL_OK;
}

osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    (void)mq_id;
    (void)msg_ptr;
    (void)msg_prio;
    (void)timeout;
    return osOK;
}

uint32_t osKernelGetTickCount(void)
{
    return 0;
}

osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    (void)mq_id;
    (void)msg_ptr;
    (void)msg_prio;
    (void)timeout;
    return osOK;
}

HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t *pData, uint32_t *pTxMailbox)
{
    (void)hcan;
    (void)pHeader;
    (void)pData;
    (void)pTxMailbox;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t fifo, CAN_RxHeaderTypeDef *pHeader, uint8_t *aData)
{
    (void)hcan;
    (void)fifo;
    (void)pHeader;
    (void)aData;

    return HAL_OK;
}

void osDelay(uint32_t ticks)
{
    (void)ticks;
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

HAL_StatusTypeDef LV_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority)
{
    (void)id;
    (void)data;
    (void)length;
    (void)priority;
    return HAL_OK;
}