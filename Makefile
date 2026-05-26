# =========================================================
# GameAK Build System
# =========================================================

.PHONY: all tests clean debug release sanitize compile_commands lib install uninstall format tidy

.DELETE_ON_ERROR:

.DEFAULT_GOAL := all

# Toolchain
CXX := clang++
AR  := ar

# Parallel build
cpus ?= $(shell nproc 2>/dev/null || echo 4)
MAKEFLAGS += -j$(cpus)

# Directories
SRC_DIR   := Src
TEST_DIR  := Tests
INC_DIR   := Include
THIRD_DIR := ThirdParty

BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
BIN_DIR   := $(BUILD_DIR)/bin
LIB_DIR   := $(BUILD_DIR)/lib
LIB_PATH  := $(LIB_DIR)/libGameAK.a

# Flags (base)
COMMON_FLAGS := -std=c++20 \
                -I$(INC_DIR) -I$(THIRD_DIR) \
                -Wall -Wextra -Werror \
                -Wno-macro-redefined \
                -fno-exceptions -fno-rtti

TEST_FLAGS := -DCEST_ENABLE_FORK

# Build profiles
DEBUG_FLAGS   := -O0 -g -DGAMEAK_DEBUG_VALIDATE
RELEASE_FLAGS := -O3 -DNDEBUG
SAN_FLAGS     := -fsanitize=address,undefined,leak

# Mode selection
MODE ?= debug

ifeq ($(MODE),debug)
  MODE_FLAGS := $(DEBUG_FLAGS)
else ifeq ($(MODE),release)
  MODE_FLAGS := $(RELEASE_FLAGS)
else ifeq ($(MODE),sanitize)
  MODE_FLAGS := $(DEBUG_FLAGS) $(SAN_FLAGS)
else
  $(error Unknown MODE=$(MODE). Use debug, release, or sanitize.)
endif

BUILD_FLAGS      := $(COMMON_FLAGS) $(MODE_FLAGS)
TEST_BUILD_FLAGS := $(COMMON_FLAGS) $(TEST_FLAGS) $(MODE_FLAGS)

# Sources (deterministic ordering)
SRC_SRCS    := $(shell find $(SRC_DIR) -name '*.cpp' | sort)
TEST_SRCS   := $(shell find $(TEST_DIR) -name '*.cpp' | sort)
SRC_HEADERS := $(shell find $(INC_DIR) -name '*.hpp' | sort)

SRC_OBJS  := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/src/%.o,$(SRC_SRCS))
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/tests/%.o,$(TEST_SRCS))
TEST_BINS := $(patsubst $(OBJ_DIR)/tests/%.o,$(BIN_DIR)/tests/%,$(TEST_OBJS))

DEPS := $(SRC_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Installation
INSTALL_PREFIX ?= /usr/local
GAMEAK_VERSION ?= 0.1.0

# Secondary (keep intermediate .o files for caching)
.SECONDARY: $(SRC_OBJS) $(TEST_OBJS)

# Compilation commands (separate flags for src vs tests)
COMPILE_SRC  = $(CXX) $(BUILD_FLAGS) -MMD -MP -c $< -o $@
COMPILE_TEST = $(CXX) $(TEST_BUILD_FLAGS) -MMD -MP -c $< -o $@

# =========================================================
# Compilation rules
# =========================================================

$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(COMPILE_SRC)

$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(@D)
	$(COMPILE_TEST)

# =========================================================
# Static library
# =========================================================

lib: $(LIB_PATH)

$(LIB_PATH): $(SRC_OBJS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

# =========================================================
# Test binaries (linked against libGameAK.a)
# =========================================================

$(BIN_DIR)/tests/%: $(OBJ_DIR)/tests/%.o $(SRC_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(TEST_BUILD_FLAGS) $< $(SRC_OBJS) -o $@

# =========================================================
# Test runner (runs all, reports summary)
# =========================================================

tests: $(TEST_BINS)
	@passed=0; failed=0; \
	for bin in $(TEST_BINS); do \
		if ./$$bin; then \
			passed=$$((passed + 1)); \
		else \
			failed=$$((failed + 1)); \
		fi; \
	done; \
	echo "=== $$passed passed, $$failed failed ==="; \
	[ $$failed -eq 0 ]

# =========================================================
# Install / Uninstall
# =========================================================

install: $(LIB_PATH)
	install -d $(DESTDIR)$(INSTALL_PREFIX)/include/
	cp -r $(INC_DIR)/AK $(DESTDIR)$(INSTALL_PREFIX)/include/
	install -d $(DESTDIR)$(INSTALL_PREFIX)/lib/
	install -m 644 $(LIB_PATH) $(DESTDIR)$(INSTALL_PREFIX)/lib/
	install -d $(DESTDIR)$(INSTALL_PREFIX)/lib/pkgconfig/
	sed 's|@prefix@|$(INSTALL_PREFIX)|g; s|@version@|$(GAMEAK_VERSION)|g' \
	  GameAK.pc.in > $(DESTDIR)$(INSTALL_PREFIX)/lib/pkgconfig/GameAK.pc

uninstall:
	rm -rf $(DESTDIR)$(INSTALL_PREFIX)/include/AK
	rm -f $(DESTDIR)$(INSTALL_PREFIX)/lib/libGameAK.a
	rm -f $(DESTDIR)$(INSTALL_PREFIX)/lib/pkgconfig/GameAK.pc

# =========================================================
# Build aliases
# =========================================================

all: tests
	$(MAKE) lib

debug:
	$(MAKE) MODE=debug all

release:
	$(MAKE) MODE=release all

sanitize:
	$(MAKE) MODE=sanitize all

# =========================================================
# Code quality
# =========================================================

FORMAT_FILES := $(SRC_SRCS) $(TEST_SRCS) $(SRC_HEADERS)

format:
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(FORMAT_FILES) && \
		echo "Formatted $(words $(FORMAT_FILES)) files"; \
	else \
		echo "clang-format not found, skipping"; \
	fi

tidy:
	@if command -v clang-tidy >/dev/null 2>&1; then \
		clang-tidy $(SRC_SRCS) -- $(BUILD_FLAGS) && \
		clang-tidy $(TEST_SRCS) -- $(TEST_BUILD_FLAGS); \
	else \
		echo "clang-tidy not found, skipping"; \
	fi

# =========================================================
# Compile Commands (clangd support)
# =========================================================

compile_commands:
	@printf '[' > compile_commands.json; \
	first=1; \
	for src in $(SRC_SRCS); do \
		obj="$(OBJ_DIR)/src/$$(echo $$src | sed 's|^$(SRC_DIR)/||; s|\.cpp$$|.o|')"; \
		if [ $$first -eq 0 ]; then printf ',' >> compile_commands.json; fi; \
		first=0; \
		printf '\n  {\n    "directory": "$(CURDIR)",\n    "file": "%s",\n    "command": "$(CXX) $(BUILD_FLAGS) -c %s -o %s"\n  }' \
			"$$src" "$$src" "$$obj" >> compile_commands.json; \
	done; \
	for src in $(TEST_SRCS); do \
		obj="$(OBJ_DIR)/tests/$$(echo $$src | sed 's|^$(TEST_DIR)/||; s|\.cpp$$|.o|')"; \
		printf ',' >> compile_commands.json; \
		printf '\n  {\n    "directory": "$(CURDIR)",\n    "file": "%s",\n    "command": "$(CXX) $(TEST_BUILD_FLAGS) -c %s -o %s"\n  }' \
			"$$src" "$$src" "$$obj" >> compile_commands.json; \
	done; \
	printf '\n]\n' >> compile_commands.json

# =========================================================
# Clean
# =========================================================

clean:
	rm -rf $(BUILD_DIR)

# =========================================================
# Dependency inclusion
# =========================================================

-include $(DEPS)
