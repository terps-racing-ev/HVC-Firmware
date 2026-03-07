#ifndef IO_H
#define IO_H

#include "cmsis_os.h"
#include <stdint.h>
#include <time.h>

// Data types
/**
 * @struct DigitalIO
 * @brief Structure for digital I/O with thread-safe access
 */
typedef struct {
    osMutexId_t mutex;          /**< Mutex for thread-safe access */
    uint8_t value;              /**< Current digital value (0 or 1) */
    uint32_t last_updated;      /**< Timestamp of last update (ms) */
} DigitalIO;

/**
 * @struct AnalogIO
 * @brief Structure for analog I/O with thread-safe access
 */
typedef struct {
    osMutexId_t mutex;          /**< Mutex for thread-safe access */
    uint16_t value;             /**< Current analog value */
    uint32_t last_updated;      /**< Timestamp of last update (ms) */
} AnalogIO;

typedef struct {
    osMutexId_t mutex;          /**< Mutex for thread-safe access */
    float value;                 /**< Calculated temperature */
    uint32_t last_updated;
} Temp;

typedef struct {
    osMutexId_t mutex;          /**< Mutex for thread-safe access */
    uint32_t value;             /**< Current in mV */
    uint32_t last_updated;
} Current;

// Setters and getters for IO/temps
uint8_t IO_GetDigitalIO(DigitalIO *dio);
uint16_t IO_GetAnalogIO(AnalogIO *aio);
float IO_GetTemp(Temp *t);
uint32_t IO_GetCurrent(Current *c);
void IO_SetDigitalIO(DigitalIO *dio, uint16_t value);
void IO_SetAnalogIO(AnalogIO *aio, uint16_t value);
void IO_SetTemp(Temp *t, float value);
void IO_SetCurrent(Current *c, uint32_t value);



// Public IO values
extern DigitalIO sdc;
extern DigitalIO imd;
extern DigitalIO bms_fault;

extern AnalogIO cs_low_raw;
extern AnalogIO cs_high_raw;
extern AnalogIO therm;

extern Temp ref_temp;
extern Current cs_low;
extern Current cs_high;

#endif /* IO_H */
