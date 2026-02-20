#ifndef IO_H
#define IO_H

#include "cmsis_os2.h"
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

// Public IO values
extern DigitalIO sdc;
extern DigitalIO imd;
extern DigitalIO bms_fault;

extern AnalogIO cs_low;
extern AnalogIO cs_high;

#endif /* IO_H */