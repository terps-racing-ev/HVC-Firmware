/** NOTE: THE OCV DATA DOES NOT ACCOUNT FOR VOLTAGE DROOP **/

#ifndef OCV_LOOKUP_TABLE_H
#define OCV_LOOKUP_TABLE_H

#include <stdint.h>

typedef uint16_t voltage_mv_t;
typedef uint16_t capacity_pct_t;

typedef struct {
    voltage_mv_t voltage_mv;
    capacity_pct_t capacity_pctx100;  // Percent scaled by 100
} OCV_Voltage_Lookup_t;

extern OCV_Voltage_Lookup_t OCV_Lookup_Table[];
extern const uint32_t OCV_Lookup_Table_Size;

#endif /* OCV_LOOKUP_TABLE_H */
