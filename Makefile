# =============================================================================
# GameAK Build System
#
# The primary build system for GameAK.
# Supports: Clang (primary), GCC, MSVC
# =============================================================================

# --- Parallelism -----------------------------------------------------------

NPROC     := $(shell nproc 2>/dev/null || echo 1)
MAKEFLAGS += -j$(NPROC)

# --- Build Type ------------------------------------------------------------

BUILD_TYPE ?= debug

# --- Tools -----------------------------------------------------------------

CXX       := clang++
CXXFLAGS  := -std=c++20 -Wall -Wextra -Wpedantic -Werror -pipe -Wno-unused-private-field -MMD -MP
CPPFLAGS  := -I Include -I Libs -I Libs/cest
LDFLAGS   :=
LDLIBS    :=
AR        := ar

ifeq ($(BUILD_TYPE),debug)
  CXXFLAGS += -Og -g
else
  CXXFLAGS += -O2 -DNDEBUG
endif

# --- ccache ----------------------------------------------------------------

CCACHE := $(shell command -v ccache 2>/dev/null)
ifneq ($(CCACHE),)
  CXX := ccache $(CXX)
endif

# --- spdlog (optional) -----------------------------------------------------

SPDLOG_CFLAGS := $(shell pkg-config --cflags spdlog 2>/dev/null)
SPDLOG_LIBS   := $(shell pkg-config --libs spdlog 2>/dev/null)
ifneq ($(SPDLOG_CFLAGS),)
  CPPFLAGS += -DGAME_AK_HAVE_SPDLOG
  LDLIBS   += $(SPDLOG_LIBS)
endif

# --- Directories -----------------------------------------------------------

BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/objs
LIB_DIR   := $(BUILD_DIR)/lib

# --- Sources (split: AK = standalone, Runtime = depends on AK) -------------

AK_SRCS      := $(wildcard Src/GameAk/Core/*.cpp)
RUNTIME_SRCS := $(wildcard Src/GameAk/Runtime/*.cpp)
TEST_SRC     := Tests/test_runtime.cpp

AK_OBJS      := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(AK_SRCS))
RUNTIME_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(RUNTIME_SRCS))
TEST_OBJ     := $(OBJ_DIR)/$(TEST_SRC:.cpp=.o)

ALL_SRCS     := $(AK_SRCS) $(RUNTIME_SRCS) $(TEST_SRC)
ALL_OBJS     := $(AK_OBJS) $(RUNTIME_OBJS) $(TEST_OBJ)

# --- Libraries -------------------------------------------------------------

AK_LIB       := $(LIB_DIR)/libgameak-core.a
RUNTIME_LIB  := $(LIB_DIR)/libgameak-runtime.a
COMBINED_LIB := $(LIB_DIR)/libgameak.a

# --- Test Binary -----------------------------------------------------------

TEST_BIN := $(BUILD_DIR)/gameak_test

# --- Dependency Tracking ---------------------------------------------------

DEPS := $(ALL_OBJS:.o=.d)
-include $(DEPS)

# ===========================================================================
# Targets
# ===========================================================================

.PHONY: all test clean install help \
        check-spec-status tidy format

all: $(COMBINED_LIB) $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN) --quiet

clean:
	$(RM) -r $(BUILD_DIR)

# ===========================================================================
# Pattern Rule — Compilation
# ===========================================================================

$(ALL_OBJS): $(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SPDLOG_CFLAGS) -c -o $@ $<

# ===========================================================================
# Libraries
# ===========================================================================

# AK (Abstraction Kit) — standalone library, no Runtime dependency.
$(AK_LIB): $(AK_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

# Runtime library — depends on AK at link time.
$(RUNTIME_LIB): $(RUNTIME_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

# Combined convenience library — both AK + Runtime in one archive.
$(COMBINED_LIB): $(AK_LIB) $(RUNTIME_LIB)
	$(AR) rcs $@ $^

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

# ===========================================================================
# Test Binary
# ===========================================================================

LDFLAGS += -L$(LIB_DIR)

$(TEST_BIN): $(TEST_OBJ) $(RUNTIME_LIB) $(AK_LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ) -lgameak-runtime -lgameak-core $(LDLIBS)

# ===========================================================================
# Development Targets
# ===========================================================================

# --- Compilation Database (clangd / LSP) -----------------------------------

compile_commands.json: $(ALL_SRCS)
	@scripts/gen_compile_commands.sh "$@" "$(CURDIR)" "$(CXX)" "$(CXXFLAGS)" "$(CPPFLAGS) $(SPDLOG_CFLAGS)"

# --- clang-tidy ------------------------------------------------------------

tidy: compile_commands.json
	@command -v clang-tidy >/dev/null 2>&1 && \
		clang-tidy $(ALL_SRCS) -p . 2>&1 | grep -v "^Skipping" || \
		echo "clang-tidy not available; install it for static analysis"

# --- clang-format ----------------------------------------------------------

FORMAT_FILES := $(ALL_SRCS) \
                $(wildcard Include/GameAk/Core/*.h) \
                $(wildcard Include/GameAk/Runtime/*.h) \
                $(wildcard Tests/*.h) \
                $(wildcard Tests/*.cpp)

format:
	@command -v clang-format >/dev/null 2>&1 && \
		clang-format -i --style=file $(FORMAT_FILES) 2>/dev/null || \
		echo "clang-format not available; install it for code formatting"

# --- Spec-Test Validation (SPEC-004) ---------------------------------------

check-spec-status:
	@.ci/check-spec-status.sh

# ===========================================================================
# Install
# ===========================================================================

PREFIX ?= /usr/local

install: $(COMBINED_LIB) $(AK_LIB) $(RUNTIME_LIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(COMBINED_LIB) $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(AK_LIB) $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(RUNTIME_LIB) $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/GameAk/Core
	install -d $(DESTDIR)$(PREFIX)/include/GameAk/Runtime
	install -m 644 Include/GameAk/Core/*.h $(DESTDIR)$(PREFIX)/include/GameAk/Core/
	install -m 644 Include/GameAk/Runtime/*.h $(DESTDIR)$(PREFIX)/include/GameAk/Runtime/

# ===========================================================================
# Help
# ===========================================================================

help:
	@echo "GameAK Build System"
	@echo ""
	@echo "Usage: make <target> [BUILD_TYPE=debug|release]"
	@echo ""
	@echo "Targets:"
	@echo "  all                  Build libraries + test binary (default)"
	@echo "  test                 Build and run tests"
	@echo "  clean                Remove build artifacts"
	@echo "  install [PREFIX=...] Install libraries and headers"
	@echo ""
	@echo "  compile_commands.json  Generate compilation database for LSP"
	@echo "  tidy                  Run clang-tidy static analysis"
	@echo "  format                Run clang-format on all sources"
	@echo "  check-spec-status     Validate spec-test correspondence"
	@echo ""
	@echo "  help                  Show this message"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE=debug     Debug build (-Og -g, default)"
	@echo "  BUILD_TYPE=release   Release build (-O2 -DNDEBUG)"
	@echo "  CXX=<compiler>       C++ compiler (default: clang++)"
	@echo "  PREFIX=<path>        Install prefix (default: /usr/local)"
