#include "acc.h"
#include "cmsis_os.h"

void Acc_GetCellVoltages(Acc_Module *acc, CellVoltages *cell_voltages){
    osMutexAcquire(acc->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        *cell_voltages = acc->cell_voltages;
    }
    osMutexRelease(acc->mutex);
}

void Acc_GetCellTemps(Acc_Module *acc, CellTemps *cell_temps){
    osMutexAcquire(acc->mutex, osWaitForever);
    if (cell_temps != NULL) {
        *cell_temps = acc->cell_temps;
    }
    osMutexRelease(acc->mutex);
}

void Acc_GetAmbientTemps(Acc_Module *acc, AmbientTemps *amb_temps){
    osMutexAcquire(acc->mutex, osWaitForever);
    if (amb_temps != NULL) {
        *amb_temps = acc->amb_temps;
    }
    osMutexRelease(acc->mutex);
}

void Acc_SetCellVoltages(Acc_Module *acc, const CellVoltages *cell_voltages){
    osMutexAcquire(acc->mutex, osWaitForever);
    if (cell_voltages != NULL) {
        acc->cell_voltages = *cell_voltages;
    }
    osMutexRelease(acc->mutex);
}

void Acc_SetCellTemps(Acc_Module *acc, const CellTemps *cell_temps){
    osMutexAcquire(acc->mutex, osWaitForever);
    if (cell_temps != NULL) {
        acc->cell_temps = *cell_temps;
    }
    osMutexRelease(acc->mutex);
}

void Acc_SetAmbientTemps(Acc_Module *acc, const AmbientTemps *amb_temps){
    osMutexAcquire(acc->mutex, osWaitForever);
    if (amb_temps != NULL) {
        acc->amb_temps = *amb_temps;
    }
    osMutexRelease(acc->mutex);
}