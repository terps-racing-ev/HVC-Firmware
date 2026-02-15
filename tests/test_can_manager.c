#include "unity.h"
#include "can_manager.h"
#include "test_stubs.h"

void setUp(void)
{
    Test_Stubs_Reset();
}

void tearDown(void)
{
}

void test_CAN_Manager_Init_success(void)
{
    osMessageQueueId_t results[4] = { (void*)0x1, (void*)0x2, (void*)0x3, (void*)0x4 };
    Test_SetQueueNewResults(results, 4);
    Test_SetCanActivateNotificationResult(HAL_OK);

    TEST_ASSERT_EQUAL(HAL_OK, CAN_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(4, Test_GetQueueNewCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetCanActivateNotificationCallCount());
}

void test_CAN_Manager_Init_queue_fail(void)
{
    osMessageQueueId_t results[4] = { (void*)0x1, NULL, (void*)0x3, (void*)0x4 };
    Test_SetQueueNewResults(results, 4);
    Test_SetCanActivateNotificationResult(HAL_OK);

    TEST_ASSERT_EQUAL(HAL_ERROR, CAN_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(2, Test_GetQueueNewCallCount());
    TEST_ASSERT_EQUAL_UINT32(0, Test_GetCanActivateNotificationCallCount());
}

void test_CAN_Manager_Init_can_notify_fail(void)
{
    osMessageQueueId_t results[4] = { (void*)0x1, (void*)0x2, (void*)0x3, (void*)0x4 };
    Test_SetQueueNewResults(results, 4);
    Test_SetCanActivateNotificationResult(HAL_ERROR);

    TEST_ASSERT_EQUAL(HAL_ERROR, CAN_Manager_Init());
    TEST_ASSERT_EQUAL_UINT32(4, Test_GetQueueNewCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetCanActivateNotificationCallCount());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_CAN_Manager_Init_success);
    RUN_TEST(test_CAN_Manager_Init_queue_fail);
    RUN_TEST(test_CAN_Manager_Init_can_notify_fail);

    return UNITY_END();
}
