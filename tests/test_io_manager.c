#include "unity.h"
#include "io_manager.h"
#include "test_stubs.h"
#include <setjmp.h>

static jmp_buf io_manager_loop_exit;
static uint32_t io_manager_delay_limit = 0U;

static void ExitIOManagerTaskAfterDelays(uint32_t ticks)
{
    (void)ticks;
    if (Test_GetDelayCallCount() >= io_manager_delay_limit) {
        longjmp(io_manager_loop_exit, 1);
    }
}

static void InitIOManagerForTaskTests(void)
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
}

static void RunIOManagerTaskIterations(uint32_t iterations)
{
    io_manager_delay_limit = iterations;
    Test_SetDelayHook(ExitIOManagerTaskAfterDelays);

    if (setjmp(io_manager_loop_exit) == 0) {
        IO_ManagerTask(NULL);
    }

    Test_SetDelayHook(NULL);
}

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
    TEST_ASSERT_FALSE(sdc.value);
    TEST_ASSERT_EQUAL_UINT32(0, sdc.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[1], imd.mutex);
    TEST_ASSERT_FALSE(imd.value);
    TEST_ASSERT_EQUAL_UINT32(0, imd.last_updated);

    TEST_ASSERT_EQUAL_PTR(results[2], bms_fault.mutex);
    TEST_ASSERT_FALSE(bms_fault.value);
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

void test_IO_ManagerTask_single_iteration_updates_io_and_sends_both_messages(void)
{
    const uint32_t adc_values[3] = { 1200U, 2400U, 1800U };
    const GPIO_PinState gpio_reads[2] = { GPIO_PIN_SET, GPIO_PIN_RESET };
    int32_t expected_low = Curr_CalculateCurrentSenseLow(adc_values[0]);
    int32_t expected_high = Curr_CalculateCurrentSenseHigh(adc_values[1]);

    InitIOManagerForTaskTests();
    bms_fault.value = true;
    Test_SetKernelTick(123U);
    Test_SetAdcValues(adc_values, 3);
    Test_SetGpioReadValues(gpio_reads, 2);

    RunIOManagerTaskIterations(1U);

    TEST_ASSERT_EQUAL_UINT32(2, Test_GetLvCanSendCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetLvCanSendCallCountForId(CAN_ID_IO_CURRENT));
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetLvCanSendCallCountForId(CAN_ID_IO_SUMMARY));
    TEST_ASSERT_EQUAL_UINT32(CAN_ID_IO_SUMMARY, Test_GetLastLvCanId());
    TEST_ASSERT_EQUAL_UINT8(7, Test_GetLastLvCanLength());
    TEST_ASSERT_EQUAL_UINT8(0x05, Test_GetLastLvCanDataByte(0));
    TEST_ASSERT_EQUAL_UINT8(0xC4, Test_GetLastLvCanDataByte(5));
    TEST_ASSERT_EQUAL_UINT8(0x09, Test_GetLastLvCanDataByte(6));

    TEST_ASSERT_EQUAL_UINT16(adc_values[0], cs_low_raw.value);
    TEST_ASSERT_EQUAL_UINT16(adc_values[1], cs_high_raw.value);
    TEST_ASSERT_EQUAL_UINT16(adc_values[2], therm.value);
    TEST_ASSERT_EQUAL_INT32(expected_low, (int32_t)cs_low.value);
    TEST_ASSERT_EQUAL_INT32(expected_high, (int32_t)cs_high.value);
    TEST_ASSERT_TRUE(sdc.value);
    TEST_ASSERT_FALSE(imd.value);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 25.0f, ref_temp.value);
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetGpioWriteCallCount());
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, Test_GetLastGpioWriteState());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDelayCallCount());
    TEST_ASSERT_EQUAL_UINT32(IO_PRIORITY_UPDATE_FREQ_MS, Test_GetLastDelayTicks());
}

void test_IO_ManagerTask_runs_priority_work_every_tick_and_regular_work_every_tenth_tick(void)
{
    uint32_t adc_values[23];
    uint32_t i = 0U;

    for (i = 0U; i < 23U; i++) {
        adc_values[i] = 1000U + i;
    }

    InitIOManagerForTaskTests();
    Test_SetAdcValues(adc_values, 23U);

    RunIOManagerTaskIterations(11U);

    TEST_ASSERT_EQUAL_UINT32(13, Test_GetLvCanSendCallCount());
    TEST_ASSERT_EQUAL_UINT32(11, Test_GetLvCanSendCallCountForId(CAN_ID_IO_CURRENT));
    TEST_ASSERT_EQUAL_UINT32(2, Test_GetLvCanSendCallCountForId(CAN_ID_IO_SUMMARY));
    TEST_ASSERT_EQUAL_UINT32(CAN_ID_IO_SUMMARY, Test_GetLastLvCanId());
    TEST_ASSERT_EQUAL_UINT32(11, Test_GetGpioWriteCallCount());
    TEST_ASSERT_EQUAL_UINT32(11, Test_GetDelayCallCount());
    TEST_ASSERT_EQUAL_UINT32(IO_PRIORITY_UPDATE_FREQ_MS, Test_GetLastDelayTicks());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_IO_Manager_Init_success);
    RUN_TEST(test_IO_Manager_Init_adc_start_fail);
    RUN_TEST(test_IO_Manager_Init_comp_start_fail);
    RUN_TEST(test_IO_Manager_Init_mutex_fail);
    RUN_TEST(test_IO_ManagerTask_single_iteration_updates_io_and_sends_both_messages);
    RUN_TEST(test_IO_ManagerTask_runs_priority_work_every_tick_and_regular_work_every_tenth_tick);

    return UNITY_END();
}
