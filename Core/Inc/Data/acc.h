#ifndef __ACC_H
#define __ACC_H

#include "cmsis_os.h"
// TODO: move all the defined stuff to types.h or something
#include "io.h"

typedef struct {
    uint16_t volt_min;
    uint8_t volt_min_id; // Cell ID for min
    uint16_t volt_max;
    uint16_t volt_avg;
} CellVoltages;

typedef struct {
    uint16_t temp_min;
    uint16_t temp_max;
    uint16_t temp_avg; // Lots of work for HVC if BMB's don't
} CellTemps;

typedef struct {
    uint16_t amb_temp_1;
    uint16_t amb_temp_2;
} AmbientTemps;

typedef struct {
    osMutexId_t mutex;
    uint32_t heartbeat_last_update;
    CellVoltages cell_voltages;
    CellTemps cell_temps;
    AmbientTemps amb_temps;
} Acc_Module;

// Todo: add getters and setters
extern Acc_Module module_0;
extern Acc_Module module_1;
extern Acc_Module module_2;
extern Acc_Module module_3;
extern Acc_Module module_4;
extern Acc_Module module_5;

extern Acc_Module *acc[6];

#endif