#include "can_id.h"
#include "can.h"
#include "acc.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Decodes reset message
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded msg (UNUSED)
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeResetBMS(const CAN_Message_t *in, BMS_Message_t *out);

/**
 * @brief Reset HVC
 * @param msg: Input pointer to decoded msg (UNUSED)
 * @retval bool
 */
bool HandleResetBMS(const BMS_Message_t *msg);

/**
 * @brief Decodes reset message
 * @param in: Input pointer to received CAN message.
 * @param out: Output pointer to decoded msg (UNUSED)
 * @retval bool: Returns true if message is meant for this decoder, false if not.
 */
bool DecodeResetLV(const CAN_Message_t *in, LV_Message_t *out);

/**
 * @brief Reset HVC
 * @param msg: Input pointer to decoded msg (UNUSED)
 * @retval bool
 */
bool HandleResetLV(const LV_Message_t *msg);