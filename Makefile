CXX = clang++
CXXFLAGS = -std=c++20 -IInclude -IThirdParty

TEST_SRCS = $(shell find Tests -name "Test*.cpp")
TEST_BINS = $(TEST_SRCS:.cpp=.bin)

all: tests

test: tests

tests: $(TEST_BINS)
	@for bin in $(TEST_BINS); do \
		./$$bin; \
	done

%.bin: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(TEST_BINS)

.PHONY: all test tests clean
