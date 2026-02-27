#ifndef __CAN_ID_H
#define __CAN_ID_H

/* HVC IDs  -------------------------------------------------------------------*/

#define CAN_ID_IO_SUMMARY   0x004001F0
#define CAN_ID_STATE        0x004001F1

#define CAN_ID_ERRORED_PANIC 0x004001F2 // TODO: figure out CAN ID


/* BMB IDs -------------------------------------------------------------------*/

typedef enum {
    /* Temperature Monitoring Messages */
    BMB_CAN_TEMP          = 0x08F00000u,  /**< Base for temperature messages */
    BMB_CAN_TEMP_RAW      = 0x08F00100u,  /**< Base for raw ADC diagnostic messages */
    BMB_CAN_TEMP_SUMMARY  = 0x08F00101u,  /**< Base for cell temp summary (min, max, BMS1/BMS2 IC temps) */

    /* Cell Voltage Messages */
    BMB_CAN_VOLTAGE_0     = 0x08F00200u,  /**< Base for voltage message 0 (cells 1-3) */
    BMB_CAN_VOLTAGE_1     = 0x08F00201u,  /**< Base for voltage message 1 (cells 4-6) */
    BMB_CAN_VOLTAGE_2     = 0x08F00202u,  /**< Base for voltage message 2 (cells 7-9) */
    BMB_CAN_VOLTAGE_3     = 0x08F00203u,  /**< Base for voltage message 3 (cells 10-12) */
    BMB_CAN_VOLTAGE_4     = 0x08F00204u,  /**< Base for voltage message 4 (cells 13-15) */
    BMB_CAN_VOLTAGE_5     = 0x08F00205u,  /**< Base for voltage message 5 (cells 16-18) */

    /* BQ76952 Chip Status Messages */
    BMB_CAN_BMS1_STATUS   = 0x08F00206u,  /**< Base for BMS1 chip status (stack voltage, alarm, temp) */
    BMB_CAN_BMS2_STATUS   = 0x08F00207u,  /**< Base for BMS2 chip status (stack voltage, alarm, temp) */

    /* BMS Status Messages */
    BMB_CAN_BMS_HEARTBEAT = 0x08F00300u,  /**< Base for BMS heartbeat/status message */
    BMB_CAN_BMS_STATS     = 0x08F00301u,  /**< Base for BMS CAN statistics (RX/TX counters) */
    // BMB_CAN_BMS_ERROR     = 0x08F00302u,  /**< Base for BMS detailed error status (optional) */

    /* Current Monitoring Messages */
    /* TODO: Add current monitoring CAN IDs here when defined */
    // BMB_CAN_CURRENT       = 0x08F00400u,  /**< Base for current measurement */

    /* State of Charge (SOC) Messages */
    /* TODO: Add SOC CAN IDs here when defined */
    // BMB_CAN_SOC           = 0x08F00500u,  /**< Base for state of charge */

    /* Command/Control Messages */
    BMB_CAN_CONFIG_CMD    = 0x08F00F00u,  /**< Base for configuration command (module ID in bits 15:12, e.g. 0x08F00F00, 0x08F01F00, etc.) */
    BMB_CAN_CONFIG_ACK    = 0x08F00F01u,  /**< Base for configuration command acknowledgement */
    BMB_CAN_RESET_CMD     = 0x08F00F02u,  /**< Base for reset command (module-specific) */
    BMB_CAN_BMS_RESET_CMD = 0x08F00F03u,  /**< Base for BQ76952 chip reset command (module-specific) */
    BMB_CAN_BMS_RESET_ACK = 0x08F00F04u,  /**< Base for BQ76952 chip reset acknowledgement */
    BMB_CAN_BALANCE_CMD   = 0x08F00F05u,  /**< Base for balance enable command (must send every 5s) */
    BMB_CAN_BALANCE_ACK   = 0x08F00F06u,  /**< Base for balance enable acknowledgement */
    BMB_CAN_BALANCE_CFG   = 0x08F00F07u,  /**< Base for balance config (target voltage, max cells) */
    BMB_CAN_BALANCE_STATUS= 0x08F00F08u,  /**< Base for balance status message (sent during balancing) */
    BMB_CAN_BMS1_BAL_DETAIL= 0x08F00F09u, /**< Base for BMS1 balance detail (readback from chip) */
    BMB_CAN_BMS2_BAL_DETAIL= 0x08F00F0Au, /**< Base for BMS2 balance detail (readback from chip) */
    BMB_CAN_DEBUG_REQUEST_ID    = 0x08F00F10u,  /**< Debug info request (broadcast - no module ID) */
    BMB_CAN_DEBUG_RESPONSE= 0x08F00F11u,  /**< Base for debug info response */
    BMB_CAN_I2C_DIAG      = 0x08F00F12u  /**< Base for I2C diagnostics response */
    // CAN_CMD           0x08F00600  /**< Base for command messages */
} BMS_Message_Type;



#endif