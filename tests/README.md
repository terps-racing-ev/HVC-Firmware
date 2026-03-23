# Unit Tests (Host + Container)

This project uses standalone host tests built with Unity + CMock. Tests live in `tests/` and build with `tests.mk` without touching the firmware build.

## Add A Test

1. Create a new test file:

   `tests/test_<module>.c`

2. Each test file must include its own Unity runner:

```c
#include "unity.h"
#include "<module>.h"

void setUp(void) { }
void tearDown(void) { }

void test_example(void)
{
    TEST_ASSERT_TRUE(1);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_example);
    return UNITY_END();
}
```

3. If the module needs extra sources, add an override in `tests.mk`:

```
MODULE_SOURCE_<module> = Core/Src/<module>.c other_file.c
```

## Run On Host (Windows PowerShell)

Run a single module:

```
make -f tests.mk test <module>
```

Run all tests:

```
make -f tests.mk test
```

## Run In Container (Docker)

Build the image once:

```
docker build -f Dockerfile.test -t hvc-tests .
```

Run all tests:

```
docker rm -f hvc-tests 2>$null
docker run --name hvc-tests --rm -v ${PWD}:/work -w /work hvc-tests
```

Run a single module:

```
docker rm -f hvc-tests 2>$null
docker run --name hvc-tests --rm -v ${PWD}:/work -w /work hvc-tests <module>
```

Notes:
- In PowerShell, use `${PWD}` for volume mounts. In cmd.exe, `%cd%` works.
- The container name is fixed (`hvc-tests`) and is removed after each run (`--rm`).
- Each test file is built into its own binary: `tests/build/test_<module>`.
