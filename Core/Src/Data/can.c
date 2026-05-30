
#include "can.h"

HAL_StatusTypeDef HVC_CAN_SendMessage(uint32_t id, uint8_t *data, uint8_t length, uint8_t priority)
{
  HAL_StatusTypeDef bms_status;
  HAL_StatusTypeDef lv_status;

  bms_status = BMS_CAN_SendMessage(id, data, length, priority);
  lv_status = LV_CAN_SendMessage(id, data, length, priority);

  if ((bms_status != HAL_OK) || (lv_status != HAL_OK)) {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Reset CAN statistics (by zeroing out)
  * @param  stats: Can statistics struct
  * @retval none
  */
void CAN_ResetStatistics(CAN_Statistics_t *stats) {
    stats->bus_off_count = 0;
    stats->error_passive_count = 0;
    stats->recovery_count = 0;
    stats->rx_invalid_count = 0;
    stats->rx_message_count = 0;
    stats->rx_queue_full_count = 0;
    stats->tx_error_count = 0;
    stats->tx_queue_full_count = 0;
    stats->tx_success_count = 0;
}