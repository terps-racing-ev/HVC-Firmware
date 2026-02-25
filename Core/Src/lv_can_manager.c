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
#include "spi_can.h"

/* Private variables ---------------------------------------------------------*/
static CAN_Statistics_t lv_can_stats = {0};
osMessageQueueId_t LV_CAN_RxQueueHandle = NULL;

/* Public variables ---------------------------------------------------------*/
osMessageQueueId_t LV_CAN_TxQueueHandle = NULL;

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

    LV_CAN_RxQueueHandle = osMessageQueueNew(
        CAN_RX_QUEUE_SIZE,
        sizeof(uCAN_MSG),
        NULL
    );
    if (LV_CAN_RxQueueHandle == NULL) {
        return HAL_ERROR;
    }

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
    while (osMessageQueueGet(LV_CAN_RxQueueHandle, &rx_message, NULL, 0) == osOK) {
        lv_can_stats.rx_message_count++;
    }

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
  * @brief  SPI Int pending callback
  * @param  argument: Not used
  * @retval None
  */
void SPI_CAN_Int_CallbackTask(void *argument)
{   
    uCAN_MSG rx_msg;
    
    // Clear out queue so the SPI interrupt gets cleared at start
    while (CANSPI_Receive(&rx_msg)) {
        if (osMessageQueuePut(LV_CAN_TxQueueHandle, &rx_msg, 0, 0) != osOK) {
            lv_can_stats.rx_queue_full_count++;
        }
    }

    for (;;) {
        osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
        
        osThreadFlagsClear(0x0001);
        if (CANSPI_isRxErrorPassive()) {
            // TODO: handle errors
            // HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        }
        
        while (CANSPI_Receive(&rx_msg)) {
            // TODO: check if message is for HVC
            if (osMessageQueuePut(LV_CAN_TxQueueHandle, &rx_msg, 0, 0) != osOK) {
                lv_can_stats.rx_queue_full_count++;
            }
        }

        osDelay(50);
    }
}


