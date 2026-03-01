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
    int dummy;
} ADC_HandleTypeDef;

typedef struct {
    int dummy;
} COMP_HandleTypeDef;

typedef struct {
    uint32_t Channel;
    uint32_t Rank;
    uint32_t SamplingTime;
    uint32_t SingleDiff;
    uint32_t OffsetNumber;
    uint32_t Offset;
} ADC_ChannelConfTypeDef;

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
#define GPIOA ((GPIO_TypeDef*)0x1)
#define GPIOB ((GPIO_TypeDef*)0x2)
#define GPIO_PIN_0 ((uint16_t)0x0001)
#define GPIO_PIN_1 ((uint16_t)0x0002)
#define GPIO_PIN_2 ((uint16_t)0x0004)
#define GPIO_PIN_3 ((uint16_t)0x0008)
#define GPIO_PIN_4 ((uint16_t)0x0010)
#define GPIO_PIN_5 ((uint16_t)0x0020)
#define GPIO_PIN_6 ((uint16_t)0x0040)
#define GPIO_PIN_7 ((uint16_t)0x0080)
#define GPIO_PIN_8 ((uint16_t)0x0100)
#define GPIO_PIN_9 ((uint16_t)0x0200)
#define GPIO_PIN_10 ((uint16_t)0x0400)

#define CAN_RTR_DATA 0U
#define CAN_ID_EXT 1U
#define DISABLE 0U

#define CAN_IT_RX_FIFO0_MSG_PENDING (1U << 0)
#define CAN_IT_RX_FIFO1_MSG_PENDING (1U << 1)
#define CAN_IT_ERROR (1U << 2)
#define CAN_IT_BUSOFF (1U << 3)

#define CAN_RX_FIFO0 0U
#define CAN_RX_FIFO1 1U

#define ADC_REGULAR_RANK_1 1U
#define ADC_SAMPLETIME_2CYCLES_5 0U
#define ADC_SINGLE_ENDED 0U
#define ADC_OFFSET_NONE 0U
#define ADC_CHANNEL_5 5U
#define ADC_CHANNEL_6 6U
#define ADC_CHANNEL_8 8U

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t active_it);
HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t *pData, uint32_t *pTxMailbox);
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t fifo, CAN_RxHeaderTypeDef *pHeader, uint8_t *aData);

HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *sConfig);
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc, uint32_t timeout);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc);

HAL_StatusTypeDef HAL_COMP_Start(COMP_HandleTypeDef *hcomp);

HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan);

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#endif /* STM32L4XX_HAL_H */
