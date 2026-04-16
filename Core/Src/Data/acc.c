#include "acc.h"
#include "cmsis_os.h"

Acc_Summary_t acc_summary = {0};
static uint8_t acc_summary_valid = 0U;

Acc_Module_t module_0 = {0};
Acc_Module_t module_1 = {0};
Acc_Module_t module_2 = {0};
Acc_Module_t module_3 = {0};
Acc_Module_t module_4 = {0};
Acc_Module_t module_5 = {0};

Acc_Module_t *acc[6] = {
    &module_0,
    &module_1,
    &module_2,
    &module_3,
    &module_4,
    &module_5
};

/**
 * @brief Gets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param last_update Output pointer for the heartbeat timestamp.
 */
void Acc_GetHeartbeatLastUpdate(Acc_Module_t *module, uint32_t* last_update){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (last_update != NULL) {
        *last_update = module->heartbeat_last_update;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Gets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Output pointer for cell voltage data.
 * @param bms1 True if request voltages for bms1
 */
void Acc_GetCellVoltages(Acc_Module_t *module, CellVoltages_t *cell_voltages, bool bms1){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        if (bms1) *cell_voltages = module->cell_voltages_bms1;
        else *cell_voltages = module->cell_voltages_bms2;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Gets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Output pointer for cell temperature data.
 */
void Acc_GetCellTemps(Acc_Module_t *module, CellTemps_t *cell_temps){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_temps != NULL) {
        *cell_temps = module->cell_temps;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Gets all ambient temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Output pointer for ambient temperature data.
 */
void Acc_GetAmbientTemps(Acc_Module_t *module, AmbientTemps_t *amb_temps){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (amb_temps != NULL) {
        *amb_temps = module->amb_temps;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Sets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param heartbeat Message with heartbeat timestamp and error status
 */
void Acc_SetHeartbeat(Acc_Module_t *module, const HeartbeatMessage_t *heartbeat){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (heartbeat != NULL) {
        module->heartbeat_last_update = heartbeat->heartbeat_timestamp;
        module->errored = heartbeat->errored;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Sets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Input pointer containing cell voltage data.
 * @param is_bms1 True for BMS1 summary, false for BMS2 summary.
 */
void Acc_SetCellVoltages(Acc_Module_t *module, const CellVoltages_t *cell_voltages, bool is_bms1){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        if (is_bms1) {
            module->cell_voltages_bms1 = *cell_voltages;
        } else {
            module->cell_voltages_bms2 = *cell_voltages;
        }
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Sets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Input pointer containing cell temperature data.
 */
void Acc_SetCellTemps(Acc_Module_t *module, const CellTemps_t *cell_temps){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_temps != NULL) {
        module->cell_temps = *cell_temps;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Sets all ambient temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Input pointer containing ambient temperature data.
 */
void Acc_SetAmbientTemps(Acc_Module_t *module, const AmbientTemps_t *amb_temps){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (amb_temps != NULL) {
        module->amb_temps = *amb_temps;
    }
    osMutexRelease(module->mutex);
}

HAL_StatusTypeDef Acc_CalculateSummary(uint8_t *modules_checked)
{
    CellVoltages_t module_voltages_bms1;
    CellVoltages_t module_voltages_bms2;
    CellTemps_t module_temps;
    Acc_Summary_t next_summary;
    uint32_t i;
    uint8_t has_valid_module = 0U;
    uint16_t min_voltage_mV, max_voltage_mV;
    int16_t min_temp_Cx10, max_temp_Cx10;
    uint32_t heartbeat_last_update;

    if (modules_checked == NULL) {
        return HAL_ERROR;
    }

    *modules_checked = 0;

    for (i = 0U; i < NUM_ACC_MODULES; i++) {
        if (acc[i] == NULL) {
            continue;
        }

        Acc_GetCellVoltages(acc[i], &module_voltages_bms1, true);
        Acc_GetCellVoltages(acc[i], &module_voltages_bms2, false);
        Acc_GetCellTemps(acc[i], &module_temps);
        Acc_GetHeartbeatLastUpdate(acc[i], &heartbeat_last_update);
        
        // Some data missing
        if (
            module_voltages_bms1.volt_min_mV == 0 || 
            module_voltages_bms2.volt_min_mV == 0 || 
            module_temps.temp_min_Cx10 == 0||
            heartbeat_last_update == 0
        ){
            continue;
        } else {
            (*modules_checked)++;
        }

        min_voltage_mV = (
            (module_voltages_bms1.volt_min_mV < module_voltages_bms2.volt_min_mV) ?
            module_voltages_bms1.volt_min_mV :
            module_voltages_bms2.volt_min_mV
        );

        max_voltage_mV = (
            (module_voltages_bms1.volt_max_mV > module_voltages_bms2.volt_max_mV) ?
            module_voltages_bms1.volt_max_mV :
            module_voltages_bms2.volt_max_mV
        );

        min_temp_Cx10 = (int16_t)module_temps.temp_min_Cx10;
        max_temp_Cx10 = (int16_t)module_temps.temp_max_Cx10;

        if (has_valid_module == 0U) {
            next_summary.volt_min_mV = min_voltage_mV;
            next_summary.volt_max_mV = max_voltage_mV;
            next_summary.temp_min_Cx10 = min_temp_Cx10;
            next_summary.temp_max_Cx10 = max_temp_Cx10;
            has_valid_module = 1U;
            continue;
        }

        if (min_voltage_mV < next_summary.volt_min_mV) {
            next_summary.volt_min_mV = min_voltage_mV;
        }
        if (max_voltage_mV > next_summary.volt_max_mV) {
            next_summary.volt_max_mV = max_voltage_mV;
        }
        if (min_temp_Cx10 < next_summary.temp_min_Cx10) {
            next_summary.temp_min_Cx10 = min_temp_Cx10;
        }
        if (max_temp_Cx10 > next_summary.temp_max_Cx10) {
            next_summary.temp_max_Cx10 = max_temp_Cx10;
        }
    }

    if (has_valid_module == 0U) {
        return HAL_ERROR;
    }

    return Acc_SetSummary(&next_summary);
}

HAL_StatusTypeDef Acc_GetSummary(Acc_Summary_t *summary)
{
    if ((summary == NULL) || (acc_summary_valid == 0U) || (acc_summary.mutex == NULL)) {
        return HAL_ERROR;
    }

    osMutexAcquire(acc_summary.mutex, osWaitForever);
    summary->mutex = NULL;
    summary->volt_min_mV = acc_summary.volt_min_mV;
    summary->volt_max_mV = acc_summary.volt_max_mV;
    summary->temp_min_Cx10 = acc_summary.temp_min_Cx10;
    summary->temp_max_Cx10 = acc_summary.temp_max_Cx10;
    osMutexRelease(acc_summary.mutex);

    return HAL_OK;
}

HAL_StatusTypeDef Acc_SetSummary(const Acc_Summary_t *summary)
{
    if ((summary == NULL) || (acc_summary.mutex == NULL)) {
        return HAL_ERROR;
    }

    osMutexAcquire(acc_summary.mutex, osWaitForever);
    acc_summary.volt_min_mV = summary->volt_min_mV;
    acc_summary.volt_max_mV = summary->volt_max_mV;
    acc_summary.temp_min_Cx10 = summary->temp_min_Cx10;
    acc_summary.temp_max_Cx10 = summary->temp_max_Cx10;
    acc_summary_valid = 1U;
    osMutexRelease(acc_summary.mutex);

    return HAL_OK;
}