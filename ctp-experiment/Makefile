# Builds the experiment CLI and all focused C++ test executables.
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude
SOURCES := $(wildcard src/*.cpp)
LIB_SOURCES := $(filter-out src/main.cpp,$(SOURCES))
TARGET := build/ctp
TEST_SOURCES := tests/test_graph.cpp tests/test_baseline.cpp tests/test_experiment.cpp tests/test_visualizer.cpp
TEST_TARGETS := $(patsubst tests/%.cpp,build/%,$(TEST_SOURCES))

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

build/%: tests/%.cpp $(LIB_SOURCES)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(TEST_TARGETS)
	@for test in $(TEST_TARGETS); do ./$$test || exit $$?; done

clean:
	rm -f $(TARGET) $(TEST_TARGETS) build/test-instance.ctp build/test-config.csv
	rm -f build/test-results.csv build/test-graph.ctp build/test-graph.dot
	rm -f build/test-graphs/run_0001.ctp build/test-dots/run_0001.dot
	@if test -d build/test-graphs; then rmdir build/test-graphs; fi
	@if test -d build/test-dots; then rmdir build/test-dots; fi
