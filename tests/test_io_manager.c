#include "unity.h"
#include "io_manager.h"
#include "test_stubs.h"
#include <setjmp.h>

static jmp_buf io_manager_loop_exit;
static jmp_buf io_priority_loop_exit;

static void ExitIOManagerTaskOnDelay(uint32_t ticks)
{
    (void)ticks;
    longjmp(io_manager_loop_exit, 1);
}

static void ExitIOPriorityManagerTaskOnDelay(uint32_t ticks)
{
    (void)ticks;
    longjmp(io_priority_loop_exit, 1);
}

static void RunIOManagerTaskSingleIteration(void)
{
    Test_SetDelayHook(ExitIOManagerTaskOnDelay);
    if (setjmp(io_manager_loop_exit) == 0) {
        IO_ManagerTask(NULL);
    }
    Test_SetDelayHook(NULL);
}

static void RunIOPriorityManagerTaskSingleIteration(void)
{
    Test_SetDelayHook(ExitIOPriorityManagerTaskOnDelay);
    if (setjmp(io_priority_loop_exit) == 0) {
        IO_PriorityManagerTask(NULL);
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
    TEST_ASSERT_EQUAL_UINT8(1, io_initialized);

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

void test_IO_Manager_Init_fails_for_each_mutex_creation_failure(void)
{
    const size_t mutex_count = 9;

    for (size_t fail_index = 0; fail_index < mutex_count; ++fail_index) {
        osMutexId_t results[9];
        for (size_t i = 0; i < mutex_count; ++i) {
            results[i] = (osMutexId_t)0x1;
        }
        results[fail_index] = NULL;

        Test_Stubs_Reset();
        Test_SetMutexNewResults(results, mutex_count);
        Test_SetAdcStartResult(HAL_OK);
        Test_SetCompStartResult(HAL_OK);

        TEST_ASSERT_EQUAL(HAL_ERROR, IO_Manager_Init());
        TEST_ASSERT_EQUAL_UINT32(fail_index + 1, Test_GetMutexNewCallCount());
    }
}

void test_IO_ReadADCChannel_returns_error_when_config_fails(void)
{
    uint16_t out = 0xBEEF;

    Test_SetAdcConfigResult(HAL_ERROR);
    Test_SetAdcStartResult(HAL_OK);
    Test_SetAdcPollResult(HAL_OK);

    TEST_ASSERT_EQUAL(HAL_ERROR, _IO_ReadADCChannel(ADC_CHANNEL_5, &out));
    TEST_ASSERT_EQUAL_UINT16(0xBEEF, out);
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcConfigCallCount());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetAdcStartCallCount());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetAdcPollCallCount());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetAdcGetValueCallCount());
}

void test_IO_ReadADCChannel_returns_error_when_start_fails(void)
{
    uint16_t out = 0xBEEF;

    Test_SetAdcConfigResult(HAL_OK);
    Test_SetAdcStartResult(HAL_ERROR);
    Test_SetAdcPollResult(HAL_OK);

    TEST_ASSERT_EQUAL(HAL_ERROR, _IO_ReadADCChannel(ADC_CHANNEL_5, &out));
    TEST_ASSERT_EQUAL_UINT16(0xBEEF, out);
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcConfigCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcStartCallCount());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetAdcPollCallCount());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetAdcGetValueCallCount());
}

void test_IO_ReadADCChannel_returns_error_when_poll_fails(void)
{
    uint16_t out = 0xBEEF;

    Test_SetAdcConfigResult(HAL_OK);
    Test_SetAdcStartResult(HAL_OK);
    Test_SetAdcPollResult(HAL_ERROR);

    TEST_ASSERT_EQUAL(HAL_ERROR, _IO_ReadADCChannel(ADC_CHANNEL_5, &out));
    TEST_ASSERT_EQUAL_UINT16(0xBEEF, out);
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcConfigCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcStartCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcPollCallCount());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetAdcGetValueCallCount());
}

void test_IO_ReadADCChannel_returns_value_on_success(void)
{
    uint16_t out = 0U;
    const uint32_t values[] = {0xBEEFU};

    Test_SetAdcConfigResult(HAL_OK);
    Test_SetAdcStartResult(HAL_OK);
    Test_SetAdcPollResult(HAL_OK);
    Test_SetAdcValueSequence(values, 1);

    TEST_ASSERT_EQUAL(HAL_OK, _IO_ReadADCChannel(ADC_CHANNEL_6, &out));
    TEST_ASSERT_EQUAL_UINT16(0xBEEF, out);
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcConfigCallCount());
    TEST_ASSERT_EQUAL_UINT32(ADC_CHANNEL_6, Test_GetAdcConfigChannel(0));
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcStartCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcPollCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcGetValueCallCount());
}

void test_IO_PackIOSummary_returns_early_when_data_null(void)
{
    uint8_t length = 0xAA;

    _IO_PackIOSummary(NULL, &length, 1U, 1U, 100U, 1U, 200U, 300U);
    TEST_ASSERT_EQUAL_UINT8(0xAA, length);
}

void test_IO_PackIOSummary_returns_early_when_length_null(void)
{
    uint8_t data[8] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

    _IO_PackIOSummary(data, NULL, 1U, 1U, 100U, 1U, 200U, 300U);
    for (size_t i = 0; i < 8; ++i) {
        TEST_ASSERT_EQUAL_UINT8(0xAA, data[i]);
    }
}

void test_IO_PackIOSummary_packs_expected_payload(void)
{
    uint8_t data[8] = {0};
    uint8_t length = 0U;

    _IO_PackIOSummary(data, &length, 1U, 0U, 0x04D2U, 1U, 0x1234U, 0xABCDU);

    TEST_ASSERT_EQUAL_UINT8(7U, length);
    TEST_ASSERT_EQUAL_UINT8(0x05U, data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x34U, data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x12U, data[2]);
    TEST_ASSERT_EQUAL_UINT8(0xCDU, data[3]);
    TEST_ASSERT_EQUAL_UINT8(0xABU, data[4]);
    TEST_ASSERT_EQUAL_UINT8(0xD2U, data[5]);
    TEST_ASSERT_EQUAL_UINT8(0x04U, data[6]);
    TEST_ASSERT_EQUAL_UINT8(0x00U, data[7]);
}

void test_IO_ManagerTask_reads_inputs_sends_summary_and_delays(void)
{
    const GPIO_PinState gpio_reads[] = {GPIO_PIN_SET, GPIO_PIN_RESET};
    const uint32_t adc_values[] = {1234U};

    Test_SetGpioReadSequence(gpio_reads, 2);
    Test_SetAdcValueSequence(adc_values, 1);
    Test_SetAdcConfigResult(HAL_OK);
    Test_SetAdcStartResult(HAL_OK);
    Test_SetAdcPollResult(HAL_OK);
    Test_SetThermResult(42.5f);
    Test_SetKernelTick(100);

    bms_fault.value = 1U;
    cs_low_raw.value = 0x1234U;
    cs_high_raw.value = 0xABCDU;

    RunIOManagerTaskSingleIteration();

    TEST_ASSERT_EQUAL_UINT8(GPIO_PIN_SET, sdc.value);
    TEST_ASSERT_EQUAL_UINT8(GPIO_PIN_RESET, imd.value);
    TEST_ASSERT_EQUAL_UINT16(1234U, therm.value);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 42.5f, ref_temp.value);
    TEST_ASSERT_EQUAL_UINT16(1234U, Test_GetThermLastAdc());

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetAdcConfigCallCount());
    TEST_ASSERT_EQUAL_UINT32(ADC_CHANNEL_8, Test_GetAdcConfigChannel(0));

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetLvCanSendCallCount());
    TEST_ASSERT_EQUAL_UINT32(CAN_ID_IO_SUMMARY, Test_GetLastLvCanId());
    TEST_ASSERT_EQUAL_UINT8(7, Test_GetLastLvCanLength());
    TEST_ASSERT_EQUAL_UINT8(0x05U, Test_GetLastLvCanDataByte(0));
    TEST_ASSERT_EQUAL_UINT8(0x34U, Test_GetLastLvCanDataByte(1));
    TEST_ASSERT_EQUAL_UINT8(0x12U, Test_GetLastLvCanDataByte(2));
    TEST_ASSERT_EQUAL_UINT8(0xCDU, Test_GetLastLvCanDataByte(3));
    TEST_ASSERT_EQUAL_UINT8(0xABU, Test_GetLastLvCanDataByte(4));
    TEST_ASSERT_EQUAL_UINT8(0xD2U, Test_GetLastLvCanDataByte(5));
    TEST_ASSERT_EQUAL_UINT8(0x04U, Test_GetLastLvCanDataByte(6));

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDelayCallCount());
    TEST_ASSERT_EQUAL_UINT32(IO_UPDATE_FREQ_MS, Test_GetLastDelayTicks());
}

void test_IO_PriorityManagerTask_sets_fault_pin_and_updates_currents(void)
{
    const uint32_t adc_values[] = {111U, 222U};

    Test_SetAdcValueSequence(adc_values, 2);
    Test_SetKernelTick(200);
    bms_fault.value = 1U;

    RunIOPriorityManagerTaskSingleIteration();

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetGpioWriteCallCount());
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, Test_GetLastGpioWriteState());
    TEST_ASSERT_EQUAL_UINT16(111U, cs_low_raw.value);
    TEST_ASSERT_EQUAL_UINT16(222U, cs_high_raw.value);
    TEST_ASSERT_EQUAL_INT32(111, cs_low.value);
    TEST_ASSERT_EQUAL_INT32(222, cs_high.value);
    TEST_ASSERT_EQUAL_UINT32(2, Test_GetAdcConfigCallCount());
    TEST_ASSERT_EQUAL_UINT32(ADC_CHANNEL_5, Test_GetAdcConfigChannel(0));
    TEST_ASSERT_EQUAL_UINT32(ADC_CHANNEL_6, Test_GetAdcConfigChannel(1));

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDelayCallCount());
    TEST_ASSERT_EQUAL_UINT32(IO_PRIORITY_UPDATE_FREQ_MS, Test_GetLastDelayTicks());
}

void test_IO_PriorityManagerTask_clears_fault_pin_when_fault_low(void)
{
    const uint32_t adc_values[] = {10U, 20U};

    Test_SetAdcValueSequence(adc_values, 2);
    bms_fault.value = 0U;

    RunIOPriorityManagerTaskSingleIteration();

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetGpioWriteCallCount());
    TEST_ASSERT_EQUAL(GPIO_PIN_RESET, Test_GetLastGpioWriteState());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_IO_Manager_Init_success);
    RUN_TEST(test_IO_Manager_Init_adc_start_fail);
    RUN_TEST(test_IO_Manager_Init_comp_start_fail);
    RUN_TEST(test_IO_Manager_Init_fails_for_each_mutex_creation_failure);
    RUN_TEST(test_IO_ReadADCChannel_returns_error_when_config_fails);
    RUN_TEST(test_IO_ReadADCChannel_returns_error_when_start_fails);
    RUN_TEST(test_IO_ReadADCChannel_returns_error_when_poll_fails);
    RUN_TEST(test_IO_ReadADCChannel_returns_value_on_success);
    RUN_TEST(test_IO_PackIOSummary_returns_early_when_data_null);
    RUN_TEST(test_IO_PackIOSummary_returns_early_when_length_null);
    RUN_TEST(test_IO_PackIOSummary_packs_expected_payload);
    RUN_TEST(test_IO_ManagerTask_reads_inputs_sends_summary_and_delays);
    RUN_TEST(test_IO_PriorityManagerTask_sets_fault_pin_and_updates_currents);
    RUN_TEST(test_IO_PriorityManagerTask_clears_fault_pin_when_fault_low);

    return UNITY_END();
}
