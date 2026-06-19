NPROC       := $(shell nproc 2>/dev/null || echo 1)
MAKEFLAGS   += -j$(NPROC)

BUILD_TYPE  ?= debug

CXX         := clang++
CXXFLAGS    := -std=c++20 -Wall -Wextra -Wpedantic -Werror -pipe -Wno-unused-private-field -MMD -MP
CPPFLAGS    := -I Include -I Libs -I Libs/cest
LDFLAGS     :=
LDLIBS      :=

ifeq ($(BUILD_TYPE),debug)
  CXXFLAGS  += -Og -g
else
  CXXFLAGS  += -O2 -DNDEBUG
endif

CCACHE := $(shell command -v ccache 2>/dev/null)
ifneq ($(CCACHE),)
  CXX := ccache $(CXX)
endif

BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/objs
LIB_DIR   := $(BUILD_DIR)/lib

SPDLOG_CFLAGS := $(shell pkg-config --cflags spdlog 2>/dev/null)
SPDLOG_LIBS   := $(shell pkg-config --libs spdlog 2>/dev/null)

ifneq ($(SPDLOG_CFLAGS),)
  CPPFLAGS += -DGAME_AK_HAVE_SPDLOG
  LDLIBS   += $(SPDLOG_LIBS)
endif

LDFLAGS += -L$(LIB_DIR)

SRCS      := $(wildcard Src/GameAk/Core/*.cpp) $(wildcard Src/GameAk/Runtime/*.cpp)
OBJS      := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

TEST_SRC  := Tests/test_runtime.cpp
TEST_OBJ  := $(OBJ_DIR)/$(TEST_SRC:.cpp=.o)

DEPS      := $(OBJS:.o=.d) $(TEST_OBJ:.o=.d)
-include $(DEPS)

LIB       := $(LIB_DIR)/libgameak.a
TEST_BIN  := $(BUILD_DIR)/gameak_test

.PHONY: all test clean install

all: clean test $(LIB)

test: $(TEST_BIN)
	./$(TEST_BIN) --quiet

clean:
	$(RM) -r $(BUILD_DIR)

PREFIX ?= /usr/local

install: $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/GameAk/Core
	install -d $(DESTDIR)$(PREFIX)/include/GameAk/Runtime
	install -m 644 Include/GameAk/Core/*.h $(DESTDIR)$(PREFIX)/include/GameAk/Core/
	install -m 644 Include/GameAk/Runtime/*.h $(DESTDIR)$(PREFIX)/include/GameAk/Runtime/

$(OBJS) $(TEST_OBJ): $(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SPDLOG_CFLAGS) -c -o $@ $<

$(LIB): $(OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

$(TEST_BIN): $(TEST_OBJ) $(LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ) -lgameak $(LDLIBS)
