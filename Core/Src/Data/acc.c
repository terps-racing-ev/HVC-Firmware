#include "acc.h"
#include "cmsis_os.h"

Acc_Summary_t acc_summary = {0};
static uint8_t acc_summary_valid = 0U;

Acc_Module module_0 = {0};
Acc_Module module_1 = {0};
Acc_Module module_2 = {0};
Acc_Module module_3 = {0};
Acc_Module module_4 = {0};
Acc_Module module_5 = {0};

Acc_Module *acc[6] = {
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
void Acc_GetHeartbeatLastUpdate(Acc_Module *module, uint32_t* last_update){
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
 */
void Acc_GetCellVoltages(Acc_Module *module, CellVoltages *cell_voltages){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        *cell_voltages = module->cell_voltages;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Gets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Output pointer for cell temperature data.
 */
void Acc_GetCellTemps(Acc_Module *module, CellTemps *cell_temps){
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
void Acc_GetAmbientTemps(Acc_Module *module, AmbientTemps *amb_temps){
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
 * @param last_update Input pointer containing the heartbeat timestamp.
 */
void Acc_SetHeartbeatLastUpdate(Acc_Module *module, uint32_t* last_update){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (last_update != NULL) {
        module->heartbeat_last_update = *last_update;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Sets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Input pointer containing cell voltage data.
 */
void Acc_SetCellVoltages(Acc_Module *module, const CellVoltages *cell_voltages){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        module->cell_voltages = *cell_voltages;
    }
    osMutexRelease(module->mutex);
}

/**
 * @brief Sets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Input pointer containing cell temperature data.
 */
void Acc_SetCellTemps(Acc_Module *module, const CellTemps *cell_temps){
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
void Acc_SetAmbientTemps(Acc_Module *module, const AmbientTemps *amb_temps){
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
    CellVoltages module_voltages;
    CellTemps module_temps;
    Acc_Summary_t next_summary;
    uint32_t i;
    uint8_t has_valid_module = 0U;
    uint32_t heartbeat_last_update;

    for (i = 0U; i < NUM_ACC_MODULES; i++) {
        if (acc[i] == NULL) {
            continue;
        }

        Acc_GetCellVoltages(acc[i], &module_voltages);
        Acc_GetCellTemps(acc[i], &module_temps);
        Acc_GetHeartbeatLastUpdate(acc[i], &heartbeat_last_update);
        
        // No data
        if (
            module_voltages.volt_min == 0 || 
            module_temps.temp_min == 0||
            heartbeat_last_update == 0
        ){
            continue;
        } else {
            *modules_checked++;
        }

        if (has_valid_module == 0U) {
            next_summary.volt_min = module_voltages.volt_min;
            next_summary.volt_max = module_voltages.volt_max;
            next_summary.temp_min = module_temps.temp_min;
            next_summary.temp_max = module_temps.temp_max;
            has_valid_module = 1U;
            continue;
        }

        if (module_voltages.volt_min < next_summary.volt_min) {
            next_summary.volt_min = module_voltages.volt_min;
        }
        if (module_voltages.volt_max > next_summary.volt_max) {
            next_summary.volt_max = module_voltages.volt_max;
        }
        if (module_temps.temp_min < next_summary.temp_min) {
            next_summary.temp_min = module_temps.temp_min;
        }
        if (module_temps.temp_max > next_summary.temp_max) {
            next_summary.temp_max = module_temps.temp_max;
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
    summary->volt_min = acc_summary.volt_min;
    summary->volt_max = acc_summary.volt_max;
    summary->temp_min = acc_summary.temp_min;
    summary->temp_max = acc_summary.temp_max;
    osMutexRelease(acc_summary.mutex);

    return HAL_OK;
}

HAL_StatusTypeDef Acc_SetSummary(const Acc_Summary_t *summary)
{
    if ((summary == NULL) || (acc_summary.mutex == NULL)) {
        return HAL_ERROR;
    }

    osMutexAcquire(acc_summary.mutex, osWaitForever);
    acc_summary.volt_min = summary->volt_min;
    acc_summary.volt_max = summary->volt_max;
    acc_summary.temp_min = summary->temp_min;
    acc_summary.temp_max = summary->temp_max;
    acc_summary_valid = 1U;
    osMutexRelease(acc_summary.mutex);

    return HAL_OK;
}