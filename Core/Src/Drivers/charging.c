#include "charging.h"
#include "can_id.h"
#include "state.h"


bool DecodeChargingHeartbeatBMS(const CAN_Message_t *in, BMS_Message_t *out) {
	(void)out;

	if (in == NULL) {
		return false;
	}

	return in->id == CAN_ID_CHARGING_REQUEST;
}

bool HandleChargingHeartbeatBMS(const BMS_Message_t *msg) {
	(void)msg;
    uint8_t data[8];


	if (charge_flag == NULL) {
		return false;
	}



	return (osEventFlagsSet(charge_flag, CHARGING_EVENT) & osFlagsError) == 0U;
}
