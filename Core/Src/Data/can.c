
#include "can.h"

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