NPROC     := $(shell nproc)
MAKEFLAGS += -j$(NPROC)

CXX       := clang++
CXXFLAGS  := -std=c++20 -Wall -Wextra -Wpedantic -Werror -pipe -Wno-unused-private-field
CPPFLAGS  := -I Include -I Libs -I Libs/cest
LDFLAGS   :=

SPDLOG_CFLAGS := $(shell pkg-config --cflags spdlog)
SPDLOG_LIBS   := $(shell pkg-config --libs spdlog)

BUILD_DIR := Build
OBJ_DIR   := $(BUILD_DIR)/Obj
LIB_DIR   := $(BUILD_DIR)/Lib

SRC_CORE    := Src/GameAk/Core/identity.cpp
SRC_RUNTIME := Src/GameAk/Runtime/command.cpp \
               Src/GameAk/Runtime/controller.cpp \
               Src/GameAk/Runtime/runtime.cpp \
               Src/GameAk/Runtime/scheduler.cpp \
               Src/GameAk/Runtime/priority_scheduler.cpp
SRCS        := $(SRC_CORE) $(SRC_RUNTIME)
OBJS        := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

TEST_SRC    := Tests/test_runtime.cpp
TEST_OBJ    := $(OBJ_DIR)/$(TEST_SRC:.cpp=.o)

LIB         := $(LIB_DIR)/libgameak.a
TEST_BIN    := $(BUILD_DIR)/gameak_test

# ccache wrapper
CXX := ccache $(CXX)

.PHONY: all test clean

all: $(TEST_BIN)

$(OBJS) $(TEST_OBJ): $(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SPDLOG_CFLAGS) -c -o $@ $<

$(LIB): $(OBJS) | $(LIB_DIR)
	ar rcs $@ $^

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

$(TEST_BIN): $(TEST_OBJ) $(LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ) -L$(LIB_DIR) -lgameak $(SPDLOG_LIBS)

test: $(TEST_BIN)
	./$(TEST_BIN) --quiet

clean:
	rm -rf $(BUILD_DIR) $(TEST_BIN)
