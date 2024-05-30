# Dynamically generate absolute paths
# From https://matheusmbar.com/bugfree-robot/2019/04/05/makefile-first-test.html
MAKEFILE_DIR = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
PROJECT_DIR = $(realpath $(MAKEFILE_DIR)../)
SRC_DIR = $(PROJECT_DIR)/src
# Define test component directories
#TEST_DIRS = $(SRC_DIR $(wildcard */))
#$(info Hello World Make just parsed past this line)
#$(info $(SRC_DIR))

# Location of CppUTest (if not already set)
CPPCHECK_HOME ?= $(PROJECT_DIR)/tools/cppcheck
# Tool definitions
CC = cppcheck

# List module source files
#CSOURCES = $(SRC_DIR)/main.c
#CSOURCES += $(wildcard $(SRC_DIR)/*.c)
#CSOURCES += $(wildcard $(SRC_DIR)/average/*.c)

# linter flags
CFLAGS += --language=c++ --error-exitcode=1

#$(info $(CSOURCES))

# Build the target application
.PHONY: lint
lint: 
	$(CPPCHECK_HOME)/$(CC) $(CFLAGS) $(SRC_DIR) 2>&1
