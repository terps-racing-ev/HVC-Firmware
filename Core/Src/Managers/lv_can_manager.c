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
static osMessageQueueId_t LV_CAN_TxQueueHandle = NULL;
static CAN_Statistics_t lv_can_stats = {0};

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef LV_CAN_TransmitMessage(CAN_Message_t *msg);

/* Public variables ---------------------------------------------------------*/

uint8_t lv_can_initialized = 0;

/**
  * @brief  Initialize LV CAN manager
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef LV_CAN_Manager_Init(void) {
    
    // Create message queues
    LV_CAN_TxQueueHandle = osMessageQueueNew(
        CAN_TX_QUEUE_SIZE,
        sizeof(CAN_Message_t),
        NULL
    );
    if (LV_CAN_TxQueueHandle == NULL) {
        return HAL_ERROR;
    }

    if (!CANSPI_Initialize()) {
        return HAL_ERROR;
    }

    CAN_ResetStatistics(&lv_can_stats);

    lv_can_initialized = 1;
    return HAL_OK;
}

/**
  * @brief  Main LV CAN manager task
  * @param  argument: Not used
  * @retval None
  */
void LV_CAN_ManagerTask(void *argument){
  CAN_Message_t rx_message;
  CAN_Message_t tx_message;

  for (;;) {

    // No RX on LV bus

    while (osMessageQueueGet(LV_CAN_TxQueueHandle, &tx_message, NULL, 0) == osOK) {
        // Converting to uCAN_MSG type
        uCAN_MSG spi_message = {0};
        uint8_t attempt = 0;
        uint8_t tx_success = 0;

        spi_message.frame.idType = dEXTENDED_CAN_MSG_ID_2_0B;
        spi_message.frame.id = tx_message.id;
        spi_message.frame.dlc = (tx_message.length <= 8) ? tx_message.length : 8;
        spi_message.frame.data0 = tx_message.data[0];
        spi_message.frame.data1 = tx_message.data[1];
        spi_message.frame.data2 = tx_message.data[2];
        spi_message.frame.data3 = tx_message.data[3];
        spi_message.frame.data4 = tx_message.data[4];
        spi_message.frame.data5 = tx_message.data[5];
        spi_message.frame.data6 = tx_message.data[6];
        spi_message.frame.data7 = tx_message.data[7];

        while (attempt < CAN_MAX_RETRIES) {
            if (CANSPI_Transmit(&spi_message)) {
                lv_can_stats.tx_success_count++;
                tx_success = true;
                break;
            }
            attempt++;
            osDelay(1);
        }

        if (tx_success != true) {
            lv_can_stats.tx_error_count++;
        }
    }

    osDelay(1);

    // TODO: handle bus errors (bus off, tx passive, rx passive)
    }
}


// TODO: move this into LV_CAN_ManagerTask
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
    CAN_Message_t msg;
    
    // Validate inputs
    if ((data == NULL) ||
        (length > CAN_MAX_DLEN) ||
        (id > CAN_EFF_MASK) ||
        (priority > CAN_PRIORITY_LOW)) {
        return HAL_ERROR;
    }
    
    // Prepare message
    msg.id = id;
    msg.length = length;
    msg.priority = priority;
    msg.timestamp = osKernelGetTickCount();
    
    // Copy data
    if (data != NULL && length > 0) {
        for (int i = 0; i < length; i++) {
            msg.data[i] = data[i];
        }
    }
    
    // Add to queue (non-blocking with timeout in ms)
    if (osMessageQueuePut(LV_CAN_TxQueueHandle, &msg, priority, CAN_TX_TIMEOUT_MS) != osOK) {
        lv_can_stats.tx_queue_full_count++;
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

static HAL_StatusTypeDef LV_CAN_TransmitMessage(CAN_Message_t *msg) {
    uCAN_MSG rx_msg;
    uint8_t retry_count = 0;
    uint8_t status = 0;
    
    // Check parameters
    if ((msg == NULL) ||
        (msg->length > CAN_MAX_DLEN) ||
        (msg->id > CAN_EFF_MASK)) {
        return HAL_ERROR;
    }
    // Convert to spi_can type
    rx_msg.frame.idType = (uint8_t) dEXTENDED_CAN_MSG_ID_2_0B; // Extended ID
    rx_msg.frame.id = msg->id & 0x1FFFFFFFUL; // Filter out high bits of extended id
    rx_msg.frame.dlc = msg->length;
    // TODO: UGLY, use the array to make this prettier
    rx_msg.frame.data0 = msg->data[0];
    rx_msg.frame.data1 = msg->data[1];
    rx_msg.frame.data2 = msg->data[2];
    rx_msg.frame.data3 = msg->data[3];
    rx_msg.frame.data4 = msg->data[4];
    rx_msg.frame.data5 = msg->data[5];
    rx_msg.frame.data6 = msg->data[6];
    rx_msg.frame.data7 = msg->data[7];

    // Retry a couple times before giving up
    while (retry_count < CAN_MAX_RETRIES) {
        status = CANSPI_Transmit(&rx_msg);

        if (status != 0) {
            lv_can_stats.tx_success_count++;
            return HAL_OK;
        }

        retry_count++;
        if (retry_count < CAN_MAX_RETRIES) {
            osDelay(1); // Wait a bit before retrying
        }
    }

    // All retries failed
    lv_can_stats.tx_error_count++;
    return HAL_ERROR;
}