#ifndef __ACC_H
#define __ACC_H

#include "cmsis_os.h"
#include "io.h"
#include "stm32l4xx_hal.h"

typedef struct {
    uint16_t volt_min;
    uint16_t volt_max;
    uint8_t volt_min_cell_id;
    uint8_t volt_max_cell_id;
    uint16_t volt_avg;
} Cell_Voltages_t;

typedef struct {
    float temp_min;
    float temp_max;
    float temp_avg; // Lots of work for HVC if BMB's don't
} Cell_Temps_t;

typedef struct {
    float amb_temp_1;
    float amb_temp_2;
} Ambient_Temps_t;

typedef struct {
    osMutexId_t mutex;
    uint32_t heartbeat_last_update;
    Cell_Voltages_t cell_voltages;
    Cell_Temps_t cell_temps;
    Ambient_Temps_t amb_temps;
} Acc_Module_t;

typedef struct {
  int32_t cs_low_val_mA;
  int32_t cs_high_val_mA;
  uint32_t timestamp_ticks;
} Curr_Sense_Reading_t;

/* Public variables  --------------------------------------------------------*/
#define NUM_ACC_MODULES 6

extern Acc_Module_t module_0;
extern Acc_Module_t module_1;
extern Acc_Module_t module_2;
extern Acc_Module_t module_3;
extern Acc_Module_t module_4;
extern Acc_Module_t module_5;

extern Acc_Module_t *acc[NUM_ACC_MODULES];

/**
 * @brief Adds curr sense reading to soc queue
 *
 * @param cs_low_val Curr Sense low channel reading
 * @param cs_high_val Curr Sense high channel reading
 * @param timestamp When reading was done
 */
HAL_StatusTypeDef Acc_AddSocCurrSense(int32_t cs_low_val, int32_t cs_high_val, uint32_t timestamp);

/* Initers  --------------------------------------------------------*/
/**
 * @brief Inits an acc module
 *
 * @param module Pointer to the ACC module instance.
 */
HAL_StatusTypeDef Acc_InitModule(Acc_Module_t *module, const char *mutex_name);

/* Getters  --------------------------------------------------------*/
/**
 * @brief Gets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param last_update Output pointer for the heartbeat timestamp.
 */
void Acc_GetHeartbeatLastUpdate(Acc_Module_t *module, uint32_t* last_update);

/**
 * @brief Gets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Output pointer for cell voltage data.
 */
void Acc_GetCellVoltages(Acc_Module_t *module, Cell_Voltages_t *cell_voltages);

/**
 * @brief Gets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Output pointer for cell temperature data.
 */
void Acc_GetCellTemps(Acc_Module_t *module, Cell_Temps_t *cell_temps);

/**
 * @brief Gets all ambient temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Output pointer for ambient temperature data.
 */
void Acc_GetAmbientTemps(Acc_Module_t *module, Ambient_Temps_t *amb_temps);

/* Setters  --------------------------------------------------------*/
/**
 * @brief Sets the last heartbeat timestamp for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param last_update Input pointer containing the heartbeat timestamp.
 */
void Acc_SetHeartbeatLastUpdate(Acc_Module_t *module, uint32_t* last_update);

/**
 * @brief Sets all cell voltages for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_voltages Input pointer containing cell voltage data.
 */
void Acc_SetCellVoltages(Acc_Module_t *module, const Cell_Voltages_t *cell_voltages);

/**
 * @brief Sets all cell temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param cell_temps Input pointer containing cell temperature data.
 */
void Acc_SetCellTemps(Acc_Module_t *module, const Cell_Temps_t *cell_temps);

/**
 * @brief Sets all ambient temperatures for the ACC module.
 *
 * @param module Pointer to the ACC module instance.
 * @param amb_temps Input pointer containing ambient temperature data.
 */
void Acc_SetAmbientTemps(Acc_Module_t *module, const Ambient_Temps_t *amb_temps);

#endif