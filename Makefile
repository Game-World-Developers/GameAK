.PHONY: all tests clean

CXX = clang++
CXXFLAGS = -std=c++20 -IInclude -IThirdParty

TEST_CXX = clang++
TEST_CXXFLAGS = -std=c++20 -IInclude -IThirdParty -Wno-macro-redefined

BUILD_DIR  = build
OBJ_DIR    = build/obj
TEST_DIR = build/tests

TEST_SRCS  = $(shell find Tests -name "Test*.cpp")
TEST_OBJS  = $(patsubst Tests/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
TEST_BINS  = $(patsubst Tests/%.cpp,$(TEST_DIR)/%,$(TEST_SRCS))

$(OBJ_DIR)/%.o: Tests/%.cpp
	@mkdir -p $(@D)
	$(TEST_CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(TEST_DIR)/%: $(OBJ_DIR)/%.o
	@mkdir -p $(@D)
	$(TEST_CXX) $< -o $@

tests: $(TEST_BINS)
	@for bin in $(TEST_BINS); do \
		./$$bin; \
	done

.SECONDARY: $(TEST_OBJS)

clean:
	rm -rf $(BUILD_DIR)

