#include "unity.h"
#include "test_stubs.h"
#include "state_manager.h"
#include <setjmp.h>

static jmp_buf state_manager_loop_exit;

static void ExitStateManagerTaskOnDelay(uint32_t ticks)
{
    (void)ticks;
    longjmp(state_manager_loop_exit, 1);
}

static void RunStateManagerTaskSingleIteration(void)
{
    Test_SetDelayHook(ExitStateManagerTaskOnDelay);

    if (setjmp(state_manager_loop_exit) == 0) {
        State_ManagerTask(NULL);
    }

    Test_SetDelayHook(NULL);
}

static void SetAllHeartbeats(uint32_t tick)
{
    for (int i = 0; i < NUM_ACC_MODULES; i++) {
        if (acc[i] != NULL) {
            acc[i]->heartbeat_last_update = tick;
        }
    }
}

static void AssertCurrentState(State expected)
{
    State current = PRE_INIT;
    State_GetState(&current);
    TEST_ASSERT_EQUAL(expected, current);
}

static void AssertErrorMask(ErrorMask expected)
{
    ErrorMask mask = 0U;
    State_GetErrorMask(&mask);
    TEST_ASSERT_EQUAL_HEX32(expected, mask);
}

void setUp(void) {
    Test_Stubs_Reset();
    State_SetState(PRE_INIT);
    State_SetErrorMask(0U);
}

void tearDown(void){
}

void test_STATE_MANAGER_Init_success(void)
{
    osMutexId_t results[1] = {(osMutexId_t)0x1};
    Test_SetMutexNewResults(results, 1);

    TEST_ASSERT_EQUAL(HAL_OK, State_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetMutexNewCallCount());
    TEST_ASSERT_EQUAL_PTR(results[0], bms_state.mutex);
    TEST_ASSERT_EQUAL(PRE_INIT, bms_state.state);
}

void test_STATE_MANAGER_Init_fails_when_mutex_creation_fails(void)
{
    osMutexId_t results[1] = {NULL};
    Test_SetMutexNewResults(results, 1);

    TEST_ASSERT_EQUAL(HAL_ERROR, State_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetMutexNewCallCount());
    TEST_ASSERT_EQUAL_PTR(NULL, bms_state.mutex);
}

void test_STATE_MANAGER_Transition_pre_init_stays_when_dependencies_missing(void)
{
    io_initialized = 1;
    bms_can_initialized = 1;
    lv_can_initialized = 0;

    RunStateManagerTaskSingleIteration();

    AssertCurrentState(PRE_INIT);
    AssertErrorMask(0U);
}

void test_STATE_MANAGER_Transition_pre_init_moves_to_ok_when_all_initialized(void)
{
    io_initialized = 1;
    bms_can_initialized = 1;
    lv_can_initialized = 1;

    RunStateManagerTaskSingleIteration();

    AssertCurrentState(OK);
    AssertErrorMask(0U);
}

void test_STATE_MANAGER_Transition_ok_stays_ok_when_no_faults(void)
{
    State_SetState(OK);
    Test_SetKernelTick(1000);
    ref_temp.value = 25.0f;
    SetAllHeartbeats(950); // 50 ticks old, below cutoff

    RunStateManagerTaskSingleIteration();

    AssertCurrentState(OK);
    AssertErrorMask(0U);
}

void test_STATE_MANAGER_Transition_ok_to_errored_when_ref_overtemp(void)
{
    State_SetState(OK);
    Test_SetKernelTick(500);
    ref_temp.value = 60.1f;
    SetAllHeartbeats(500); // keep heartbeat fresh so only temp fault is active

    RunStateManagerTaskSingleIteration();

    AssertCurrentState(ERRORED);
    AssertErrorMask((ErrorMask)1U << BMS_ERR_REF_OVER_TEMP);
}

void test_STATE_MANAGER_Transition_ok_to_errored_when_any_heartbeat_is_stale(void)
{
    State_SetState(OK);
    Test_SetKernelTick(1000);
    ref_temp.value = 20.0f;
    SetAllHeartbeats(950);
    acc[2]->heartbeat_last_update = 899; // 101 ticks old

    RunStateManagerTaskSingleIteration();

    AssertCurrentState(ERRORED);
    AssertErrorMask((ErrorMask)1U << BMS_ERR_MODULE_TIMEOUT);
}

void test_STATE_MANAGER_Transition_ok_to_errored_when_both_faults_present(void)
{
    State_SetState(OK);
    Test_SetKernelTick(1000);
    ref_temp.value = 75.0f;
    SetAllHeartbeats(1000);
    acc[0]->heartbeat_last_update = 0;

    RunStateManagerTaskSingleIteration();

    AssertCurrentState(ERRORED);
    AssertErrorMask(((ErrorMask)1U << BMS_ERR_REF_OVER_TEMP) |
                    ((ErrorMask)1U << BMS_ERR_MODULE_TIMEOUT));
}

void test_STATE_MANAGER_Transition_errored_to_ok_when_faults_clear(void)
{
    State_SetState(ERRORED);
    State_SetErrorMask(((ErrorMask)1U << BMS_ERR_REF_OVER_TEMP) |
                       ((ErrorMask)1U << BMS_ERR_MODULE_TIMEOUT));

    Test_SetKernelTick(1200);
    ref_temp.value = 25.0f;
    SetAllHeartbeats(1200);

    RunStateManagerTaskSingleIteration();

    AssertCurrentState(OK);
    AssertErrorMask(0U);
}

void test_STATE_MANAGER_Task_sends_state_on_both_buses_and_delays(void)
{
    State_SetState(OK);
    Test_SetKernelTick(50);
    ref_temp.value = 25.0f;
    SetAllHeartbeats(50);

    RunStateManagerTaskSingleIteration();

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetBmsCanSendCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetLvCanSendCallCount());
    TEST_ASSERT_EQUAL_UINT32(CAN_ID_STATE, Test_GetLastBmsCanId());
    TEST_ASSERT_EQUAL_UINT32(CAN_ID_STATE, Test_GetLastLvCanId());
    TEST_ASSERT_EQUAL_UINT8(1, Test_GetLastBmsCanLength());
    TEST_ASSERT_EQUAL_UINT8(1, Test_GetLastLvCanLength());
    TEST_ASSERT_EQUAL_UINT8((uint8_t)OK, Test_GetLastBmsCanDataByte(0));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)OK, Test_GetLastLvCanDataByte(0));
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDelayCallCount());
    TEST_ASSERT_EQUAL_UINT32(STATE_REFRESH_FREQ_MS, Test_GetLastDelayTicks());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_STATE_MANAGER_Init_success);
    RUN_TEST(test_STATE_MANAGER_Init_fails_when_mutex_creation_fails);
    RUN_TEST(test_STATE_MANAGER_Transition_pre_init_stays_when_dependencies_missing);
    RUN_TEST(test_STATE_MANAGER_Transition_pre_init_moves_to_ok_when_all_initialized);
    RUN_TEST(test_STATE_MANAGER_Transition_ok_stays_ok_when_no_faults);
    RUN_TEST(test_STATE_MANAGER_Transition_ok_to_errored_when_ref_overtemp);
    RUN_TEST(test_STATE_MANAGER_Transition_ok_to_errored_when_any_heartbeat_is_stale);
    RUN_TEST(test_STATE_MANAGER_Transition_ok_to_errored_when_both_faults_present);
    RUN_TEST(test_STATE_MANAGER_Transition_errored_to_ok_when_faults_clear);
    RUN_TEST(test_STATE_MANAGER_Task_sends_state_on_both_buses_and_delays);
    return UNITY_END();
}

