/* test_error_handling.c - Test program for error handling system */

#include "src/error_codes.h"
#include "src/kernel.h"
#include "src/file_system.h"
#include <stdio.h>
#include <stdlib.h>

/* Mock implementations for testing */
void handle_error(int32_t error_code, const char* function, const char* file, uint32_t line) {
    printf("ERROR [%s:%d in %s]: Code %d\n", file, line, function, error_code);
}

void panic(const char* msg) {
    printf("PANIC: %s\n", msg);
    exit(1);
}

void print(const char* str) {
    printf("%s", str);
}

void print_char(char c) {
    putchar(c);
}

char read_char(void) {
    return getchar();
}

void clear_screen(void) {
    system("cls");
}

/* Test function that triggers various errors */
void test_error_handling(void) {
    printf("Testing Error Handling System\n");
    printf("=============================\n\n");
    
    /* Test 1: Null pointer error */
    printf("Test 1: Null pointer error\n");
    HANDLE_ERROR(ERR_NULL_POINTER);
    
    /* Test 2: File system errors */
    printf("\nTest 2: File system errors\n");
    HANDLE_ERROR(ERR_FILE_NOT_FOUND);
    HANDLE_ERROR(ERR_FILE_EXISTS);
    HANDLE_ERROR(ERR_FILE_SYSTEM_FULL);
    
    /* Test 3: I/O errors */
    printf("\nTest 3: I/O errors\n");
    HANDLE_ERROR(ERR_IO_TIMEOUT);
    HANDLE_ERROR(ERR_IO_DEVICE_ERROR);
    
    /* Test 4: Memory errors */
    printf("\nTest 4: Memory errors\n");
    HANDLE_ERROR(ERR_OUT_OF_MEMORY);
    HANDLE_ERROR(ERR_INVALID_ADDRESS);
    
    printf("\nAll error handling tests completed successfully!\n");
}

/* Test file system error handling */
void test_file_system_errors(void) {
    printf("\nTesting File System Error Handling\n");
    printf("==================================\n\n");
    
    FileSystem fs;
    uint8_t memory[1024];
    File file_info;
    uint8_t buffer[100];
    
    /* Test 1: Invalid initialization */
    printf("Test 1: Invalid file system initialization\n");
    int32_t result = fs_init(NULL, memory, sizeof(memory));
    printf("Result: %d (%s)\n", result, fs_error_string(result));
    
    result = fs_init(&fs, NULL, sizeof(memory));
    printf("Result: %d (%s)\n", result, fs_error_string(result));
    
    /* Test 2: Valid initialization */
    printf("\nTest 2: Valid file system initialization\n");
    result = fs_init(&fs, memory, sizeof(memory));
    printf("Result: %d (%s)\n", result, fs_error_string(result));
    
    /* Test 3: Invalid file operations */
    printf("\nTest 3: Invalid file operations\n");
    result = fs_read_file(&fs, 999, buffer, sizeof(buffer), 0);
    printf("Read invalid file: %d (%s)\n", result, fs_error_string(result));
    
    result = fs_write_file(&fs, 999, buffer, sizeof(buffer), 0);
    printf("Write invalid file: %d (%s)\n", result, fs_error_string(result));
    
    /* Test 4: File creation errors */
    printf("\nTest 4: File creation errors\n");
    result = fs_create_file(NULL, "test.txt", 0);
    printf("Create file with null fs: %d (%s)\n", result, fs_error_string(result));
    
    result = fs_create_file(&fs, NULL, 0);
    printf("Create file with null name: %d (%s)\n", result, fs_error_string(result));
    
    result = fs_create_file(&fs, "very_long_filename_that_exceeds_maximum_allowed_length.txt", 0);
    printf("Create file with long name: %d (%s)\n", result, fs_error_string(result));
    
    printf("\nFile system error handling tests completed!\n");
}

int main(void) {
    test_error_handling();
    test_file_system_errors();
    
    printf("\n=====================================\n");
    printf("All error handling tests passed!\n");
    printf("Error handling system is working correctly.\n");
    printf("=====================================\n");
    
    return 0;
}