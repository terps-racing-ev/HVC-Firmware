#include "unity.h"
#include "mcp2515.h"
#include "test_stubs.h"

void setUp(void)
{
    Test_Stubs_Reset();
    Test_ResetSpiCounts();
}

void tearDown(void) {}

/* ── MCP2515_Initialize ──────────────────────────────────────────────────── */

void test_MCP2515_Initialize_spi_ready(void)
{
    /* HAL_SPI_GetState stub always returns READY, so should return true */
    TEST_ASSERT_TRUE(MCP2515_Initialize());
}

/* ── MCP2515_SetConfigMode ───────────────────────────────────────────────── */

void test_MCP2515_SetConfigMode_success(void)
{
    /* ReadByte (SPI_Rx) returns 0x80 → (0x80 & 0xE0) == 0x80 → config mode */
    Test_SetSpiReceiveResult(HAL_OK, 0x80);
    TEST_ASSERT_TRUE(MCP2515_SetConfigMode());
}

void test_MCP2515_SetConfigMode_fail(void)
{
    /* ReadByte returns wrong value — never confirms config mode */
    Test_SetSpiReceiveResult(HAL_OK, 0x00);
    TEST_ASSERT_FALSE(MCP2515_SetConfigMode());
}

/* ── MCP2515_SetNormalMode ───────────────────────────────────────────────── */

void test_MCP2515_SetNormalMode_success(void)
{
    /* ReadByte returns 0x00 → (0x00 & 0xE0) == 0x00 → normal mode */
    Test_SetSpiReceiveResult(HAL_OK, 0x00);
    TEST_ASSERT_TRUE(MCP2515_SetNormalMode());
}

void test_MCP2515_SetNormalMode_fail(void)
{
    /* ReadByte returns 0x80 — stays in config mode, never confirms normal */
    Test_SetSpiReceiveResult(HAL_OK, 0x80);
    TEST_ASSERT_FALSE(MCP2515_SetNormalMode());
}

/* ── MCP2515_SetSleepMode ────────────────────────────────────────────────── */

void test_MCP2515_SetSleepMode_success(void)
{
    /* ReadByte returns 0x20 → (0x20 & 0xE0) == 0x20 → sleep mode */
    Test_SetSpiReceiveResult(HAL_OK, 0x20);
    TEST_ASSERT_TRUE(MCP2515_SetSleepMode());
}

void test_MCP2515_SetSleepMode_fail(void)
{
    Test_SetSpiReceiveResult(HAL_OK, 0x00);
    TEST_ASSERT_FALSE(MCP2515_SetSleepMode());
}

/* ── MCP2515_Reset ───────────────────────────────────────────────────────── */

void test_MCP2515_Reset_sends_one_byte(void)
{
    MCP2515_Reset();
    /* Reset sends exactly 1 SPI byte (the RESET command) */
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetSpiTransmitCallCount());
}

/* ── MCP2515_ReadByte ────────────────────────────────────────────────────── */

void test_MCP2515_ReadByte_sends_two_bytes(void)
{
    Test_SetSpiReceiveResult(HAL_OK, 0xAB);
    MCP2515_ReadByte(0x0E);
    /* READ command + address = 2 transmit calls */
    TEST_ASSERT_EQUAL_UINT32(2, Test_GetSpiTransmitCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetSpiReceiveCallCount());
}

void test_MCP2515_ReadByte_returns_rx_value(void)
{
    Test_SetSpiReceiveResult(HAL_OK, 0xAB);
    uint8_t val = MCP2515_ReadByte(0x0E);
    TEST_ASSERT_EQUAL_UINT8(0xAB, val);
}

/* ── MCP2515_WriteByte ───────────────────────────────────────────────────── */

void test_MCP2515_WriteByte_sends_three_bytes(void)
{
    MCP2515_WriteByte(0x0F, 0x55);
    /* WRITE command + address + data = 3 transmit calls */
    TEST_ASSERT_EQUAL_UINT32(3, Test_GetSpiTransmitCallCount());
}

/* ── MCP2515_ReadStatus ──────────────────────────────────────────────────── */

void test_MCP2515_ReadStatus_sends_one_receives_one(void)
{
    Test_SetSpiReceiveResult(HAL_OK, 0x7F);
    uint8_t status = MCP2515_ReadStatus();
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetSpiTransmitCallCount());
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetSpiReceiveCallCount());
    TEST_ASSERT_EQUAL_UINT8(0x7F, status);
}

/* ── MCP2515_GetRxStatus ─────────────────────────────────────────────────── */

void test_MCP2515_GetRxStatus_returns_value(void)
{
    Test_SetSpiReceiveResult(HAL_OK, 0x42);
    uint8_t status = MCP2515_GetRxStatus();
    TEST_ASSERT_EQUAL_UINT8(0x42, status);
}

/* ── MCP2515_BitModify ───────────────────────────────────────────────────── */

void test_MCP2515_BitModify_sends_four_bytes(void)
{
    MCP2515_BitModify(0x2C, 0x03, 0x01);
    /* BIT_MOD + address + mask + data = 4 transmit calls */
    TEST_ASSERT_EQUAL_UINT32(4, Test_GetSpiTransmitCallCount());
}

/* ── MCP2515_RequestToSend ───────────────────────────────────────────────── */

void test_MCP2515_RequestToSend_sends_one_byte(void)
{
    MCP2515_RequestToSend(0x81);
    TEST_ASSERT_EQUAL_UINT32(1, Test_GetSpiTransmitCallCount());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_MCP2515_Initialize_spi_ready);
    RUN_TEST(test_MCP2515_SetConfigMode_success);
    RUN_TEST(test_MCP2515_SetConfigMode_fail);
    RUN_TEST(test_MCP2515_SetNormalMode_success);
    RUN_TEST(test_MCP2515_SetNormalMode_fail);
    RUN_TEST(test_MCP2515_SetSleepMode_success);
    RUN_TEST(test_MCP2515_SetSleepMode_fail);
    RUN_TEST(test_MCP2515_Reset_sends_one_byte);
    RUN_TEST(test_MCP2515_ReadByte_sends_two_bytes);
    RUN_TEST(test_MCP2515_ReadByte_returns_rx_value);
    RUN_TEST(test_MCP2515_WriteByte_sends_three_bytes);
    RUN_TEST(test_MCP2515_ReadStatus_sends_one_receives_one);
    RUN_TEST(test_MCP2515_GetRxStatus_returns_value);
    RUN_TEST(test_MCP2515_BitModify_sends_four_bytes);
    RUN_TEST(test_MCP2515_RequestToSend_sends_one_byte);

    return UNITY_END();
}
