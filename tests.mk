# Standalone unit test Makefile (host build only)

CC ?= gcc

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

TEST_DIR := tests
BUILD_DIR := $(TEST_DIR)/build
INCLUDE_DIRS := -I$(TEST_DIR)/include -ICore/Inc -ICore/Inc/Managers -ICore/Inc/Drivers -ICore/Inc/Data -ICore/Inc/Config -ICore/Src

UNITY_SOURCES := $(TEST_DIR)/unity.c $(TEST_DIR)/cmock.c
COMMON_SOURCES := $(TEST_DIR)/stubs.c

# Override common sources for specific tests with COMMON_SOURCES_<name>
COMMON_SOURCES_bms_can_manager := $(TEST_DIR)/bms_can_stubs.c
MODULE_SOURCE_curr_sense := Core/Src/Drivers/curr_sense.c
MODULE_SOURCE_mcp2515 := Core/Src/Drivers/mcp2515.c

REQUESTED_TESTS := $(filter-out test clean,$(MAKECMDGOALS))
ifneq ($(REQUESTED_TESTS),)
TESTS := $(REQUESTED_TESTS)
else
TESTS := $(patsubst $(TEST_DIR)/test_%.c,%,$(wildcard $(TEST_DIR)/test_*.c))
endif

TEST_BINS := $(addprefix $(BUILD_DIR)/test_,$(addsuffix $(EXE),$(TESTS)))

CFLAGS += -std=c99 -Wall -Wextra $(INCLUDE_DIRS)
CFLAGS += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function

# Run in container:
#   docker build -f Dockerfile.test -t hvc-tests .
#   docker run --rm -v $(shell pwd):/work -w /work hvc-tests [module]

.PHONY: test clean $(TESTS)

test: $(TESTS)

# Override module source for a test with MODULE_SOURCE_<name>
define module_source
$(if $(MODULE_SOURCE_$(1)),$(MODULE_SOURCE_$(1)),Core/Src/Managers/$(1).c)
endef

define common_sources
$(if $(COMMON_SOURCES_$(1)),$(COMMON_SOURCES_$(1)),$(COMMON_SOURCES))
endef

define make_test_rule
$(1): $(BUILD_DIR)/test_$(1)$(EXE)
	$(BUILD_DIR)/test_$(1)$(EXE)

$(BUILD_DIR)/test_$(1)$(EXE): $(UNITY_SOURCES) $(call common_sources,$(1)) $(TEST_DIR)/test_$(1).c $(call module_source,$(1))
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $$^ -o $$@
endef

$(foreach t,$(TESTS),$(eval $(call make_test_rule,$(t))))

clean:
	rm -rf $(BUILD_DIR)
