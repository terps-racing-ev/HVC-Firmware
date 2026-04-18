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
HAL_StatusTypeDef LV_CAN_TransmitMessage(CAN_Message_t *msg);
HAL_StatusTypeDef LV_CAN_ProcessRXMessage(CAN_Message_t *msg);
static void _CONVERT_UCAN_TO_CAN_MESSAGE(uCAN_MSG *in, CAN_Message_t *out);
static void _CONVERT_CAN_MESSAGE_TO_UCAN(CAN_Message_t *in, uCAN_MSG *out);

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
    CAN_Message_t can_message;
  
    for (;;) {

        while (osMessageQueueGet(LV_CAN_TxQueueHandle, &can_message, NULL, 0) == osOK) {
            LV_CAN_TransmitMessage(&can_message);
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

HAL_StatusTypeDef LV_CAN_TransmitMessage(CAN_Message_t *msg) {
    uCAN_MSG rx_msg;
    uint8_t retry_count = 0;
    uint8_t status = 0;
    
    // Check parameters
    if ((msg == NULL) ||
        (msg->length > CAN_MAX_DLEN) ||
        (msg->id > CAN_EFF_MASK)) {
        return HAL_ERROR;
    }

    _CONVERT_CAN_MESSAGE_TO_UCAN(msg, &rx_msg);

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

HAL_StatusTypeDef LV_CAN_ProcessRXMessage(CAN_Message_t *msg) {
    LV_Message_t decoded_msg;
    
    if (msg == NULL){
        return HAL_ERROR;
    }

    for (int i = 0; i < LV_DispatchRegisterCount; i++) {
        // Decode returns true only when message is for it
        if (LV_DispatchRegister[i].decode(msg, &decoded_msg)) {
            LV_DispatchRegister[i].handle(&decoded_msg);
        }
    }

    return HAL_OK;
}

/**
  * @brief  SPI Int pending callback
  * @param  argument: Not used
  * @retval None
  */
void SPI_IntCallbackTask(void *argument)
{   
    uCAN_MSG rx_msg;
    CAN_Message_t msg;

    // Clear out queue so the SPI interrupt gets cleared at start
    while (CANSPI_Receive(&rx_msg)) {
        _CONVERT_UCAN_TO_CAN_MESSAGE(&rx_msg, &msg);
        LV_CAN_ProcessRXMessage(&msg);
    }

    for (;;) {
        osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(0x0001);
        
        if (CANSPI_isRxErrorPassive()) {
            // TODO: handle errors
            // HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        }

        while (CANSPI_Receive(&rx_msg)) {
            _CONVERT_UCAN_TO_CAN_MESSAGE(&rx_msg, &msg);
            LV_CAN_ProcessRXMessage(&msg);
        }

        osDelay(50);
    }
}

static void _CONVERT_CAN_MESSAGE_TO_UCAN(CAN_Message_t *in, uCAN_MSG *out) {
        out->frame.idType = dEXTENDED_CAN_MSG_ID_2_0B;
        out->frame.id = in->id;
        out->frame.dlc = (in->length <= 8) ? in->length : 8;
        out->frame.data0 = in->data[0];
        out->frame.data1 = in->data[1];
        out->frame.data2 = in->data[2];
        out->frame.data3 = in->data[3];
        out->frame.data4 = in->data[4];
        out->frame.data5 = in->data[5];
        out->frame.data6 = in->data[6];
        out->frame.data7 = in->data[7];
}

static void _CONVERT_UCAN_TO_CAN_MESSAGE(uCAN_MSG *in, CAN_Message_t *out) {
    if ((in == NULL) || (out == NULL)) {
        return;
    }

    out->id = in->frame.id;
    out->length = (in->frame.dlc <= CAN_MAX_DLEN) ? in->frame.dlc : CAN_MAX_DLEN;
    out->data[0] = in->frame.data0;
    out->data[1] = in->frame.data1;
    out->data[2] = in->frame.data2;
    out->data[3] = in->frame.data3;
    out->data[4] = in->frame.data4;
    out->data[5] = in->frame.data5;
    out->data[6] = in->frame.data6;
    out->data[7] = in->frame.data7;
    out->priority = CAN_PRIORITY_NORMAL;
    out->timestamp = osKernelGetTickCount();
}