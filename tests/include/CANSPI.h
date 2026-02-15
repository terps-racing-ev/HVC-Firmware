#ifndef CAN_SPI_H
#define CAN_SPI_H

#include <stdint.h>

typedef struct {
    uint8_t data[8];
} uCAN_MSG;

typedef struct {
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[8];
} can_frame;

int CANSPI_Receive(uCAN_MSG *msg);
int CANSPI_isRxErrorPassive(void);

#endif /* CAN_SPI_H */
