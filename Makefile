# Tool definitions
CC ?= gcc
CXX ?= g++

# Settings
SRC_DIR = ./src
LINT_DIR = ./lint
TEST_DIR = ./tests
BUILD_DIR = .
NAME = app.elf

# Search path for header files
CFLAGS += -I$(SRC_DIR)/average

# List module source files
CSOURCES = $(SRC_DIR)/main.c
CSOURCES += $(wildcard $(SRC_DIR)/average/*.c)

# Compiler flags
CFLAGS += -Wall

# Linker flags
LDFLAGS += 

# Generate names for output object files (*.o)
COBJECTS = $(patsubst %.c, %.o, $(CSOURCES))


# Run tests
.PHONY: test
test:
	make -C $(TEST_DIR)
	
# Clean tests
.PHONY: test_clean
test_clean:
	make -C $(TEST_DIR) clean
	
# Run tests
.PHONY: lint
lint:
	make -C $(LINT_DIR)