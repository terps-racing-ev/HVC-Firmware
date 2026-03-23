#include "acc.h"
#include "cmsis_os.h"

/**
 * @brief Inits an acc moduel
 *
 * @param module Pointer to the ACC module instance.
 */
HAL_StatusTypeDef Acc_InitModule(Acc_Module_t *module, const char *mutex_name) {
    if (module == NULL || mutex_name == NULL) {
        return HAL_ERROR;
    }
    const osMutexAttr_t module_mutex_attr = {
        .name = mutex_name
    };
    module->mutex = osMutexNew(&module_mutex_attr);
    if (!module->mutex) return HAL_ERROR;

    // Heartbeat
    module->heartbeat_last_update = 0;

    // Cell Voltages
    module->cell_voltages.volt_min = 0;
    module->cell_voltages.volt_max = 0;
    module->cell_voltages.volt_min_cell_id = 0;
    module->cell_voltages.volt_max_cell_id = 0;
    module->cell_voltages.volt_avg = 0;

    // Cell temps
    module->cell_temps.temp_min = 0.0f;
    module->cell_temps.temp_max = 0.0f;
    module->cell_temps.temp_avg = 0.0f;

    // Ambient temps
    module->amb_temps.amb_temp_1 = 0;
    module->amb_temps.amb_temp_2 = 0;

    return HAL_OK;
}

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
 */
void Acc_GetCellVoltages(Acc_Module_t *module, Cell_Voltages_t *cell_voltages){
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
void Acc_GetCellTemps(Acc_Module_t *module, Cell_Temps_t *cell_temps){
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
void Acc_GetAmbientTemps(Acc_Module_t *module, Ambient_Temps_t *amb_temps){
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
void Acc_SetHeartbeatLastUpdate(Acc_Module_t *module, uint32_t* last_update){
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
void Acc_SetCellVoltages(Acc_Module_t *module, const Cell_Voltages_t *cell_voltages){
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
void Acc_SetCellTemps(Acc_Module_t *module, const Cell_Temps_t *cell_temps){
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
void Acc_SetAmbientTemps(Acc_Module_t *module, const Ambient_Temps_t *amb_temps){
    if (module == NULL) {
        return;
    }

    osMutexAcquire(module->mutex, osWaitForever);
    if (amb_temps != NULL) {
        module->amb_temps = *amb_temps;
    }
    osMutexRelease(module->mutex);
}