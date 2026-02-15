#ifndef STM32L4XX_HAL_H
#define STM32L4XX_HAL_H

#include <stdint.h>

typedef enum {
    HAL_OK = 0,
    HAL_ERROR = 1
} HAL_StatusTypeDef;

typedef struct {
    int dummy;
} CAN_HandleTypeDef;

typedef struct {
    uint32_t ExtId;
    uint32_t StdId;
    uint32_t RTR;
    uint32_t IDE;
    uint32_t DLC;
    uint32_t TransmitGlobalTime;
} CAN_TxHeaderTypeDef;

typedef struct {
    uint32_t ExtId;
    uint32_t StdId;
    uint32_t RTR;
    uint32_t IDE;
    uint32_t DLC;
    uint32_t Timestamp;
} CAN_RxHeaderTypeDef;

typedef struct {
    int dummy;
} GPIO_TypeDef;

typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET = 1
} GPIO_PinState;

// Minimal GPIO defines used by main.h in host tests
#define GPIOB ((GPIO_TypeDef*)0x2)
#define GPIO_PIN_3 ((uint16_t)0x0008)

#define CAN_RTR_DATA 0U
#define CAN_ID_EXT 1U
#define DISABLE 0U

#define CAN_IT_RX_FIFO0_MSG_PENDING (1U << 0)
#define CAN_IT_RX_FIFO1_MSG_PENDING (1U << 1)
#define CAN_IT_ERROR (1U << 2)
#define CAN_IT_BUSOFF (1U << 3)

#define CAN_RX_FIFO0 0U
#define CAN_RX_FIFO1 1U

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t active_it);
HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t *pData, uint32_t *pTxMailbox);
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t fifo, CAN_RxHeaderTypeDef *pHeader, uint8_t *aData);

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

#endif /* STM32L4XX_HAL_H */
