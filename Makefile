# Makefile for Operating System Testing Framework

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I. -Isrc -Itests
LDFLAGS = 

# Source directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files
KERNEL_SRC = $(SRC_DIR)/kernel.c
MEMORY_SRC = $(SRC_DIR)/memory_management.c
IO_SRC = $(SRC_DIR)/io.c
FILESYSTEM_SRC = $(SRC_DIR)/file_system.c
SHELL_SRC = $(SRC_DIR)/shell.c
STRING_SRC = $(SRC_DIR)/string.c

# Test source files
UNITY_SRC = $(TEST_DIR)/unity.c
TEST_KERNEL_SRC = $(TEST_DIR)/test_kernel.c
TEST_MEMORY_SRC = $(TEST_DIR)/test_memory_management.c
TEST_IO_SRC = $(TEST_DIR)/test_io.c
TEST_FILESYSTEM_SRC = $(TEST_DIR)/test_file_system.c
TEST_SCALABILITY_SRC = $(TEST_DIR)/test_scalability.c
TEST_RUNNER_SRC = $(TEST_DIR)/test_runner.c

# Object files
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
MEMORY_OBJ = $(BUILD_DIR)/memory_management.o
IO_OBJ = $(BUILD_DIR)/io.o
FILESYSTEM_OBJ = $(BUILD_DIR)/file_system.o
SHELL_OBJ = $(BUILD_DIR)/shell.o
STRING_OBJ = $(BUILD_DIR)/string.o

# Test object files
UNITY_OBJ = $(BUILD_DIR)/unity.o
TEST_KERNEL_OBJ = $(BUILD_DIR)/test_kernel.o
TEST_MEMORY_OBJ = $(BUILD_DIR)/test_memory_management.o
TEST_IO_OBJ = $(BUILD_DIR)/test_io.o
TEST_FILESYSTEM_OBJ = $(BUILD_DIR)/test_file_system.o
TEST_SCALABILITY_OBJ = $(BUILD_DIR)/test_scalability.o
TEST_RUNNER_OBJ = $(BUILD_DIR)/test_runner.o

# Test executables
TEST_KERNEL_EXEC = $(BUILD_DIR)/test_kernel
TEST_MEMORY_EXEC = $(BUILD_DIR)/test_memory
TEST_IO_EXEC = $(BUILD_DIR)/test_io
TEST_FILESYSTEM_EXEC = $(BUILD_DIR)/test_file_system
TEST_ALL_EXEC = $(BUILD_DIR)/test_all

# Main OS executable
OS_EXEC = os

# Default target
.PHONY: all clean test test-all test-kernel test-memory test-io test-filesystem

all: $(OS_EXEC)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build OS executable
$(OS_EXEC): $(KERNEL_OBJ) $(MEMORY_OBJ) $(IO_OBJ) $(FILESYSTEM_OBJ) $(SHELL_OBJ) $(STRING_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Build object files
$(KERNEL_OBJ): $(KERNEL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(MEMORY_OBJ): $(MEMORY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(IO_OBJ): $(IO_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(FILESYSTEM_OBJ): $(FILESYSTEM_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHELL_OBJ): $(SHELL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(STRING_OBJ): $(STRING_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build Unity framework
$(UNITY_OBJ): $(UNITY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test object files
$(TEST_KERNEL_OBJ): $(TEST_KERNEL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTEST_MOCK -c $< -o $@

$(TEST_MEMORY_OBJ): $(TEST_MEMORY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTEST_MOCK -c $< -o $@

$(TEST_IO_OBJ): $(TEST_IO_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTEST_MOCK -c $< -o $@

$(TEST_FILESYSTEM_OBJ): $(TEST_FILESYSTEM_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTEST_MOCK -c $< -o $@

$(TEST_SCALABILITY_OBJ): $(TEST_SCALABILITY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTEST_MOCK -c $< -o $@

$(TEST_RUNNER_OBJ): $(TEST_RUNNER_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build individual test executables
$(TEST_KERNEL_EXEC): $(TEST_KERNEL_OBJ) $(UNITY_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(TEST_MEMORY_EXEC): $(TEST_MEMORY_OBJ) $(UNITY_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(TEST_IO_EXEC): $(TEST_IO_OBJ) $(UNITY_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(TEST_FILESYSTEM_EXEC): $(TEST_FILESYSTEM_OBJ) $(UNITY_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Build comprehensive test suite
$(TEST_ALL_EXEC): $(TEST_RUNNER_OBJ) $(TEST_KERNEL_OBJ) $(TEST_MEMORY_OBJ) $(TEST_IO_OBJ) $(TEST_FILESYSTEM_OBJ) $(TEST_SCALABILITY_OBJ) $(UNITY_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Test targets
test: test-all

test-all: $(TEST_ALL_EXEC)
	@echo "Running all tests..."
	@./$(TEST_ALL_EXEC)

test-kernel: $(TEST_KERNEL_EXEC)
	@echo "Running kernel tests..."
	@./$(TEST_KERNEL_EXEC)

test-memory: $(TEST_MEMORY_EXEC)
	@echo "Running memory management tests..."
	@./$(TEST_MEMORY_EXEC)

test-io: $(TEST_IO_EXEC)
	@echo "Running I/O tests..."
	@./$(TEST_IO_EXEC)

test-filesystem: $(TEST_FILESYSTEM_EXEC)
	@echo "Running file system tests..."
	@./$(TEST_FILESYSTEM_EXEC)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(OS_EXEC) $(TEST_ALL_EXEC) $(TEST_KERNEL_EXEC) $(TEST_MEMORY_EXEC) $(TEST_IO_EXEC) $(TEST_FILESYSTEM_EXEC)

# Help target
help:
	@echo "Operating System Testing Framework"
	@echo "=================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build the OS executable"
	@echo "  test         - Run all tests (same as test-all)"
	@echo "  test-all     - Build and run comprehensive test suite"
	@echo "  test-kernel  - Build and run kernel tests only"
	@echo "  test-memory  - Build and run memory management tests only"
	@echo "  test-io      - Build and run I/O tests only"
	@echo "  test-filesystem - Build and run file system tests only"
	@echo "  clean        - Remove build artifacts"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Example usage:"
	@echo "  make test-all    # Run all tests"
	@echo "  make test-kernel # Run kernel tests only"
	@echo "  make clean       # Clean build files"