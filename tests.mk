# Standalone unit test Makefile (host build only)

CC ?= gcc

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

TEST_DIR := tests
BUILD_DIR := $(TEST_DIR)/build
INCLUDE_DIRS := -I$(TEST_DIR)/include -ICore/Inc -ICore/Src

UNITY_SOURCES := $(TEST_DIR)/unity.c $(TEST_DIR)/cmock.c
COMMON_SOURCES := $(TEST_DIR)/stubs.c

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
$(if $(MODULE_SOURCE_$(1)),$(MODULE_SOURCE_$(1)),Core/Src/$(1).c)
endef

define make_test_rule
$(1): $(BUILD_DIR)/test_$(1)$(EXE)
	$(BUILD_DIR)/test_$(1)$(EXE)

$(BUILD_DIR)/test_$(1)$(EXE): $(UNITY_SOURCES) $(COMMON_SOURCES) $(TEST_DIR)/test_$(1).c $(call module_source,$(1))
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $$^ -o $$@
endef

$(foreach t,$(TESTS),$(eval $(call make_test_rule,$(t))))

clean:
	rm -rf $(BUILD_DIR)
