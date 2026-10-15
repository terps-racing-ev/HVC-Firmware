#include "unity.h"
#include "curr_sense.h"
#include <stdlib.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_Curr_CalculateCurrentSenseHigh_returns_expected_value_near_zero_crossing(void)
{
    TEST_ASSERT_EQUAL_INT32(347, Curr_CalculateCurrentSenseHigh(1552U));
}

void test_Curr_CalculateCurrentSenseHigh_returns_expected_value_at_mid_scale(void)
{
    TEST_ASSERT_EQUAL_INT32(200201, Curr_CalculateCurrentSenseHigh(2048U));
}

void test_Curr_CalculateCurrentSenseHigh_returns_expected_value_at_full_scale(void)
{
    TEST_ASSERT_EQUAL_INT32(1025000, Curr_CalculateCurrentSenseHigh(4095U));
}

void test_Curr_CalculateCurrentSenseHigh_returns_negative_at_zero_adc(void)
{
    TEST_ASSERT_EQUAL_INT32(-625000, Curr_CalculateCurrentSenseHigh(0U));
}

void test_Curr_CalculateCurrentSenseHigh_increases_with_adc_value_in_positive_region(void)
{
    int32_t low = Curr_CalculateCurrentSenseHigh(1600U);
    int32_t mid = Curr_CalculateCurrentSenseHigh(3000U);
    int32_t high = Curr_CalculateCurrentSenseHigh(4095U);

    TEST_ASSERT_TRUE(low < mid);
    TEST_ASSERT_TRUE(mid < high);
}

void test_Curr_CalculateCurrentSenseLow_returns_expected_value_near_zero_crossing(void)
{
    TEST_ASSERT_EQUAL_INT32(52, Curr_CalculateCurrentSenseLow(1552U));
}

void test_Curr_CalculateCurrentSenseLow_returns_expected_value_at_mid_scale(void)
{
    TEST_ASSERT_EQUAL_INT32(29992, Curr_CalculateCurrentSenseLow(2048U));
}

void test_Curr_CalculateCurrentSenseLow_returns_expected_value_at_full_scale(void)
{
    TEST_ASSERT_EQUAL_INT32(153558, Curr_CalculateCurrentSenseLow(4095U));
}

void test_Curr_CalculateCurrentSenseLow_returns_negative_at_zero_adc(void)
{
    TEST_ASSERT_EQUAL_INT32(-93632, Curr_CalculateCurrentSenseLow(0U));
}

void test_Curr_CalculateCurrentSenseLow_increases_with_adc_value_in_positive_region(void)
{
    int32_t low = Curr_CalculateCurrentSenseLow(1600U);
    int32_t mid = Curr_CalculateCurrentSenseLow(3000U);
    int32_t high = Curr_CalculateCurrentSenseLow(4095U);

    TEST_ASSERT_TRUE(low < mid);
    TEST_ASSERT_TRUE(mid < high);
}

void test_Curr_CalculateCurrentSenseLow_absolute_value_is_less_than_high(void)
{
    const uint32_t samples[] = {0U, 512U, 1024U, 1552U, 2048U, 3072U, 4095U};
    const size_t count = sizeof(samples) / sizeof(samples[0]);
    size_t i;

    for (i = 0; i < count; ++i) {
        int32_t low = Curr_CalculateCurrentSenseLow(samples[i]);
        int32_t high = Curr_CalculateCurrentSenseHigh(samples[i]);
        TEST_ASSERT_TRUE(labs(low) < labs(high));
    }
}

void test_Curr_CalculateCurrentSense_returns_about_zero_at_1p25V(void)
{
    const uint32_t adc_floor = 1551U;
    const uint32_t adc_ceil = 1552U;
    const int32_t high_tol = 500;
    const int32_t low_tol = 100;

    int32_t high_floor = Curr_CalculateCurrentSenseHigh(adc_floor);
    int32_t high_ceil = Curr_CalculateCurrentSenseHigh(adc_ceil);
    int32_t low_floor = Curr_CalculateCurrentSenseLow(adc_floor);
    int32_t low_ceil = Curr_CalculateCurrentSenseLow(adc_ceil);

    TEST_ASSERT_TRUE(high_floor <= 0);
    TEST_ASSERT_TRUE(high_ceil >= 0);
    TEST_ASSERT_TRUE(low_floor <= 0);
    TEST_ASSERT_TRUE(low_ceil >= 0);

    TEST_ASSERT_TRUE(labs(high_floor) <= high_tol);
    TEST_ASSERT_TRUE(labs(high_ceil) <= high_tol);
    TEST_ASSERT_TRUE(labs(low_floor) <= low_tol);
    TEST_ASSERT_TRUE(labs(low_ceil) <= low_tol);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Curr_CalculateCurrentSenseHigh_returns_expected_value_near_zero_crossing);
    RUN_TEST(test_Curr_CalculateCurrentSenseHigh_returns_expected_value_at_mid_scale);
    RUN_TEST(test_Curr_CalculateCurrentSenseHigh_returns_expected_value_at_full_scale);
    RUN_TEST(test_Curr_CalculateCurrentSenseHigh_returns_negative_at_zero_adc);
    RUN_TEST(test_Curr_CalculateCurrentSenseHigh_increases_with_adc_value_in_positive_region);
    RUN_TEST(test_Curr_CalculateCurrentSenseLow_returns_expected_value_near_zero_crossing);
    RUN_TEST(test_Curr_CalculateCurrentSenseLow_returns_expected_value_at_mid_scale);
    RUN_TEST(test_Curr_CalculateCurrentSenseLow_returns_expected_value_at_full_scale);
    RUN_TEST(test_Curr_CalculateCurrentSenseLow_returns_negative_at_zero_adc);
    RUN_TEST(test_Curr_CalculateCurrentSenseLow_increases_with_adc_value_in_positive_region);
    RUN_TEST(test_Curr_CalculateCurrentSenseLow_absolute_value_is_less_than_high);
    RUN_TEST(test_Curr_CalculateCurrentSense_returns_about_zero_at_1p25V);
    return UNITY_END();
}