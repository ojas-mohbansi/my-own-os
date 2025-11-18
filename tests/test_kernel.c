/* test_kernel.c - Unit tests for kernel functions */

#include "unity.h"
#include "test_config.h"
#include "../src/error_codes.h"
#include "../src/kernel.h"

/* Mock implementations for testing */
static int mock_panic_called = 0;
static char mock_panic_msg[256];
static int mock_error_logged = 0;
static int32_t last_error_code;

/* Mock panic function */
void panic(const char* msg) {
    mock_panic_called = 1;
    strncpy(mock_panic_msg, msg, sizeof(mock_panic_msg) - 1);
    mock_panic_msg[sizeof(mock_panic_msg) - 1] = '\0';
}

/* Mock error handler */
void handle_error(int32_t error_code, const char* function, const char* file, uint32_t line) {
    mock_error_logged = 1;
    last_error_code = error_code;
    (void)function; /* Suppress unused parameter */
    (void)file;     /* Suppress unused parameter */
    (void)line;     /* Suppress unused parameter */
}

/* Test setup and teardown */
void setUp(void) {
    mock_panic_called = 0;
    mock_panic_msg[0] = '\0';
    mock_error_logged = 0;
    last_error_code = 0;
}

void tearDown(void) {
    /* Cleanup after each test */
}

/* Test cases for error handling */
void test_handle_error_info_level(void) {
    /* Test that info-level errors don't cause panic */
    HANDLE_ERROR(ERR_SUCCESS);
    TEST_ASSERT_EQUAL(1, mock_error_logged);
    TEST_ASSERT_EQUAL(ERR_SUCCESS, last_error_code);
    TEST_ASSERT_EQUAL(0, mock_panic_called);
}

void test_handle_error_warning_level(void) {
    /* Test that warning-level errors don't cause panic */
    HANDLE_ERROR(ERR_IO_TIMEOUT);
    TEST_ASSERT_EQUAL(1, mock_error_logged);
    TEST_ASSERT_EQUAL(ERR_IO_TIMEOUT, last_error_code);
    TEST_ASSERT_EQUAL(0, mock_panic_called);
}

void test_handle_error_error_level(void) {
    /* Test that error-level errors don't cause panic */
    HANDLE_ERROR(ERR_FILE_NOT_FOUND);
    TEST_ASSERT_EQUAL(1, mock_error_logged);
    TEST_ASSERT_EQUAL(ERR_FILE_NOT_FOUND, last_error_code);
    TEST_ASSERT_EQUAL(0, mock_panic_called);
}

void test_handle_error_critical_level(void) {
    /* Test that critical-level errors don't cause panic */
    HANDLE_ERROR(ERR_OUT_OF_MEMORY);
    TEST_ASSERT_EQUAL(1, mock_error_logged);
    TEST_ASSERT_EQUAL(ERR_OUT_OF_MEMORY, last_error_code);
    TEST_ASSERT_EQUAL(0, mock_panic_called);
}

void test_handle_error_fatal_level(void) {
    /* Test that fatal-level errors cause panic */
    HANDLE_ERROR(ERR_KERNEL_PANIC);
    TEST_ASSERT_EQUAL(1, mock_error_logged);
    TEST_ASSERT_EQUAL(ERR_KERNEL_PANIC, last_error_code);
    TEST_ASSERT_EQUAL(1, mock_panic_called);
}

void test_panic_function(void) {
    /* Test panic function behavior */
    const char* test_msg = "Test panic message";
    panic(test_msg);
    TEST_ASSERT_EQUAL(1, mock_panic_called);
    TEST_ASSERT_EQUAL_STRING(test_msg, mock_panic_msg);
}

void test_error_code_ranges(void) {
    /* Test that error codes are properly categorized */
    TEST_ASSERT_TRUE(ERR_SUCCESS >= 0);
    TEST_ASSERT_TRUE(ERR_NULL_POINTER < 0);
    TEST_ASSERT_TRUE(ERR_IO_TIMEOUT < 0);
    TEST_ASSERT_TRUE(ERR_FILE_NOT_FOUND < 0);
    TEST_ASSERT_TRUE(ERR_OUT_OF_MEMORY < 0);
    TEST_ASSERT_TRUE(ERR_KERNEL_PANIC < 0);
}

void test_error_severity_mapping(void) {
    /* Test that error codes map to correct severity levels */
    /* Info level (0) - success */
    TEST_ASSERT_EQUAL(0, get_error_severity(ERR_SUCCESS));
    
    /* Warning level (1) - I/O timeouts, etc. */
    TEST_ASSERT_EQUAL(1, get_error_severity(ERR_IO_TIMEOUT));
    TEST_ASSERT_EQUAL(1, get_error_severity(ERR_IO_DEVICE_ERROR));
    
    /* Error level (2) - file not found, etc. */
    TEST_ASSERT_EQUAL(2, get_error_severity(ERR_FILE_NOT_FOUND));
    TEST_ASSERT_EQUAL(2, get_error_severity(ERR_INVALID_PARAMETER));
    
    /* Critical level (3) - out of memory, etc. */
    TEST_ASSERT_EQUAL(3, get_error_severity(ERR_OUT_OF_MEMORY));
    TEST_ASSERT_EQUAL(3, get_error_severity(ERR_FILE_SYSTEM_FULL));
    
    /* Fatal level (4) - kernel panic, etc. */
    TEST_ASSERT_EQUAL(4, get_error_severity(ERR_KERNEL_PANIC));
}

/* Helper function to get error severity (mock implementation) */
static int get_error_severity(int32_t error_code) {
    if (error_code == ERR_SUCCESS) return 0;  /* INFO */
    if (error_code >= -20 && error_code <= -13) return 1;  /* WARNING: I/O errors */
    if (error_code >= -40 && error_code <= -20) return 2;  /* ERROR: File system */
    if (error_code >= -37 && error_code <= -30) return 2;  /* ERROR: File system extended */
    if (error_code >= -53 && error_code <= -50) return 3;  /* CRITICAL: Memory errors */
    if (error_code >= -70 && error_code <= -70) return 4;  /* FATAL: Kernel panic */
    return 2;  /* Default to ERROR level */
}

/* Test group runner */
int run_kernel_tests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_handle_error_info_level);
    RUN_TEST(test_handle_error_warning_level);
    RUN_TEST(test_handle_error_error_level);
    RUN_TEST(test_handle_error_critical_level);
    RUN_TEST(test_handle_error_fatal_level);
    RUN_TEST(test_panic_function);
    RUN_TEST(test_error_code_ranges);
    RUN_TEST(test_error_severity_mapping);
    
    return UNITY_END();
}