.PHONY: all tests clean

CXX = clang++
CXXFLAGS = -std=c++20 -IInclude -IThirdParty

TEST_CXX = clang++
TEST_CXXFLAGS = -std=c++20 -IInclude -IThirdParty -Wno-macro-redefined -fno-exceptions -fno-rtti -fsanitize=address -fsanitize=undefined -fsanitize=leak

BUILD_DIR  = build
OBJ_DIR    = build/obj
TEST_DIR = build/tests

TEST_SRCS  = $(shell find Tests -name "Test*.cpp")
TEST_OBJS  = $(patsubst Tests/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
TEST_BINS  = $(patsubst Tests/%.cpp,$(TEST_DIR)/%,$(TEST_SRCS))

SRC_SRCS   = $(shell find Src -name "*.cpp")
SRC_OBJS   = $(patsubst Src/%.cpp,$(OBJ_DIR)/Src/%.o,$(SRC_SRCS))

$(OBJ_DIR)/%.o: Tests/%.cpp
	@mkdir -p $(@D)
	$(TEST_CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/Src/%.o: Src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_DIR)/%: $(OBJ_DIR)/%.o $(SRC_OBJS)
	@mkdir -p $(@D)
	$(TEST_CXX) $(TEST_CXXFLAGS) $^ -o $@

tests: $(TEST_BINS)
	@for bin in $(TEST_BINS); do \
		./$$bin; \
	done

.SECONDARY: $(TEST_OBJS) $(SRC_OBJS)

clean:
	rm -rf $(BUILD_DIR)

