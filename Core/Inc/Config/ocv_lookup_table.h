/** NOTE: THE OCV DATA DOES NOT ACCOUNT FOR VOLTAGE DROOP **/

#include <stdint.h>

typedef uint32_t voltage_mv_t;
typedef uint32_t capacity_pct_t;

typedef struct {
    voltage_mv_t voltage_mv;
    capacity_pct_t capacity_pct;  // Percent scaled by 100
} OCV_Voltage_Lookup_t;

static OCV_Voltage_Lookup_t OCV_Lookup_Table[];

#define OCV_TABLE_SIZE (sizeof(OCV_Lookup_Table)/sizeof(OCV_Voltage_Lookup_t))
