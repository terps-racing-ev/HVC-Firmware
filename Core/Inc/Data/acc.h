#ifndef __ACC_H
#define __ACC_H

#include "cmsis_os.h"
// TODO: move all the defined stuff to types.h or something (?)
#include "io.h"

typedef struct {
    uint16_t volt_min_mV;
    uint16_t volt_max_mV;
    uint16_t volt_avg_mV;
    uint8_t volt_min_cell_id;
    uint8_t volt_max_cell_id;
} CellVoltages_t;

typedef struct {
    uint16_t temp_min_Cx10;
    uint16_t temp_max_Cx10;
    uint16_t temp_avg_Cx10;
    uint8_t temp_min_cell_id;
    uint8_t temp_max_cell_id;
} CellTemps_t;

typedef struct {
    int16_t amb_temp_1_Cx10;
    int16_t amb_temp_2_Cx10;
} AmbientTemps_t;

typedef struct {
    uint32_t heartbeat_timestamp;
    bool errored;
} HeartbeatMessage_t;

typedef struct {
    osMutexId_t mutex;
    uint32_t heartbeat_last_update;
    bool errored;
    CellVoltages_t cell_voltages_bms1;
    CellVoltages_t cell_voltages_bms2;
    CellTemps_t cell_temps;
    AmbientTemps_t amb_temps;
} Acc_Module_t;

typedef struct {
    int32_t cs_low;
    int32_t cs_high;
    uint32_t timestamp;
} Acc_CurrSenseSample_t;

typedef struct {
    osMutexId_t mutex;
    uint16_t volt_min_mV;
    uint16_t volt_max_mV;
    int16_t temp_min_Cx10;
    int16_t temp_max_Cx10;
} Acc_Summary_t;

/* Public variables  --------------------------------------------------------*/
#define NUM_ACC_MODULES 6
#define ACC_CURR_SENSE_QUEUE_SIZE 32U
#define ACC_FLOATING_CUTOFF_VOLTAGE_MV 100000   // Voltage below which batt input is considered floating
#define ACC_FLOATING_CUTOFF_HYSTERESIS_VOLTAGE_MV 5000  // Hysteresis so no oscillation

extern Acc_Module_t module_0;
extern Acc_Module_t module_1;
extern Acc_Module_t module_2;
extern Acc_Module_t module_3;
extern Acc_Module_t module_4;
extern Acc_Module_t module_5;

extern Acc_Module_t *acc[NUM_ACC_MODULES];
extern Acc_Summary_t acc_summary;

/* Getters  --------------------------------------------------------*/
/**
 * @brief Gets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param last_update Output pointer for the heartbeat timestamp.
 */
void Acc_GetHeartbeatLastUpdate(Acc_Module_t *module, uint32_t* last_update);

/**
 * @brief Gets the error status for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param errored Output pointer for the error status.
 */
void Acc_GetErrorStatus(Acc_Module_t *module, bool* errored);

/**
 * @brief Gets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Output pointer for cell voltage data.
 * @param bms1 True if request voltages for bms1
 */
void Acc_GetCellVoltages(Acc_Module_t *module, CellVoltages_t *cell_voltages, bool bms1);

/**
 * @brief Gets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Output pointer for cell temperature data.
 */
void Acc_GetCellTemps(Acc_Module_t *module, CellTemps_t *cell_temps);

/**
 * @brief Gets all ambient temperatures for the ACC module (deci-degC, Cx10).
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Output pointer for ambient temperature data.
 */
void Acc_GetAmbientTemps(Acc_Module_t *module, AmbientTemps_t *amb_temps);

/* Setters  --------------------------------------------------------*/
/**
 * @brief Sets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param heartbeat Message with heartbeat timestamp and error status
 */
void Acc_SetHeartbeat(Acc_Module_t *module, const HeartbeatMessage_t *heartbeat);

/**
 * @brief Sets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Input pointer containing cell voltage data.
 * @param is_bms1 True for BMS1 summary, false for BMS2 summary.
 */
void Acc_SetCellVoltages(Acc_Module_t *module, const CellVoltages_t *cell_voltages, bool is_bms1);

/**
 * @brief Sets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Input pointer containing cell temperature data.
 */
void Acc_SetCellTemps(Acc_Module_t *module, const CellTemps_t *cell_temps);

/**
 * @brief Sets all ambient temperatures for the ACC module (deci-degC, Cx10).
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Input pointer containing ambient temperature data.
 */
void Acc_SetAmbientTemps(Acc_Module_t *module, const AmbientTemps_t *amb_temps);

/**
 * @brief Pushes current-sense values and timestamp to the ACC-owned queue.
 *
 * @param cs_low Low-side current sample.
 * @param cs_high High-side current sample.
 * @param timestamp Tick timestamp for this sample.
 * @param timeout_ms Timeout for queue push (ms).
 * @return HAL_StatusTypeDef Queue operation status.
 */
HAL_StatusTypeDef Acc_CurrSenseQueue_Push(
    int32_t cs_low,
    int32_t cs_high,
    uint32_t timestamp,
    uint32_t timeout_ms
);

/**
 * @brief Calculates overall min/max cell voltage and temperature across all ACC modules.
 *
 * @param modules_checked Outpoint ptr to number of modules checked
 * @return HAL_OK when calculation succeeds, HAL_ERROR when no valid module data is available.
 */
HAL_StatusTypeDef Acc_CalculateSummary(uint8_t* modules_checked);

/**
 * @brief Gets the most recently calculated ACC summary.
 *
 * @param summary Output pointer for summary values.
 * @return HAL_OK when summary is available, HAL_ERROR if arguments are invalid or summary was not calculated yet.
 */
HAL_StatusTypeDef Acc_GetSummary(Acc_Summary_t *summary);

/**
 * @brief Sets the ACC summary values.
 *
 * @param summary Input pointer for summary values.
 * @return HAL_OK when summary is updated, HAL_ERROR if arguments are invalid or summary mutex is not initialized.
 */
HAL_StatusTypeDef Acc_SetSummary(const Acc_Summary_t *summary);

#endif
