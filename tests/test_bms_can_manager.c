#include "unity.h"
#include "bms_can_stubs.h"

void setUp(void)
{
    Test_Stubs_Reset();
}

void tearDown(void)
{
}

void test_BMS_CAN_ProcessRXMessage_module0(void)
{
    CAN_Message_t msg = {0x08F00101, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(45.6, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(47.8, acc[0]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_module1(void)
{
    CAN_Message_t msg = {0x08F01101, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(45.6, acc[1]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(47.8, acc[1]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_module2(void)
{
    CAN_Message_t msg = {0x08F02101, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(45.6, acc[2]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(47.8, acc[2]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_module3(void)
{
    CAN_Message_t msg = {0x08F03101, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(45.6, acc[3]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(47.8, acc[3]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_module4(void)
{
    CAN_Message_t msg = {0x08F04101, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(45.6, acc[4]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(47.8, acc[4]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_module5(void)
{
    CAN_Message_t msg = {0x08F05101, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(45.6, acc[5]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(47.8, acc[5]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_temp1(void)
{
    CAN_Message_t msg = {0x08F00101, {0x03, 0x01, 0x19, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(25.9, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(28.1, acc[0]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_temp2(void)
{
    CAN_Message_t msg = {0x08F00101, {0x3E, 0x02, 0x57, 0x02, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(57.4, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(59.9, acc[0]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_temp_error_value(void)
{
    CAN_Message_t msg = {0x08F00101, {0x0A, 0xFB, 0x0A, 0xFB, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_FLOAT(-127.0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(-127.0, acc[0]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_no_match(void)
{
    CAN_Message_t msg = {0x08F00202, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(0, Test_GetDispatchRegisterMatchCount());
}

void test_BMS_CAN_ProcessRXMessage_incomplete_data(void)
{
    CAN_Message_t msg = {0x08F00101, {0xC8, 0x01, 0xDE, 0x00, 0x00, 0x00, 0x00, 0x00}, 3, 2, 0x0};
    HAL_StatusTypeDef ret;

    ret = Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL(HAL_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_max);
}

void test_BMS_CAN_ProcessRXMessage_invalid_module(void)
{
    CAN_Message_t msg = {0x08F06101, {0xC8, 0x01, 0xDE, 0x01, 0xC4, 0x01, 0xD5, 0x01}, 8, 2, 0x0};
    HAL_StatusTypeDef ret;

    ret = Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL(HAL_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[0]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[1]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[2]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[3]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[4]->cell_temps.temp_max);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_min);
    TEST_ASSERT_EQUAL_FLOAT(0, acc[5]->cell_temps.temp_max);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_BMS_CAN_ProcessRXMessage_module0);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_module1);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_module2);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_module3);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_module4);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_module5);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_temp1);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_temp2);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_temp_error_value);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_incomplete_data);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_invalid_module);

    return UNITY_END();
}