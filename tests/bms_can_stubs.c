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
    BMS_Message decoded_msg;
    
    if (msg == NULL){
        return HAL_ERROR;
    }

    if (BMS_ECHO_MSGS) {
        LV_CAN_SendMessage(msg->id, msg->data, msg->length, msg->priority);
    }

    for (int i = 0; i < DispatchRegisterCount; i++) {
        // Decode returns true only when message is for it
        if (DispatchRegister[i].decode(msg, &decoded_msg)) {
            dispatch_register_match_count++;
            DispatchRegister[i].handle(&decoded_msg);
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

bool DecodeAmbientTemps(const CAN_Message_t *in, BMS_Message *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 8U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_TEMP + AMBIENT_TEMP_MSG_INDEX) return false;

    out->module = MODULE_ID(in->id);
   
    // Ambient temp 1 in bytes 4-5, ambient temp 2 in bytes 6-7 (0.1C)
    int16_t temp_1_raw = (int16_t)((uint16_t)in->data[4] | ((uint16_t)in->data[5] << 8));
    int16_t temp_2_raw = (int16_t)((uint16_t)in->data[6] | ((uint16_t)in->data[7] << 8));

    out->amb_temps.amb_temp_1 = ((float)temp_1_raw) / 10.0f;
    out->amb_temps.amb_temp_2 = ((float)temp_2_raw) / 10.0f;

    return true;
}

bool HandleAmbientTemps(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetAmbientTemps(acc[msg->module], &msg->amb_temps);

    return true;
}

bool DecodeCellVoltages(const CAN_Message_t *in, BMS_Message *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 6U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (REMOVE_VOLTAGE_MSG_INDEX(cmd) != BMB_CAN_VOLTAGE_BASE) return false;

    out->module = MODULE_ID(in->id);
   
    Acc_GetCellVoltages(acc[out->module], &out->cell_voltages);

    uint8_t cell_id[CELLS_PER_VOLTAGE_MSG];
    uint16_t volt[CELLS_PER_VOLTAGE_MSG];
    
    for (int i = 0; i < CELLS_PER_VOLTAGE_MSG; i++) {
        // cells 1-3 in message 0, 4-6 in message 1, 7-9 in message 2, and so on
        cell_id[i] = CELLS_PER_VOLTAGE_MSG * VOLTAGE_MSG_INDEX(cmd) + i + 1;
        // first voltage in bytes 0-1, second voltage in bytes 2-3, third voltage in bytes 4-5 (mV)
        volt[i] = (uint16_t)in->data[2 * i] | ((uint16_t)in->data[2 * i + 1] << 8);
        
        if ((out->cell_voltages.volt_min == 0) || (volt[i] < out->cell_voltages.volt_min)) {
            out->cell_voltages.volt_min_cell_id = cell_id[i];
            out->cell_voltages.volt_min = volt[i];
        } else if (volt[i] > out->cell_voltages.volt_max) {
            out->cell_voltages.volt_max_cell_id = cell_id[i];
            out->cell_voltages.volt_max = volt[i];
        }
    }

    return true;
}

bool HandleCellVoltages(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetCellVoltages(acc[msg->module], &msg->cell_voltages);

    return true;
}

bool DecodeBMSHeartbeat(const CAN_Message_t *in, BMS_Message *out){
    uint32_t cmd;

    if (in == NULL || out == NULL) return false;
    if (in->length < 8U) return false;
    
    cmd = REMOVE_MODULE_ID(in->id);
    if (cmd != BMB_CAN_BMS_HEARTBEAT) return false;

    out->module = MODULE_ID(in->id);
   
    out->heartbeat_timestamp = in->timestamp;

    return true;
}

bool HandleBMSHeartbeat(const BMS_Message *msg){
    if (msg == NULL) return false;
    if (msg->module >= 6U) return false;
    
    Acc_SetHeartbeatLastUpdate(acc[msg->module], &msg->heartbeat_timestamp);

    return true;
}

void Acc_GetCellVoltages(Acc_Module *module, CellVoltages *cell_voltages){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        *cell_voltages = module->cell_voltages;
    }
    osMutexRelease(module->mutex);
}

void Acc_SetHeartbeatLastUpdate(Acc_Module *module, uint32_t* last_update){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (last_update != NULL) {
        module->heartbeat_last_update = *last_update;
    }
    osMutexRelease(module->mutex);
}

void Acc_SetCellVoltages(Acc_Module *module, const CellVoltages *cell_voltages){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        module->cell_voltages = *cell_voltages;
    }
    osMutexRelease(module->mutex);
}

void Acc_SetCellTemps(Acc_Module *module, const CellTemps *cell_temps){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_temps != NULL) {
        module->cell_temps = *cell_temps;
    }
    osMutexRelease(module->mutex);
}

void Acc_SetAmbientTemps(Acc_Module *module, const AmbientTemps *amb_temps){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (amb_temps != NULL) {
        module->amb_temps = *amb_temps;
    }
    osMutexRelease(module->mutex);
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