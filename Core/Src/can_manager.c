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
#include "can_manager.h"

/* Private variables ---------------------------------------------------------*/
osMessageQueueId_t CAN_BMS_TxQueueHandle = NULL;
osMessageQueueId_t CAN_BMS_RxQueueHandle = NULL;

osMessageQueueId_t CAN_CTRL_TxQueueHandle = NULL;
osMessageQueueId_t CAN_CTRL_RxQueueHandle = NULL;

static CAN_Statistics_t can_stats = {0};

/* Private function prototypes -----------------------------------------------*/
// static void CAN_ProcessTxQueue(void);
// static void CAN_ProcessRxMessage(CAN_Message_t *msg);


static HAL_StatusTypeDef CAN_TransmitMessage(CAN_Message_t *msg);
// static void CAN_ConfigureFilters(void);

/**
  * @brief  Initialize CAN manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef CAN_Manager_Init(void)
{
    // // Create TX message queues
    CAN_BMS_TxQueueHandle = osMessageQueueNew(CAN_TX_QUEUE_SIZE, sizeof(CAN_Message_t), NULL);
    if (CAN_BMS_TxQueueHandle == NULL) {
        return HAL_ERROR;
    }
    CAN_CTRL_TxQueueHandle = osMessageQueueNew(CAN_TX_QUEUE_SIZE, sizeof(CAN_Message_t), NULL);
    if (CAN_CTRL_TxQueueHandle == NULL) {
        return HAL_ERROR;
    }
    
    // Create RX message queues
    CAN_BMS_RxQueueHandle = osMessageQueueNew(CAN_RX_QUEUE_SIZE, sizeof(CAN_Message_t), NULL);
    if (CAN_BMS_RxQueueHandle == NULL) {
        return HAL_ERROR;
    }
    // Create RX message queues
    CAN_CTRL_RxQueueHandle = osMessageQueueNew(CAN_RX_QUEUE_SIZE, sizeof(CAN_Message_t), NULL);
    if (CAN_CTRL_RxQueueHandle == NULL) {
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
    // CAN_ResetStatistics();
    
    return HAL_OK;
}

/**
  * @brief  Main CAN manager task
  * @param  argument: Not used
  * @retval None
  */
void CAN_ManagerTask(void *argument)
{
    CAN_Message_t rx_msg;
    rx_msg.id = 0x000;
    rx_msg.length = 4;
    rx_msg.data[0] = 0xF1;
    rx_msg.data[1] = 0xFF;
    rx_msg.data[2] = 0xFF;
    rx_msg.data[3] = 0xFF;
    can_frame frame;
    frame.can_id = 0x000;
    frame.can_dlc = 4;
    frame.data[0] = 0xFF;
    frame.data[1] = 0xFF;
    frame.data[2] = 0xFF;
    frame.data[3] = 0xFF;

    for (;;) {

        // if (ret) {
        //     HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
        // }

        // Blink LED while getting messages from BMS CAN
        // if (osMessageQueueGet(CAN_CTRL_RxQueueHandle, &rx_msg, NULL, 0) == osOK) {
        //     HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
        // }
        osDelay(100);
    }
}

/**
  * @brief  Transmit single message to CAN hardware
  * @param  msg: Pointer to message structure
  * @retval HAL_StatusTypeDef
  */
static HAL_StatusTypeDef CAN_TransmitMessage(CAN_Message_t *msg)
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
            can_stats.tx_success_count++;
            return HAL_OK;
        }
        
        retry_count++;
        
        // Brief delay before retry (1ms)
        if (retry_count < CAN_MAX_RETRIES) {
            osDelay(1);
        }
    }
    
    // All retries failed
    can_stats.tx_error_count++;
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
        
        // // Ignore all messages not for this module
        // if (CAN_IsMessageForThisModule(msg.id)) {
        //         // Add to RX queue (from ISR context)
        //     if (osMessageQueuePut(CANRxQueueHandle, &msg, 0, 0) != osOK) {
        //         can_stats.rx_queue_full_count++;
        //     }
        // }
        // HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
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


    // // Get message from FIFO
    // if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, msg.data) == HAL_OK) {
    //     // Store message details (all messages are extended ID)
    //     msg.id = RxHeader.ExtId;
    //     msg.length = RxHeader.DLC;
    //     msg.priority = 0;  // RX messages don't have priority
    //     msg.timestamp = osKernelGetTickCount();
        
    //     // Ignore all messages not for this module
    //     if (CAN_IsMessageForThisModule(msg.id)) {
    //         // Add to RX queue (from ISR context)
    //         if (osMessageQueuePut(CAN_BMS_RxQueueHandle, &msg, 0, 0) != osOK) {
    //             can_stats.rx_queue_full_count++;
    //         }
    //     }

    // }
}

/**
  * @brief  SPI Int pending callback
  * @param  argument: Not used
  * @retval None
  */
void SPICANIntCallbackTask(void *argument)
{   
    uCAN_MSG rx_msg;

    
    while (CANSPI_Receive(&rx_msg)) {
        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
    }
    
    for (;;) {
        // osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
        
        // osThreadFlagsClear(0x0001);
        if (CANSPI_isRxErrorPassive()) {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        }
        
        while (CANSPI_Receive(&rx_msg)) {
            // CANSPI_Transmit(&rx_msg);
            HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
        }

        osDelay(50);
    }
}


