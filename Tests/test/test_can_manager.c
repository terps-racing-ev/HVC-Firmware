#include "unity.h"
#include "can_manager.h"
#include "stub_platform.h"

void setUp(void)
{
    stub_platform_reset();
}

void tearDown(void)
{
}

void test_CAN_Manager_Init_returns_error_when_first_queue_fails(void)
{
    stub_queue_new_returns[0] = NULL;
    TEST_ASSERT_EQUAL(HAL_ERROR, CAN_Manager_Init());
}

void test_CAN_Manager_Init_returns_error_when_second_queue_fails(void)
{
    stub_queue_new_returns[0] = (osMessageQueueId_t)0x1;
    stub_queue_new_returns[1] = NULL;
    TEST_ASSERT_EQUAL(HAL_ERROR, CAN_Manager_Init());
}

void test_CAN_Manager_Init_returns_ok_when_queues_and_can_ok(void)
{
    stub_queue_new_returns[0] = (osMessageQueueId_t)0x1;
    stub_queue_new_returns[1] = (osMessageQueueId_t)0x2;
    stub_can_activate_status = HAL_OK;
    TEST_ASSERT_EQUAL(HAL_OK, CAN_Manager_Init());
}

void test_CAN_Manager_Init_returns_error_when_can_notification_fails(void)
{
    stub_queue_new_returns[0] = (osMessageQueueId_t)0x1;
    stub_queue_new_returns[1] = (osMessageQueueId_t)0x2;
    stub_can_activate_status = HAL_ERROR;
    TEST_ASSERT_EQUAL(HAL_ERROR, CAN_Manager_Init());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_CAN_Manager_Init_returns_error_when_first_queue_fails);
    RUN_TEST(test_CAN_Manager_Init_returns_error_when_second_queue_fails);
    RUN_TEST(test_CAN_Manager_Init_returns_ok_when_queues_and_can_ok);
    RUN_TEST(test_CAN_Manager_Init_returns_error_when_can_notification_fails);
    return UNITY_END();
}
