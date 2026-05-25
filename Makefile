# =========================================================
# GameAK Build System (Level 2 - Engine Grade Makefile)
# =========================================================

.PHONY: all tests clean debug release sanitize compile_commands

# -------------------------
# Toolchain
# -------------------------
CXX := clang++
AR  := ar

# -------------------------
# Parallel build
# -------------------------
cpus ?= $(shell nproc 2>/dev/null || echo 4)
MAKEFLAGS += -j$(cpus)

# -------------------------
# Directories
# -------------------------
SRC_DIR   := Src
TEST_DIR  := Tests
INC_DIR   := Include

BUILD_DIR := build
OBJ_DIR    := $(BUILD_DIR)/obj
BIN_DIR    := $(BUILD_DIR)/bin

# -------------------------
# Flags (base)
# -------------------------
COMMON_FLAGS := -std=c++20 \
                -I$(INC_DIR) -IThirdParty \
                -Wall -Wextra -Werror \
                -Wno-macro-redefined \
                -fno-exceptions -fno-rtti

# -------------------------
# Build profiles
# -------------------------
DEBUG_FLAGS   := -O0 -g
RELEASE_FLAGS := -O3 -DNDEBUG

SAN_FLAGS := -fsanitize=address,undefined,leak

# -------------------------
# Mode selection
# -------------------------
MODE ?= debug

ifeq ($(MODE),debug)
    BUILD_FLAGS := $(COMMON_FLAGS) $(DEBUG_FLAGS)
endif

ifeq ($(MODE),release)
    BUILD_FLAGS := $(COMMON_FLAGS) $(RELEASE_FLAGS)
endif

ifeq ($(MODE),sanitize)
    BUILD_FLAGS := $(COMMON_FLAGS) $(DEBUG_FLAGS) $(SAN_FLAGS)
endif

# -------------------------
# Sources (NO find, deterministic)
# -------------------------
SRC_SRCS  := $(shell find $(SRC_DIR) -name "*.cpp")
TEST_SRCS := $(shell find $(TEST_DIR) -name "*.cpp")

SRC_OBJS  := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/src/%.o,$(SRC_SRCS))
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/tests/%.o,$(TEST_SRCS))

DEPS := $(SRC_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# -------------------------
# Test binaries
# -------------------------
TEST_BINS := $(patsubst $(OBJ_DIR)/tests/%.o,$(BIN_DIR)/tests/%,$(TEST_OBJS))

# =========================================================
# RULES
# =========================================================

# -------------------------
# Compile src
# -------------------------
$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(BUILD_FLAGS) -MMD -MP -c $< -o $@

# -------------------------
# Compile tests
# -------------------------
$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(BUILD_FLAGS) -MMD -MP -c $< -o $@

# -------------------------
# Link tests
# -------------------------
$(BIN_DIR)/tests/%: $(OBJ_DIR)/tests/%.o $(SRC_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(BUILD_FLAGS) $^ -o $@

# -------------------------
# Run tests
# -------------------------
tests: $(TEST_BINS)
	@set -e; \
	for bin in $(TEST_BINS); do \
		./$$bin; \
	done

# -------------------------
# Compile Commands (clangd support)
# -------------------------
compile_commands:
	@echo "[" > compile_commands.json; \
	first=1; \
	for file in $(SRC_SRCS) $(TEST_SRCS); do \
		obj=$$(echo $$file | sed 's|^|$(BUILD_DIR)/obj/|' | sed 's|\.cpp|.o|'); \
		if [ $$first -eq 0 ]; then echo "," >> compile_commands.json; fi; \
		first=0; \
		echo "{ \"directory\": \"$(shell pwd)\", \"file\": \"$$file\", \"command\": \"$(CXX) $(BUILD_FLAGS) -c $$file -o $$obj\" }" >> compile_commands.json; \
	done; \
	echo "]" >> compile_commands.json

# -------------------------
# Build aliases
# -------------------------
debug:
	$(MAKE) MODE=debug tests

release:
	$(MAKE) MODE=release tests

sanitize:
	$(MAKE) MODE=sanitize tests

# -------------------------
# Clean
# -------------------------
clean:
	rm -rf $(BUILD_DIR)

# -------------------------
# Dependency inclusion
# -------------------------
-include $(DEPS)
