/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : can_manager.c
  * @brief          : CAN bus manager implementation
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
#include "bms_can_manager.h"

/* Private variables ---------------------------------------------------------*/
static osMessageQueueId_t BMS_CAN_RxQueueHandle = NULL;
static osMessageQueueId_t BMS_CAN_TxQueueHandle = NULL;
static CAN_Statistics_t bms_can_stats = {0};

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef BMS_CAN_ProcessRXMessage(CAN_Message_t *msg);
static HAL_StatusTypeDef BMS_CAN_TransmitMessage(CAN_Message_t *msg);
// static void CAN_ConfigureFilters(void);

/* Public variables ---------------------------------------------------------*/
uint8_t bms_can_initialized = 0;


/**
  * @brief  Initialize CAN manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BMS_CAN_Manager_Init(void)
{
    // Create message queues
    BMS_CAN_TxQueueHandle = osMessageQueueNew(CAN_TX_QUEUE_SIZE, sizeof(CAN_Message_t), NULL);
    if (BMS_CAN_TxQueueHandle == NULL) {
        return HAL_ERROR;
    }
    BMS_CAN_RxQueueHandle = osMessageQueueNew(CAN_RX_QUEUE_SIZE, sizeof(CAN_Message_t), NULL);
    if (BMS_CAN_RxQueueHandle == NULL) {
        return HAL_ERROR;
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        return HAL_ERROR;
    }
    // NOTE: CAN filter is now configured in MX_CAN1_Init() BEFORE HAL_CAN_Start()
    // This is critical - filters must be configured before starting CAN!
    
    // Activate CAN RX FIFO notifications (CAN must already be started)
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | 
                                              CAN_IT_RX_FIFO1_MSG_PENDING |
                                              CAN_IT_ERROR |
                                              CAN_IT_BUSOFF) != HAL_OK) {
        return HAL_ERROR;
    }
    // Reset statistics
    CAN_ResetStatistics(&bms_can_stats);
    
    bms_can_initialized = 1;
    return HAL_OK;
}

/**
  * @brief  Main CAN manager task
  * @param  argument: Not used
  * @retval None
  */
void BMS_CAN_ManagerTask(void *argument)
{
    CAN_Message_t msg;

    for (;;) {

        // TODO: limit on how many messages we process at a time?
        // Clear RX queue
        while (osMessageQueueGet(BMS_CAN_RxQueueHandle, &msg, NULL, 0) == osOK) {
            BMS_CAN_ProcessRXMessage(&msg);
        }
        // Clear TX queue
        while (osMessageQueueGet(BMS_CAN_TxQueueHandle, &msg, NULL, 0) == osOK) {
            BMS_CAN_TransmitMessage(&msg);
        }
        
        // TODO: Implement CAN bus error handling
        osDelay(10);
    }
}

/**
  * @brief  Send CAN message on BMS bus (non-blocking, queues message)
  * @param  id: CAN message ID (29-bit extended, max 0x1FFFFFFF)
  * @param  data: Pointer to data buffer (up to 8 bytes)
  * @param  length: Data length (0-8 bytes)
  * @param  priority: Message priority (0 = highest, 3 = lowest)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BMS_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority)
{
    CAN_Message_t msg;
    
    // Validate inputs (29-bit extended ID max)
    if (length > 8 || id > 0x1FFFFFFF || data == NULL || length < 0 || priority < 0) {
        return HAL_ERROR;
    }
    
    // Prepare message
    msg.id = id;
    msg.length = length;
    msg.priority = priority;
    msg.timestamp = osKernelGetTickCount();
    memcpy(msg.data, data, length);
    
    // Add to queue (non-blocking with timeout in ms)
    if (osMessageQueuePut(BMS_CAN_TxQueueHandle, &msg, priority, CAN_TX_TIMEOUT_MS) != osOK) {
        bms_can_stats.tx_queue_full_count++;
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

static HAL_StatusTypeDef BMS_CAN_ProcessRXMessage(CAN_Message_t *msg) {
    // TODO: implement RX processing
    return HAL_OK;
}

/**
  * @brief  Transmit single message to CAN hardware
  * @param  msg: Pointer to message structure
  * @retval HAL_StatusTypeDef
  */
static HAL_StatusTypeDef BMS_CAN_TransmitMessage(CAN_Message_t *msg)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    HAL_StatusTypeDef status;
    uint8_t retry_count = 0;
    
    // Configure TX header for extended ID
    TxHeader.ExtId = msg->id;
    TxHeader.StdId = 0;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_EXT;
    TxHeader.DLC = msg->length;
    TxHeader.TransmitGlobalTime = DISABLE;
    
    // Attempt transmission with retries
    while (retry_count < CAN_MAX_RETRIES) {
        status = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, msg->data, &TxMailbox);
        
        if (status == HAL_OK) {
            bms_can_stats.tx_success_count++;
            return HAL_OK;
        }
        
        retry_count++;
        
        // Brief delay before retry (1ms)
        if (retry_count < CAN_MAX_RETRIES) {
            osDelay(1);
        }
    }
    
    // All retries failed
    bms_can_stats.tx_error_count++;
    return HAL_ERROR;
}

/* CAN Interrupt Callbacks ------------------------------------------------*/

/**
  * @brief  CAN RX FIFO 0 message pending callback
  * @param  hcan: pointer to CAN handle
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    CAN_Message_t msg;
    
    // Get message from FIFO
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, msg.data) == HAL_OK) {
        // Store message details (all messages are extended ID)
        msg.id = RxHeader.ExtId;
        msg.length = RxHeader.DLC;
        msg.priority = 0;  // RX messages don't have priority
        msg.timestamp = osKernelGetTickCount();
        
        // TODO: implement message filtering
        // Add to RX queue (from ISR context)
        if (osMessageQueuePut(BMS_CAN_RxQueueHandle, &msg, 0, 0) != osOK) {
            bms_can_stats.rx_queue_full_count++;
        }
    }
}

/**
  * @brief  CAN RX FIFO 1 message pending callback
  * @param  hcan: pointer to CAN handle
  * @retval None
  */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    CAN_Message_t msg;

    // Get message from FIFO
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, msg.data) == HAL_OK) {
        // Store message details (all messages are extended ID)
        msg.id = RxHeader.ExtId;
        msg.length = RxHeader.DLC;
        msg.priority = 0;  // RX messages don't have priority
        msg.timestamp = osKernelGetTickCount();
        
        // Add to RX queue (from ISR context)
        if (osMessageQueuePut(BMS_CAN_RxQueueHandle, &msg, 0, 0) != osOK) {
            bms_can_stats.rx_queue_full_count++;
        }

    }
}


