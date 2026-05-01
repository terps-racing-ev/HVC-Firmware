/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lv_can_manager.c
  * @brief          : LV CAN bus manager implementation
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "lv_can_manager.h"

/* Private variables ---------------------------------------------------------*/
static osMessageQueueId_t LV_CAN_RxQueueHandle = NULL;
static osMessageQueueId_t LV_CAN_TxQueueHandle = NULL;
static CAN_Statistics_t lv_can_stats = {0};
static volatile uint8_t lv_can_bus_off_pending = 0;
static volatile uint8_t lv_can_error_passive_pending = 0;
static volatile uint32_t lv_can_last_error_tick = 0xFFFFFFFFU;
static uint32_t lv_can_recovery_backoff_ms = CAN_RECOVERY_BACKOFF_BASE_MS;

/* Private function prototypes -----------------------------------------------*/
HAL_StatusTypeDef LV_CAN_TransmitMessage(uCAN_MSG *msg);
HAL_StatusTypeDef LV_CAN_ProcessRXMessage(uCAN_MSG *msg);
static HAL_StatusTypeDef LV_CAN_QueueRXMessage(const uCAN_MSG *msg);
static HAL_StatusTypeDef LV_CAN_RecoverBus(void);
static void LV_CAN_HandleBusErrors(void);
static void LV_CAN_MarkError(void);

/* Public variables ---------------------------------------------------------*/

uint8_t lv_can_initialized = 0;

/**
  * @brief  Initialize LV CAN manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef LV_CAN_Manager_Init(void) {
    
    // Create message queues
    LV_CAN_RxQueueHandle = osMessageQueueNew(
        CAN_RX_QUEUE_SIZE,
        sizeof(uCAN_MSG),
        NULL
    );
    if (LV_CAN_RxQueueHandle == NULL) {
        return HAL_ERROR;
    }

    LV_CAN_TxQueueHandle = osMessageQueueNew(
        CAN_TX_QUEUE_SIZE,
        sizeof(uCAN_MSG),
        NULL
    );
    if (LV_CAN_TxQueueHandle == NULL) {
        return HAL_ERROR;
    }

    if (!CANSPI_Initialize()) {
        return HAL_ERROR;
    }

    CAN_ResetStatistics(&lv_can_stats);
    lv_can_bus_off_pending = 0;
    lv_can_error_passive_pending = 0;
    lv_can_last_error_tick = 0xFFFFFFFFU;
    lv_can_recovery_backoff_ms = CAN_RECOVERY_BACKOFF_BASE_MS;

    lv_can_initialized = 1;
    return HAL_OK;
}

/**
  * @brief  Main LV CAN manager task
  * @param  argument: Not used
  * @retval None
  */
void LV_CAN_ManagerTask(void *argument){
    uCAN_MSG msg;
    uint32_t rx_count;
    uint32_t tx_count;
  
    for (;;) {
        rx_count = 0;
        tx_count = 0;

        while ((rx_count < CAN_TASK_MAX_RX_PER_CYCLE) &&
               (osMessageQueueGet(LV_CAN_RxQueueHandle, &msg, NULL, 0) == osOK)) {
            LV_CAN_ProcessRXMessage(&msg);
            rx_count++;
        }

        while ((tx_count < CAN_TASK_MAX_TX_PER_CYCLE) &&
               (osMessageQueueGet(LV_CAN_TxQueueHandle, &msg, NULL, 0) == osOK)) {
            LV_CAN_TransmitMessage(&msg);
            tx_count++;
        }

        LV_CAN_HandleBusErrors();

        osDelay(1);
    }
}

/**
  * @brief  Send CAN message on LV bus (non-blocking, queues message)
  * @param  id: CAN message ID (29-bit extended, max 0x1FFFFFFF)
  * @param  data: Pointer to data buffer (up to 8 bytes)
  * @param  length: Data length (0-8 bytes)
  * @param  priority: Message priority (0 = highest, 3 = lowest)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef LV_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority)
{
    uCAN_MSG msg;
    
    // Validate inputs
    if (((data == NULL) && (length > 0)) ||
        (length > CAN_MAX_DLEN) ||
        (id > CAN_EFF_MASK) ||
        (priority > CAN_PRIORITY_LOW)) {
        return HAL_ERROR;
    }

    #ifdef DEBUG
    
    BMS_CAN_SendMessage(id, data, length, priority);

    #endif
    
    // Prepare message
    msg.frame.idType = dEXTENDED_CAN_MSG_ID_2_0B;
    msg.frame.id = id;
    msg.frame.dlc = length;
    msg.frame.data0 = (length > 0) ? data[0] : 0;
    msg.frame.data1 = (length > 1) ? data[1] : 0;
    msg.frame.data2 = (length > 2) ? data[2] : 0;
    msg.frame.data3 = (length > 3) ? data[3] : 0;
    msg.frame.data4 = (length > 4) ? data[4] : 0;
    msg.frame.data5 = (length > 5) ? data[5] : 0;
    msg.frame.data6 = (length > 6) ? data[6] : 0;
    msg.frame.data7 = (length > 7) ? data[7] : 0;
    
    // Add to queue (non-blocking with timeout in ms)
    if (osMessageQueuePut(LV_CAN_TxQueueHandle, &msg, priority, CAN_TX_TIMEOUT_MS) != osOK) {
        lv_can_stats.tx_queue_full_count++;
        LV_CAN_MarkError();
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

bool LV_CAN_HasError(void)
{
    uint32_t now;

    if (lv_can_bus_off_pending || lv_can_error_passive_pending) {
        return true;
    }

    if (lv_can_last_error_tick == 0xFFFFFFFFU) {
        return false;
    }

    now = osKernelGetTickCount();
    return ((now - lv_can_last_error_tick) <= CAN_ERROR_HOLD_TICKS);
}

static void LV_CAN_MarkError(void)
{
    lv_can_last_error_tick = osKernelGetTickCount();
}

HAL_StatusTypeDef LV_CAN_TransmitMessage(uCAN_MSG *msg) {
    uint8_t retry_count = 0;
    uint8_t status = 0;
    
    // Check parameters
    if ((msg == NULL) ||
        (msg->frame.dlc > CAN_MAX_DLEN) ||
        (msg->frame.id > CAN_EFF_MASK)) {
        LV_CAN_MarkError();
        return HAL_ERROR;
    }

    // Retry a couple times before giving up
    while (retry_count < CAN_MAX_RETRIES) {
        status = CANSPI_Transmit(msg);

        if (status != 0) {
            lv_can_stats.tx_success_count++;
            return HAL_OK;
        }

        retry_count++;

        if (CANSPI_isBussOff()) {
            lv_can_stats.bus_off_count++;
            lv_can_bus_off_pending = 1;
            LV_CAN_MarkError();
            break;
        }

        if (CANSPI_isTxErrorPassive()) {
            lv_can_error_passive_pending = 1;
            LV_CAN_MarkError();
        }

        if (retry_count < CAN_MAX_RETRIES) {
            osDelay(1); // Wait a bit before retrying
        }
    }

    // All retries failed
    lv_can_stats.tx_error_count++;
    LV_CAN_MarkError();
    return HAL_ERROR;
}

HAL_StatusTypeDef LV_CAN_ProcessRXMessage(uCAN_MSG *msg) {
    if (msg == NULL){
        return HAL_ERROR;
    }

    if ((msg->frame.idType != dEXTENDED_CAN_MSG_ID_2_0B) ||
        (msg->frame.dlc > CAN_MAX_DLEN)) {
        lv_can_stats.rx_invalid_count++;
        LV_CAN_MarkError();
        return HAL_ERROR;
    }

    for (int i = 0; i < LV_DispatchRegisterCount; i++) {
        // Decode returns true only when message is for it
        if (LV_DispatchRegister[i].decode(msg)) {
            LV_DispatchRegister[i].handle(msg);
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef LV_CAN_QueueRXMessage(const uCAN_MSG *msg)
{
    if ((msg == NULL) ||
        (LV_CAN_RxQueueHandle == NULL)) {
        return HAL_ERROR;
    }

    if ((msg->frame.idType != dEXTENDED_CAN_MSG_ID_2_0B) ||
        (msg->frame.dlc > CAN_MAX_DLEN)) {
        lv_can_stats.rx_invalid_count++;
        LV_CAN_MarkError();
        return HAL_ERROR;
    }

    if (osMessageQueuePut(LV_CAN_RxQueueHandle, msg, 0, 0) != osOK) {
        lv_can_stats.rx_queue_full_count++;
        LV_CAN_MarkError();
        return HAL_ERROR;
    }

    lv_can_stats.rx_message_count++;
    return HAL_OK;
}

static HAL_StatusTypeDef LV_CAN_RecoverBus(void)
{
    if (lv_can_recovery_backoff_ms > 0) {
        osDelay(lv_can_recovery_backoff_ms);
    }

    if (!CANSPI_Initialize()) {
        LV_CAN_MarkError();
        if (lv_can_recovery_backoff_ms < CAN_RECOVERY_BACKOFF_MAX_MS) {
            lv_can_recovery_backoff_ms <<= 1;
            if (lv_can_recovery_backoff_ms > CAN_RECOVERY_BACKOFF_MAX_MS) {
                lv_can_recovery_backoff_ms = CAN_RECOVERY_BACKOFF_MAX_MS;
            }
        }
        return HAL_ERROR;
    }

    lv_can_bus_off_pending = 0;
    lv_can_recovery_backoff_ms = CAN_RECOVERY_BACKOFF_BASE_MS;
    lv_can_stats.recovery_count++;
    return HAL_OK;
}

static void LV_CAN_HandleBusErrors(void)
{
    if (CANSPI_isBussOff()) {
        lv_can_stats.bus_off_count++;
        lv_can_bus_off_pending = 1;
        LV_CAN_MarkError();
    }

    if (CANSPI_isRxErrorPassive() || CANSPI_isTxErrorPassive()) {
        lv_can_error_passive_pending = 1;
        LV_CAN_MarkError();
    }

    if (lv_can_error_passive_pending) {
        lv_can_stats.error_passive_count++;
        lv_can_error_passive_pending = 0;
    }

    if (lv_can_bus_off_pending) {
        LV_CAN_RecoverBus();
    }
}

/**
  * @brief  SPI Int pending callback
  * @param  argument: Not used
  * @retval None
  */
void SPI_IntCallbackTask(void *argument)
{   
    uCAN_MSG rx_msg;

    // Clear out queue so the SPI interrupt gets cleared at start
    while (CANSPI_Receive(&rx_msg)) {
        LV_CAN_QueueRXMessage(&rx_msg);
    }

    for (;;) {
        osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(0x0001);
        
        if (CANSPI_isBussOff()) {
            lv_can_stats.bus_off_count++;
            lv_can_bus_off_pending = 1;
            LV_CAN_MarkError();
        }

        if (CANSPI_isRxErrorPassive() || CANSPI_isTxErrorPassive()) {
            lv_can_error_passive_pending = 1;
            LV_CAN_MarkError();
        }

        while (CANSPI_Receive(&rx_msg)) {
            LV_CAN_QueueRXMessage(&rx_msg);
        }

        osDelay(50);
    }
}
