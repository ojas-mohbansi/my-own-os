/* test_io.c - Unit tests for I/O functions */

#include "unity.h"
#include "test_config.h"
#include "../src/error_codes.h"

/* Mock I/O functions for testing */
typedef struct {
    char output_buffer[TEST_BUFFER_SIZE];
    int output_index;
    char input_buffer[TEST_BUFFER_SIZE];
    int input_index;
    int input_available;
    int timeout_occurred;
} MockIO;

static MockIO mock_io;

/* Mock I/O implementations */
void mock_print_char(char c) {
    if (mock_io.output_index < TEST_BUFFER_SIZE - 1) {
        mock_io.output_buffer[mock_io.output_index++] = c;
        mock_io.output_buffer[mock_io.output_index] = '\0';
    }
}

void mock_print(const char* str) {
    if (str) {
        while (*str && mock_io.output_index < TEST_BUFFER_SIZE - 1) {
            mock_io.output_buffer[mock_io.output_index++] = *str++;
        }
        mock_io.output_buffer[mock_io.output_index] = '\0';
    }
}

char mock_read_char(void) {
    if (mock_io.input_available && mock_io.input_index < TEST_BUFFER_SIZE) {
        return mock_io.input_buffer[mock_io.input_index++];
    }
    return '\0';
}

char mock_read_char_timeout(uint32_t timeout_ms, int32_t* error_code) {
    if (mock_io.timeout_occurred) {
        if (error_code) *error_code = ERR_IO_TIMEOUT;
        return '\0';
    }
    
    if (mock_io.input_available && mock_io.input_index < TEST_BUFFER_SIZE) {
        if (error_code) *error_code = ERR_SUCCESS;
        return mock_io.input_buffer[mock_io.input_index++];
    }
    
    if (error_code) *error_code = ERR_IO_DEVICE_ERROR;
    return '\0';
}

int32_t mock_print_char_safe(char c) {
    if (c < 0 || c > 127) {
        return ERR_INVALID_PARAMETER;
    }
    mock_print_char(c);
    return ERR_SUCCESS;
}

int32_t mock_print_string_safe(const char* str) {
    if (!str) {
        return ERR_NULL_POINTER;
    }
    mock_print(str);
    return ERR_SUCCESS;
}

void mock_clear_screen(void) {
    mock_io.output_index = 0;
    mock_io.output_buffer[0] = '\0';
}

/* Test setup and teardown */
void setUp(void) {
    mock_io.output_index = 0;
    mock_io.input_index = 0;
    mock_io.input_available = 0;
    mock_io.timeout_occurred = 0;
    mock_io.output_buffer[0] = '\0';
    mock_io.input_buffer[0] = '\0';
}

void tearDown(void) {
    /* Cleanup after each test */
}

/* Test cases for I/O functions */
void test_print_char_basic(void) {
    mock_print_char('A');
    TEST_ASSERT_EQUAL_STRING("A", mock_io.output_buffer);
    TEST_ASSERT_EQUAL(1, mock_io.output_index);
}

void test_print_char_special_chars(void) {
    mock_print_char('\n');
    TEST_ASSERT_EQUAL_STRING("\n", mock_io.output_buffer);
    
    mock_print_char('\t');
    TEST_ASSERT_EQUAL_STRING("\n\t", mock_io.output_buffer);
}

void test_print_string_basic(void) {
    mock_print("Hello, World!");
    TEST_ASSERT_EQUAL_STRING("Hello, World!", mock_io.output_buffer);
    TEST_ASSERT_EQUAL(13, mock_io.output_index);
}

void test_print_string_empty(void) {
    mock_print("");
    TEST_ASSERT_EQUAL_STRING("", mock_io.output_buffer);
    TEST_ASSERT_EQUAL(0, mock_io.output_index);
}

void test_print_string_null(void) {
    mock_print(NULL);
    TEST_ASSERT_EQUAL_STRING("", mock_io.output_buffer);
    TEST_ASSERT_EQUAL(0, mock_io.output_index);
}

void test_print_multiple_strings(void) {
    mock_print("Hello");
    mock_print(" ");
    mock_print("World");
    TEST_ASSERT_EQUAL_STRING("Hello World", mock_io.output_buffer);
}

void test_read_char_available(void) {
    mock_io.input_available = 1;
    mock_io.input_buffer[0] = 'X';
    mock_io.input_buffer[1] = 'Y';
    mock_io.input_buffer[2] = '\0';
    
    char c1 = mock_read_char();
    TEST_ASSERT_EQUAL('X', c1);
    
    char c2 = mock_read_char();
    TEST_ASSERT_EQUAL('Y', c2);
}

void test_read_char_not_available(void) {
    mock_io.input_available = 0;
    char c = mock_read_char();
    TEST_ASSERT_EQUAL('\0', c);
}

void test_read_char_timeout_success(void) {
    mock_io.input_available = 1;
    mock_io.input_buffer[0] = 'T';
    int32_t error_code;
    
    char c = mock_read_char_timeout(1000, &error_code);
    
    TEST_ASSERT_EQUAL('T', c);
    TEST_ASSERT_EQUAL(ERR_SUCCESS, error_code);
}

void test_read_char_timeout_no_data(void) {
    mock_io.input_available = 0;
    int32_t error_code;
    
    char c = mock_read_char_timeout(1000, &error_code);
    
    TEST_ASSERT_EQUAL('\0', c);
    TEST_ASSERT_EQUAL(ERR_IO_DEVICE_ERROR, error_code);
}

void test_read_char_timeout_timeout(void) {
    mock_io.timeout_occurred = 1;
    int32_t error_code;
    
    char c = mock_read_char_timeout(1000, &error_code);
    
    TEST_ASSERT_EQUAL('\0', c);
    TEST_ASSERT_EQUAL(ERR_IO_TIMEOUT, error_code);
}

void test_print_char_safe_valid(void) {
    int32_t result = mock_print_char_safe('Z');
    TEST_ASSERT_EQUAL(ERR_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING("Z", mock_io.output_buffer);
}

void test_print_char_safe_invalid_low(void) {
    int32_t result = mock_print_char_safe(-1);
    TEST_ASSERT_EQUAL(ERR_INVALID_PARAMETER, result);
    TEST_ASSERT_EQUAL_STRING("", mock_io.output_buffer);
}

void test_print_char_safe_invalid_high(void) {
    int32_t result = mock_print_char_safe(128);
    TEST_ASSERT_EQUAL(ERR_INVALID_PARAMETER, result);
    TEST_ASSERT_EQUAL_STRING("", mock_io.output_buffer);
}

void test_print_string_safe_valid(void) {
    int32_t result = mock_print_string_safe("Safe String");
    TEST_ASSERT_EQUAL(ERR_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING("Safe String", mock_io.output_buffer);
}

void test_print_string_safe_null(void) {
    int32_t result = mock_print_string_safe(NULL);
    TEST_ASSERT_EQUAL(ERR_NULL_POINTER, result);
    TEST_ASSERT_EQUAL_STRING("", mock_io.output_buffer);
}

void test_clear_screen(void) {
    mock_print("Some text");
    TEST_ASSERT_EQUAL_STRING("Some text", mock_io.output_buffer);
    
    mock_clear_screen();
    TEST_ASSERT_EQUAL_STRING("", mock_io.output_buffer);
    TEST_ASSERT_EQUAL(0, mock_io.output_index);
}

void test_buffer_overflow_protection(void) {
    /* Fill buffer almost to capacity */
    for (int i = 0; i < TEST_BUFFER_SIZE - 2; i++) {
        mock_print_char('A');
    }
    
    /* Try to add one more character */
    mock_print_char('B');
    
    /* Should not crash and should have null terminator */
    TEST_ASSERT_EQUAL(TEST_BUFFER_SIZE - 1, mock_io.output_index);
    TEST_ASSERT_EQUAL('\0', mock_io.output_buffer[TEST_BUFFER_SIZE - 1]);
}

/* Test group runner */
int run_io_tests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_print_char_basic);
    RUN_TEST(test_print_char_special_chars);
    RUN_TEST(test_print_string_basic);
    RUN_TEST(test_print_string_empty);
    RUN_TEST(test_print_string_null);
    RUN_TEST(test_print_multiple_strings);
    RUN_TEST(test_read_char_available);
    RUN_TEST(test_read_char_not_available);
    RUN_TEST(test_read_char_timeout_success);
    RUN_TEST(test_read_char_timeout_no_data);
    RUN_TEST(test_read_char_timeout_timeout);
    RUN_TEST(test_print_char_safe_valid);
    RUN_TEST(test_print_char_safe_invalid_low);
    RUN_TEST(test_print_char_safe_invalid_high);
    RUN_TEST(test_print_string_safe_valid);
    RUN_TEST(test_print_string_safe_null);
    RUN_TEST(test_clear_screen);
    RUN_TEST(test_buffer_overflow_protection);
    
    return UNITY_END();
}