#include "acc.h"
#include "cmsis_os.h"

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