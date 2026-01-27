/*
 * mcp2515.h
 */

#ifndef APPLICATION_USER_INC_MCP2515_H_
#define APPLICATION_USER_INC_MCP2515_H_

#include <stdint.h>
#include "can.h"
#include "mcp2515_consts.h"
#include "cmsis_os.h"
#include "main.h" // Todo: yucky circular dependency

typedef enum {
	ERROR_OK = 0,
	ERROR_FAIL = 1,
	ERROR_ALLTXBUSY = 2,
	ERROR_FAILINIT = 3,
	ERROR_FAILTX = 4,
	ERROR_NOMSG = 5
} CAN_Error;

typedef enum {
	CLKOUT_DISABLE = -1,
	CLKOUT_DIV1 = 0x0,
	CLKOUT_DIV2 = 0x1,
	CLKOUT_DIV4 = 0x2,
	CLKOUT_DIV8 = 0x3,
} CAN_CLKOUT;

typedef enum {
	MCP_20MHZ, MCP_16MHZ, MCP_12MHZ, MCP_8MHZ
} CAN_CLOCK;

typedef enum {
	CAN_5KBPS,
	CAN_10KBPS,
	CAN_20KBPS,
	CAN_31K25BPS,
	CAN_33KBPS,
	CAN_40KBPS,
	CAN_50KBPS,
	CAN_80KBPS,
	CAN_83K3BPS,
	CAN_95KBPS,
	CAN_100KBPS,
	CAN_125KBPS,
	CAN_200KBPS,
	CAN_250KBPS,
	CAN_500KBPS,
	CAN_1000KBPS
} CAN_SPEED;

typedef enum {
	MASK0, MASK1
} MASK;

typedef enum {
	RXF0 = 0, RXF1 = 1, RXF2 = 2, RXF3 = 3, RXF4 = 4, RXF5 = 5
} RXF;

typedef enum {
	TXB0 = 0, TXB1 = 1, TXB2 = 2
} TXBn;

typedef enum {
	RXB0 = 0, RXB1 = 1
} RXBn;


/**
 * @brief Test individual SPI byte transfer.
 *
 * Sends a byte and returns what was received.
 * Use this to diagnose SPI issues:
 * - If always 0x00: likely MISO not connected
 * - If always 0xFF: likely pulled up or no CS working
 * - If echoes back: good sign, but check MISO isolation
 *
 * @param txByte Byte to transmit
 * @return Byte received from SPI
 */
uint8_t MCP_testSPIByte(uint8_t txByte); //

/**
 * @brief Test SPI communication with MCP2515.
 *
 * Reads CANSTAT register to verify SPI is working.
 * Expected value after reset: 0x80 (CONFIG mode)
 * If no chip/bad SPI: likely 0xFF
 *
 * @return CANSTAT register value
 */
uint8_t MCP_testSPIComm(void); //

/**
 * @brief Reset and initialize the MCP2515 controller.
 *
 * Performs a soft reset and basic initialization of filters, masks and
 * interrupt settings.
 */
CAN_Error MCP_reset(void); //
/**
 * @brief Enter listen-only mode (receive-only, no ACKs) on the MCP2515.
 */
CAN_Error MCP_setListenOnlyMode(); //
/**
 * @brief Put the MCP2515 into sleep (low-power) mode.
 */
CAN_Error MCP_setSleepMode(); //
/**
 * @brief Put the MCP2515 into loopback (self-test) mode.
 */
CAN_Error MCP_setLoopbackMode(); //
/**
 * @brief Put the MCP2515 into normal (active) CAN bus operation mode.
 */
CAN_Error MCP_setNormalMode(); //
/**
 * @brief Configure CAN bitrate assuming a 16 MHz MCP clock.
 *
 * Convenience wrapper around MCP_setBitrateClock().
 */
CAN_Error MCP_setBitrate(CAN_SPEED canSpeed); //
/**
 * @brief Configure CAN bitrate for a specific MCP clock source.
 *
 * @param canSpeed Desired CAN bus speed.
 * @param canClock MCP oscillator clock selection.
 */
CAN_Error MCP_setBitrateClock(CAN_SPEED canSpeed, CAN_CLOCK canClock); //
/**
 * @brief Program a filter mask (MASK0 or MASK1).
 *
 * @param num Mask index.
 * @param ext Non-zero for extended ID mask.
 * @param ulData Mask value.
 */
CAN_Error MCP_setFilterMask(MASK num, uint8_t ext, uint32_t ulData); //
/**
 * @brief Program one receive filter (RXF0..RXF5).
 *
 * @param num Filter index.
 * @param ext Non-zero for extended ID filter.
 * @param ulData Filter ID value.
 */
CAN_Error MCP_setFilter(RXF num, uint8_t ext, uint32_t ulData); //
/**
 * @brief Send a CAN frame using a specific transmit buffer.
 *
 * @param txbn Transmit buffer index.
 * @param frame Pointer to can_frame to send.
 */
CAN_Error MCP_sendMessageTo(TXBn txbn, can_frame *frame); //
/**
 * @brief Send a CAN frame using the next available transmit buffer.
 *
 * @param frame Pointer to the can_frame to send.
 */
CAN_Error MCP_sendMessage(can_frame *frame); //
/**
 * @brief Read a CAN frame from a specific receive buffer into @p frame.
 *
 * @param rxbn Receive buffer index.
 * @param frame Pointer to can_frame to receive data.
 */
CAN_Error MCP_readMessageFrom(RXBn rxbn, can_frame *frame); //
/**
 * @brief Read a CAN frame if available (checks RX0/RX1 flags).
 *
 * @param frame Pointer to can_frame to receive data.
 */
CAN_Error MCP_readMessage(can_frame *frame); //
/**
 * @brief Check if a receive message is available.
 *
 * @return 1 if a message is available, 0 otherwise.
 */
uint8_t MCP_checkReceive(void); //
/**
 * @brief Check whether an error condition is present.
 *
 * @return 1 if an error is present, 0 otherwise.
 */
uint8_t MCP_checkError(void); //

/**
 * @brief Read the MCP2515 EFLG (error) register.
 *
 * @return Raw EFLG register value.
 */
uint8_t MCP_getErrorFlags(void); //
/**
 * @brief Clear RX overflow flags.
 */
void MCP_clearRXnOVRFlags(void); //

/**
 * @brief Read the interrupt flag register (CANINTF).
 */
uint8_t MCP_getInterrupts(void); //

/**
 * @brief Read the interrupt enable mask (CANINTE).
 */
uint8_t MCP_getInterruptMask(void); //

/**
 * @brief Clear all interrupt flags (CANINTF).
 */
void MCP_clearInterrupts(void); //

/**
 * @brief Clear transmit complete interrupt flags for all TX buffers.
 */
void MCP_clearTXInterrupts(void); 
/**
 * @brief Read the MCP2515 read-status command result.
 *
 * @return Status byte returned by the MCP read-status instruction.
 */
uint8_t MCP_getStatus(void); //

/**
 * @brief Clear RX overflow conditions and related interrupts if set.
 */
void MCP_clearRXnOVR(void); //

/**
 * @brief Clear the message error (MERRF) interrupt flag.
 */
void MCP_clearMERR(); //

/**
 * @brief Clear the error interrupt flag (ERRIF).
 */
void MCP_clearERRIF(); //

uint8_t readRegister(REGISTER reg);

#endif /* APPLICATION_USER_INC_MCP2515_H_ */
