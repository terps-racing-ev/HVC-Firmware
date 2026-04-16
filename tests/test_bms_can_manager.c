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

    TEST_ASSERT_EQUAL_UINT32(0, Test_GetDispatchRegisterMatchCount());
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

void test_BMS_CAN_ProcessRXMessage_amb_temps(void)
{
    CAN_Message_t msg = {0x08F0000D, {0xC4, 0x01, 0xD5, 0x01, 0xC8, 0x01, 0xDE, 0x01}, 8, 2, 0x0};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_INT16(456, acc[0]->amb_temps.amb_temp_1_Cx10);
    TEST_ASSERT_EQUAL_INT16(478, acc[0]->amb_temps.amb_temp_2_Cx10);

}

void test_BMS_CAN_ProcessRXMessage_heartbeat(void)
{
    CAN_Message_t msg = {0x08F00300, {0xC4, 0x01, 0xD5, 0x01, 0xC8, 0x01, 0xDE, 0x01}, 8, 0, 0x2EFE};

    Test_BMS_CAN_ProcessRXMessage(&msg);

    TEST_ASSERT_EQUAL_UINT32(1, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_UINT32(12030, acc[0]->heartbeat_last_update);

}

void test_BMS_CAN_ProcessRXMessage_voltages(void)
{
    CAN_Message_t msg0 = {0x08F00200, {0x18, 0x10, 0xDE, 0x0D, 0x82, 0x0F, 0x00, 0x00}, 6, 2, 0x0}; // 4120, 3550, 3970
    CAN_Message_t msg1 = {0x08F00201, {0x86, 0x10, 0x96, 0x0F, 0x1C, 0x11, 0x00, 0x00}, 6, 2, 0x0}; // 4230, 3990, 4380
    CAN_Message_t msg2 = {0x08F00202, {0x32, 0x0F, 0x54, 0x10, 0xA0, 0x0F, 0x00, 0x00}, 6, 2, 0x0}; // 3890, 4180, 4000  

    Test_BMS_CAN_ProcessRXMessage(&msg0);
    Test_BMS_CAN_ProcessRXMessage(&msg1);
    Test_BMS_CAN_ProcessRXMessage(&msg2);

    TEST_ASSERT_EQUAL_UINT32(3, Test_GetDispatchRegisterMatchCount());
    TEST_ASSERT_EQUAL_UINT16(3550, acc[0]->cell_voltages.volt_min_mV);
    TEST_ASSERT_EQUAL_UINT8(2, acc[0]->cell_voltages.volt_min_cell_id);
    TEST_ASSERT_EQUAL_UINT16(4380, acc[0]->cell_voltages.volt_max_mV);
    TEST_ASSERT_EQUAL_UINT8(6, acc[0]->cell_voltages.volt_max_cell_id);

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
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_amb_temps);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_heartbeat);
    RUN_TEST(test_BMS_CAN_ProcessRXMessage_voltages);

    return UNITY_END();
}