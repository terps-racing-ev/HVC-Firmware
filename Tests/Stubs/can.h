#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include "main.h"

typedef struct {
    int dummy;
} CAN_HandleTypeDef;

typedef struct {
    uint32_t StdId;
    uint32_t ExtId;
    uint32_t IDE;
    uint32_t RTR;
    uint32_t DLC;
    uint32_t TransmitGlobalTime;
} CAN_TxHeaderTypeDef;

typedef struct {
    uint32_t ExtId;
    uint32_t DLC;
} CAN_RxHeaderTypeDef;

typedef enum {
    HAL_OK = 0,
    HAL_ERROR = 1
} HAL_StatusTypeDef;

#define CAN_RTR_DATA 0U
#define CAN_ID_EXT 1U

#define CAN_IT_RX_FIFO0_MSG_PENDING 0x0001U
#define CAN_IT_RX_FIFO1_MSG_PENDING 0x0002U
#define CAN_IT_ERROR 0x0004U
#define CAN_IT_BUSOFF 0x0008U

#define CAN_RX_FIFO0 0U
#define CAN_RX_FIFO1 1U

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs);
HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t *aData, uint32_t *pTxMailbox);
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *pHeader, uint8_t aData[]);

#endif
