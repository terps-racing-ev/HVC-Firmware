#include "unity.h"
#include "flash.h"

#include <stdint.h>
#include <string.h>

#define TEST_FLASH_PAGE_BYTES 0x1000u
#define TEST_FLASH_WORD_SIZE 4u
#define TEST_DOUBLEWORD_SIZE 8u

static uint8_t fake_flash_pages[TEST_FLASH_PAGE_BYTES * 2u];
static HAL_StatusTypeDef flash_unlock_status = HAL_OK;
static HAL_StatusTypeDef flash_lock_status = HAL_OK;
static HAL_StatusTypeDef flash_erase_status = HAL_OK;
static int32_t program_fail_on_call = -1;
static uint32_t program_call_count = 0;

static uint8_t *flash_addr_to_ptr(uint32_t addr)
{
    if (addr >= FLASH_SOC_PAGE_2_ADDRESS && addr < (FLASH_SOC_PAGE_2_ADDRESS + TEST_FLASH_PAGE_BYTES)) {
        return &fake_flash_pages[addr - FLASH_SOC_PAGE_2_ADDRESS];
    }

    if (addr >= FLASH_SOC_PAGE_3_ADDRESS && addr < (FLASH_SOC_PAGE_3_ADDRESS + TEST_FLASH_PAGE_BYTES)) {
        return &fake_flash_pages[TEST_FLASH_PAGE_BYTES + (addr - FLASH_SOC_PAGE_3_ADDRESS)];
    }

    return NULL;
}

uint32_t Flash_ReadWord(uint32_t addr)
{
    uint8_t *ptr = flash_addr_to_ptr(addr);

    if (ptr == NULL || ((addr % TEST_FLASH_WORD_SIZE) != 0u)) {
        return 0u;
    }

    uint32_t out = 0u;
    memcpy(&out, ptr, sizeof(out));
    return out;
}

HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
    return flash_unlock_status;
}

HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
    return flash_lock_status;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *pageError)
{
    (void)pageError;

    if (flash_erase_status != HAL_OK) {
        return flash_erase_status;
    }

    if (pEraseInit == NULL || pEraseInit->NbPages != 1u) {
        return HAL_ERROR;
    }

    if (pEraseInit->Page == ((FLASH_SOC_PAGE_2_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE)) {
        memset(&fake_flash_pages[0], 0xFF, TEST_FLASH_PAGE_BYTES);
        return HAL_OK;
    }

    if (pEraseInit->Page == ((FLASH_SOC_PAGE_3_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE)) {
        memset(&fake_flash_pages[TEST_FLASH_PAGE_BYTES], 0xFF, TEST_FLASH_PAGE_BYTES);
        return HAL_OK;
    }

    return HAL_ERROR;
}

HAL_StatusTypeDef HAL_FLASH_Program(uint32_t typeProgram, uint32_t address, uint64_t data)
{
    (void)typeProgram;

    uint8_t *ptr = flash_addr_to_ptr(address);
    if (ptr == NULL || ((address % TEST_DOUBLEWORD_SIZE) != 0u)) {
        return HAL_ERROR;
    }

    program_call_count++;
    if (program_fail_on_call >= 0 && (int32_t)program_call_count == program_fail_on_call) {
        return HAL_ERROR;
    }

    memcpy(ptr, &data, sizeof(data));
    return HAL_OK;
}

void setUp(void)
{
    memset(fake_flash_pages, 0xFF, sizeof(fake_flash_pages));
    flash_unlock_status = HAL_OK;
    flash_lock_status = HAL_OK;
    flash_erase_status = HAL_OK;
    program_fail_on_call = -1;
    program_call_count = 0;

    TEST_ASSERT_EQUAL(HAL_OK, Flash_Init());
}

void tearDown(void)
{
}

void test_Flash_WriteSOCData_and_ReadSOCData_round_trip(void)
{
    Flash_SOC_Data_t written = {
        .soc_total_capacity = 123456u,
        .soc_last_load_time = 10u,
        .soc_last_calculated = 77u,
        .soc_last_calculated_time = 20u
    };
    Flash_SOC_Data_t read = {0};

    TEST_ASSERT_EQUAL(HAL_OK, Flash_WriteSOCData(&written));
    TEST_ASSERT_EQUAL(HAL_OK, Flash_ReadSOCData(&read));

    TEST_ASSERT_EQUAL_UINT32(written.soc_total_capacity, read.soc_total_capacity);
    TEST_ASSERT_EQUAL_UINT32(written.soc_last_load_time, read.soc_last_load_time);
    TEST_ASSERT_EQUAL_UINT32(written.soc_last_calculated, read.soc_last_calculated);
    TEST_ASSERT_EQUAL_UINT32(written.soc_last_calculated_time, read.soc_last_calculated_time);
}

void test_Flash_WriteSOCData_power_loss_before_erase_keeps_last_valid(void)
{
    Flash_SOC_Data_t first = {
        .soc_total_capacity = 111u,
        .soc_last_load_time = 1u,
        .soc_last_calculated = 2u,
        .soc_last_calculated_time = 3u
    };
    Flash_SOC_Data_t second = {
        .soc_total_capacity = 222u,
        .soc_last_load_time = 4u,
        .soc_last_calculated = 5u,
        .soc_last_calculated_time = 6u
    };
    Flash_SOC_Data_t read = {0};

    TEST_ASSERT_EQUAL(HAL_OK, Flash_WriteSOCData(&first));

    flash_unlock_status = HAL_ERROR;
    TEST_ASSERT_EQUAL(HAL_ERROR, Flash_WriteSOCData(&second));

    TEST_ASSERT_EQUAL(HAL_OK, Flash_ReadSOCData(&read));
    TEST_ASSERT_EQUAL_UINT32(first.soc_total_capacity, read.soc_total_capacity);
    TEST_ASSERT_EQUAL_UINT32(first.soc_last_load_time, read.soc_last_load_time);
    TEST_ASSERT_EQUAL_UINT32(first.soc_last_calculated, read.soc_last_calculated);
    TEST_ASSERT_EQUAL_UINT32(first.soc_last_calculated_time, read.soc_last_calculated_time);
}

void test_Flash_WriteSOCData_power_loss_mid_program_after_erase_reverts_to_last_valid(void)
{
    Flash_SOC_Data_t first = {
        .soc_total_capacity = 333u,
        .soc_last_load_time = 7u,
        .soc_last_calculated = 8u,
        .soc_last_calculated_time = 9u
    };
    Flash_SOC_Data_t second = {
        .soc_total_capacity = 444u,
        .soc_last_load_time = 10u,
        .soc_last_calculated = 11u,
        .soc_last_calculated_time = 12u
    };
    Flash_SOC_Data_t read = {0};

    TEST_ASSERT_EQUAL(HAL_OK, Flash_WriteSOCData(&first));

    /* Fail before final commit doubleword is written. */
    program_fail_on_call = 2;
    program_call_count = 0;
    TEST_ASSERT_EQUAL(HAL_ERROR, Flash_WriteSOCData(&second));

    TEST_ASSERT_EQUAL(HAL_OK, Flash_ReadSOCData(&read));
    TEST_ASSERT_EQUAL_UINT32(first.soc_total_capacity, read.soc_total_capacity);
    TEST_ASSERT_EQUAL_UINT32(first.soc_last_load_time, read.soc_last_load_time);
    TEST_ASSERT_EQUAL_UINT32(first.soc_last_calculated, read.soc_last_calculated);
    TEST_ASSERT_EQUAL_UINT32(first.soc_last_calculated_time, read.soc_last_calculated_time);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Flash_WriteSOCData_and_ReadSOCData_round_trip);
    RUN_TEST(test_Flash_WriteSOCData_power_loss_before_erase_keeps_last_valid);
    RUN_TEST(test_Flash_WriteSOCData_power_loss_mid_program_after_erase_reverts_to_last_valid);
    return UNITY_END();
}
