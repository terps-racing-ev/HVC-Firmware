#include "unity.h"
#include "acc.h"
#include "flash.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t voltage_mv_t;
typedef uint32_t capacity_pct_t;

typedef struct {
    voltage_mv_t voltage_mv;
    capacity_pct_t capacity_pct;
} OCV_Voltage_Lookup;

extern OCV_Voltage_Lookup ocv_lookup_table[];
extern size_t ocv_lookup_table_size;

extern int32_t delta_capacity_a_ms;

void SOC_Init(void);
void SOC_UpdateDeltaCapacity(Curr_Sense_Reading_t *cs_reading);
capacity_pct_t SOC_CalculateCapacityPct(uint32_t cell_voltage_mV);
uint32_t SOC_CapacityPctToAms(capacity_pct_t capacity_pct);

static Flash_Data_t flash_stub_data;
static HAL_StatusTypeDef flash_stub_status = HAL_OK;

HAL_StatusTypeDef Flash_ReadData(Flash_Data_t *data)
{
    if (data != NULL) {
        *data = flash_stub_data;
    }
    return flash_stub_status;
}

static void prime_soc_timestamp(uint32_t timestamp_ticks)
{
    Curr_Sense_Reading_t reading = {
        .cs_low_val_mA = 0,
        .cs_high_val_mA = 0,
        .timestamp_ticks = timestamp_ticks
    };

    SOC_UpdateDeltaCapacity(&reading);
}

void setUp(void)
{
    ocv_lookup_table[0].voltage_mv = 4200U;
    ocv_lookup_table[0].capacity_pct = 10000U;
    ocv_lookup_table[1].voltage_mv = 3200U;
    ocv_lookup_table[1].capacity_pct = 0U;
    ocv_lookup_table_size = 2U;

    flash_stub_status = HAL_OK;
    flash_stub_data.soc_total_capacity = 0U;
    flash_stub_data.soc_last_load_time = 0U;

    SOC_Init();
    prime_soc_timestamp(1000U);
}

void tearDown(void)
{
}

void test_SOC_Init_resets_delta_capacity(void)
{
    delta_capacity_a_ms = 12345;
    SOC_Init();
    TEST_ASSERT_EQUAL_INT32(0, delta_capacity_a_ms);
}

void test_SOC_UpdateDeltaCapacity_uses_low_channel_within_bounds(void)
{
    Curr_Sense_Reading_t reading = {
        .cs_low_val_mA = 1000,
        .cs_high_val_mA = 8000,
        .timestamp_ticks = 2000U
    };

    SOC_UpdateDeltaCapacity(&reading);

    TEST_ASSERT_EQUAL_INT32(-1000, delta_capacity_a_ms);
}

void test_SOC_UpdateDeltaCapacity_uses_high_channel_when_low_out_of_range(void)
{
    Curr_Sense_Reading_t reading = {
        .cs_low_val_mA = 70001,
        .cs_high_val_mA = 15000,
        .timestamp_ticks = 2000U
    };

    SOC_UpdateDeltaCapacity(&reading);

    TEST_ASSERT_EQUAL_INT32(-15000, delta_capacity_a_ms);
}

void test_SOC_UpdateDeltaCapacity_negative_current_increases_capacity(void)
{
    Curr_Sense_Reading_t reading = {
        .cs_low_val_mA = -5000,
        .cs_high_val_mA = -8000,
        .timestamp_ticks = 2000U
    };

    SOC_UpdateDeltaCapacity(&reading);

    TEST_ASSERT_EQUAL_INT32(5000, delta_capacity_a_ms);
}

void test_SOC_CalculateCapacityPct_returns_full_for_voltage_at_or_above_max(void)
{
    TEST_ASSERT_EQUAL_UINT32(10000U, SOC_CalculateCapacityPct(4200U));
    TEST_ASSERT_EQUAL_UINT32(10000U, SOC_CalculateCapacityPct(4500U));
}

void test_SOC_CalculateCapacityPct_returns_zero_below_min_voltage(void)
{
    TEST_ASSERT_EQUAL_UINT32(0U, SOC_CalculateCapacityPct(3199U));
}

void test_SOC_CapacityPctToAms_scales_percent_correctly(void)
{
    flash_stub_data.soc_total_capacity = 500000U;
    flash_stub_status = HAL_OK;

    TEST_ASSERT_EQUAL_UINT32(500000U, SOC_CapacityPctToAms(10000U));
}

void test_SOC_CapacityPctToAms_returns_zero_on_flash_read_failure(void)
{
    flash_stub_data.soc_total_capacity = 123456U;
    flash_stub_status = HAL_ERROR;

    TEST_ASSERT_EQUAL_UINT32(0U, SOC_CapacityPctToAms(10000U));
}

void test_SOC_CapacityPctToAms_clamps_on_overflow(void)
{
    flash_stub_data.soc_total_capacity = UINT32_MAX;
    flash_stub_status = HAL_OK;

    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, SOC_CapacityPctToAms(10000U));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_SOC_Init_resets_delta_capacity);
    RUN_TEST(test_SOC_UpdateDeltaCapacity_uses_low_channel_within_bounds);
    RUN_TEST(test_SOC_UpdateDeltaCapacity_uses_high_channel_when_low_out_of_range);
    RUN_TEST(test_SOC_UpdateDeltaCapacity_negative_current_increases_capacity);
    RUN_TEST(test_SOC_CalculateCapacityPct_returns_full_for_voltage_at_or_above_max);
    RUN_TEST(test_SOC_CalculateCapacityPct_returns_zero_below_min_voltage);
    RUN_TEST(test_SOC_CapacityPctToAms_scales_percent_correctly);
    RUN_TEST(test_SOC_CapacityPctToAms_returns_zero_on_flash_read_failure);
    RUN_TEST(test_SOC_CapacityPctToAms_clamps_on_overflow);
    return UNITY_END();
}
