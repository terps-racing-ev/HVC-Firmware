#include "unity.h"
#include "io_manager.h"
#include "test_stubs.h"

void setUp(void)
{
    Test_Stubs_Reset();
}

void tearDown(void)
{
}

void test_IO_Manager_Init_success(void)
{
    osMutexId_t results[9] = {
        (void*)0x1, (void*)0x2, (void*)0x3,
        (void*)0x4, (void*)0x5, (void*)0x6,
        (void*)0x7, (void*)0x8, (void*)0x9
    };
    Test_SetMutexNewResults(results, 9);
    Test_SetAdcStartResult(HAL_OK);
    Test_SetCompStartResult(HAL_OK);

    TEST_ASSERT_EQUAL(HAL_OK, IO_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(9, Test_GetMutexNewCallCount());

    TEST_ASSERT_EQUAL_PTR(results[0], sdc.mutex);
    TEST_ASSERT_EQUAL_UINT8(0, sdc.value);
    TEST_ASSERT_EQUAL_UINT32(0, sdc.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[1], imd.mutex);
    TEST_ASSERT_EQUAL_UINT8(0, imd.value);
    TEST_ASSERT_EQUAL_UINT32(0, imd.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[2], bms_fault.mutex);
    TEST_ASSERT_EQUAL_UINT8(0, bms_fault.value);
    TEST_ASSERT_EQUAL_UINT32(0, bms_fault.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[3], cs_low_raw.mutex);
    TEST_ASSERT_EQUAL_UINT16(0, cs_low_raw.value);
    TEST_ASSERT_EQUAL_UINT32(0, cs_low_raw.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[4], cs_high_raw.mutex);
    TEST_ASSERT_EQUAL_UINT16(0, cs_high_raw.value);
    TEST_ASSERT_EQUAL_UINT32(0, cs_high_raw.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[5], therm.mutex);
    TEST_ASSERT_EQUAL_UINT16(0, therm.value);
    TEST_ASSERT_EQUAL_UINT32(0, therm.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[6], ref_temp.mutex);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ref_temp.value);
    TEST_ASSERT_EQUAL_UINT32(0, ref_temp.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[7], cs_low.mutex);
    TEST_ASSERT_EQUAL_UINT32(0, cs_low.value);
    TEST_ASSERT_EQUAL_UINT32(0, cs_low.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[8], cs_high.mutex);
    TEST_ASSERT_EQUAL_UINT32(0, cs_high.value);
    TEST_ASSERT_EQUAL_UINT32(0, cs_high.last_updated);
}

void test_IO_Manager_Init_adc_start_fail(void)
{
    Test_SetAdcStartResult(HAL_ERROR);

    TEST_ASSERT_EQUAL(HAL_ERROR, IO_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetMutexNewCallCount());
}

void test_IO_Manager_Init_comp_start_fail(void)
{
    Test_SetAdcStartResult(HAL_OK);
    Test_SetCompStartResult(HAL_ERROR);

    TEST_ASSERT_EQUAL(HAL_ERROR, IO_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetMutexNewCallCount());
}

void test_IO_Manager_Init_mutex_fail(void)
{
    osMutexId_t results[7] = { (void*)0x1, (void*)0x2, (void*)0x3, (void*)0x4, (void*)0x5, NULL, (void*)0x7 };
    Test_SetMutexNewResults(results, 7);
    Test_SetAdcStartResult(HAL_OK);
    Test_SetCompStartResult(HAL_OK);

    TEST_ASSERT_EQUAL(HAL_ERROR, IO_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(6, Test_GetMutexNewCallCount());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_IO_Manager_Init_success);
    RUN_TEST(test_IO_Manager_Init_adc_start_fail);
    RUN_TEST(test_IO_Manager_Init_comp_start_fail);
    RUN_TEST(test_IO_Manager_Init_mutex_fail);

    return UNITY_END();
}
