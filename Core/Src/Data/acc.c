#include "acc.h"
#include "cmsis_os.h"

void Acc_GetCellTemps(Acc_Module *mod, CellTemps *out) {
    osMutexAcquire(mod->mutex, osWaitForever);
    *out = mod->cell_temps;
    osMutexRelease(mod->mutex);
}

void Acc_SetCellTemps(Acc_Module *mod, const CellTemps *temps) {
    uint32_t now = osKernelGetTickCount();

    osMutexAcquire(mod->mutex, osWaitForever);
    mod->cell_temps = *temps;
    mod->heartbeat_last_update = now;
    osMutexRelease(mod->mutex);
}
