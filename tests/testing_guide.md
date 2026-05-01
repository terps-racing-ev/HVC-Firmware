# Testing Guide

This document provides guidelines and instructions for writing and maintaining C module tests in the HVC-Firmware project. By adopting these standards, you ensure tests remain clean, DRY, and maintainable.

## Using Unity
We use [Unity](http://www.throwtheswitch.org/unity) as our C unit testing framework. 
- Use standard `TEST_ASSERT_xxx` macros instead of rolling your own checks.
- Keep tests self-contained; ensure global state is correctly reset in `setUp()`.
- Only `include "unity.h"` and the components you need.

## Setup and Stubs Reset
The function `Test_Stubs_Reset()` clears out mock state and should be called in every `setUp()`:
```c
void setUp(void)
{
    Test_Stubs_Reset();
    // Additional domain-specific initialization here
}
```
**Important**: Any new persistent stub variables you introduce must be reset inside `Test_Stubs_Reset()` located in `stubs.c` to prevent side effects across unit tests. Avoid fragmenting reset logic into multiple stub-specific files.

## Mocking with Stubs
Prefer centralized stubs over module-specific overrides.
- Use `Test_SetMutexNewResults()`, `Test_SetQueueNewResults()`, etc., for standard RTOS objects.
- Prefer array literals combined with an `ARRAY_SIZE` macro when providing arrays to standard mock results.
- Mock interactions with HAL peripherals directly in `stubs.c`.

## Avoiding Code Duplication
- **Parameterized Tests**: Instead of duplicating 5+ methods that test exact same behaviors on different IDs or items, structure your test with an internal loop.
- **Data-Driven Context**: If a test iterates over several points, pack your parameters into a struct array or standard macro array, then map them through the assertions (e.g. testing ADC conversion across various scale points).

## General Style Guidelines
1. **Magic Numbers**: Do not hardcode literal CAN data sequences or ADC values. Where possible, define standard ranges in macros (`#define ADC_ZERO_CROSSING_VAL 1552U`).
2. **Library Standard Avoidance**: Include standard library tools (e.g., `<stdlib.h>` for `abs()` or `labs()`) rather than re-implementing basic behavior manually.
3. **Encapsulation**: Treat module-internal fields as much "black-box" as you can unless directly exposed via variables (like `acc`).
4. **Sign Consistency**: Match assertion types (e.g., `TEST_ASSERT_EQUAL_INT32`) to the return types to avoid implicit sign-extension bugs or casting. Make use of `_UINT32`, `_FLOAT`, `_INT16` assertions selectively.