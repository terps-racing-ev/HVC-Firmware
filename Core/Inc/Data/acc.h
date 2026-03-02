#ifndef __ACC_H
#define __ACC_H

#include "cmsis_os.h"
// TODO: move all the defined stuff to types.h or something (?)
#include "io.h"

typedef struct {
    uint16_t volt_min;
    uint16_t volt_max;
    uint8_t volt_min_cell_id;
    uint8_t volt_max_cell_id;
    uint16_t volt_avg;
} CellVoltages;

typedef struct {
    float temp_min;
    float temp_max;
    float temp_avg; // Lots of work for HVC if BMB's don't
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

/* Public variables  --------------------------------------------------------*/
#define NUM_ACC_MODULES 6

extern Acc_Module module_0;
extern Acc_Module module_1;
extern Acc_Module module_2;
extern Acc_Module module_3;
extern Acc_Module module_4;
extern Acc_Module module_5;

extern Acc_Module *acc[NUM_ACC_MODULES];

/* Getters  --------------------------------------------------------*/
/**
 * @brief Gets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param last_update Output pointer for the heartbeat timestamp.
 */
void Acc_GetHeartbeatLastUpdate(Acc_Module *module, uint32_t* last_update);

/**
 * @brief Gets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Output pointer for cell voltage data.
 */
void Acc_GetCellVoltages(Acc_Module *module, CellVoltages *cell_voltages);

/**
 * @brief Gets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Output pointer for cell temperature data.
 */
void Acc_GetCellTemps(Acc_Module *module, CellTemps *cell_temps);

/**
 * @brief Gets all ambient temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Output pointer for ambient temperature data.
 */
void Acc_GetAmbientTemps(Acc_Module *module, AmbientTemps *amb_temps);

/* Setters  --------------------------------------------------------*/
/**
 * @brief Sets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param last_update Input pointer containing the heartbeat timestamp.
 */
void Acc_SetHeartbeatLastUpdate(Acc_Module *module, uint32_t* last_update);

/**
 * @brief Sets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Input pointer containing cell voltage data.
 */
void Acc_SetCellVoltages(Acc_Module *module, const CellVoltages *cell_voltages);

/**
 * @brief Sets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Input pointer containing cell temperature data.
 */
void Acc_SetCellTemps(Acc_Module *module, const CellTemps *cell_temps);

/**
 * @brief Sets all ambient temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Input pointer containing ambient temperature data.
 */
void Acc_SetAmbientTemps(Acc_Module *module, const AmbientTemps *amb_temps);

#endif