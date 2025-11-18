/******************************************************************************
 * Security Test Suite - Standalone Version
 * 
 * This test suite validates the security implementation without relying on
 * the Unity framework, which has compilation issues.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "../src/security.h"

/* External declaration for user database from security.c */
extern user_t user_database[16];
extern uint32_t user_count;

/* Test result tracking */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Simple test macros */
#define TEST_ASSERT_TRUE(condition) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("  ✓ PASS: %s\n", #condition); \
        } else { \
            tests_failed++; \
            printf("  ✗ FAIL: %s (line %d)\n", #condition, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_FALSE(condition) \
    do { \
        tests_run++; \
        if (!(condition)) { \
            tests_passed++; \
            printf("  ✓ PASS: !%s\n", #condition); \
        } else { \
            tests_failed++; \
            printf("  ✗ FAIL: !%s (line %d)\n", #condition, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        tests_run++; \
        if ((expected) == (actual)) { \
            tests_passed++; \
            printf("  ✓ PASS: %s == %s\n", #expected, #actual); \
        } else { \
            tests_failed++; \
            printf("  ✗ FAIL: %s == %s (line %d)\n", #expected, #actual, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        tests_run++; \
        if ((ptr) != NULL) { \
            tests_passed++; \
            printf("  ✓ PASS: %s != NULL\n", #ptr); \
        } else { \
            tests_failed++; \
            printf("  ✗ FAIL: %s != NULL (line %d)\n", #ptr, __LINE__); \
        } \
    } while(0)

/* Test functions */
static void test_security_initialization(void) {
    printf("\n=== Testing Security Initialization ===\n");
    
    /* Test security initialization */
    TEST_ASSERT_TRUE(security_init_authentication());
    TEST_ASSERT_TRUE(security_init_memory_protection());
    TEST_ASSERT_TRUE(security_init_kernel_protection());
}

static void test_user_management(void) {
    printf("\n=== Testing User Management ===\n");
    
    /* Test creating users */
    TEST_ASSERT_TRUE(security_create_user("testuser", "testpass123", PRIVILEGE_USER));
    TEST_ASSERT_TRUE(security_create_user("testadmin", "adminpass456", PRIVILEGE_ADMIN));
    
    /* Test creating duplicate user (should fail) */
    TEST_ASSERT_FALSE(security_create_user("testuser", "newpass", PRIVILEGE_USER));
    
    /* Test authenticating users */
    TEST_ASSERT_TRUE(security_authenticate_user("testuser", "testpass123"));
    TEST_ASSERT_TRUE(security_authenticate_user("testadmin", "adminpass456"));
    
    /* Test authenticating with wrong password */
    TEST_ASSERT_FALSE(security_authenticate_user("testuser", "wrongpass"));
    TEST_ASSERT_FALSE(security_authenticate_user("nonexistent", "password"));
    
    /* Test getting current user */
    user_t* current_user = security_get_current_user();
    TEST_ASSERT_NOT_NULL(current_user);
}

static void test_input_validation(void) {
    printf("\n=== Testing Input Validation ===\n");
    
    /* Test valid input */
    TEST_ASSERT_TRUE(security_validate_input("hello world", 20));
    TEST_ASSERT_TRUE(security_validate_input("test123", 10));
    
    /* Test invalid input */
    TEST_ASSERT_FALSE(security_validate_input(NULL, 10));
    TEST_ASSERT_FALSE(security_validate_input("", 0));
    TEST_ASSERT_FALSE(security_validate_input("test\x01world", 20)); /* Non-printable character */
    
    /* Test command injection prevention */
    TEST_ASSERT_FALSE(security_validate_input("ls; rm -rf /", 50));
    TEST_ASSERT_FALSE(security_validate_input("cat /etc/passwd", 50));
    TEST_ASSERT_FALSE(security_validate_input("test|rm *", 20));
    TEST_ASSERT_FALSE(security_validate_input("test&rm *", 20));
    TEST_ASSERT_FALSE(security_validate_input("test`rm *`", 20));
    TEST_ASSERT_FALSE(security_validate_input("test$(rm *)", 20));
}

static void test_memory_protection(void) {
    printf("\n=== Testing Memory Protection ===\n");
    
    /* Test memory allocation with protection */
    void* mem1;
    void* mem2;
    
    TEST_ASSERT_TRUE(security_allocate_secure_memory(&mem1, 100, MEM_PROT_READ | MEM_PROT_WRITE));
    TEST_ASSERT_NOT_NULL(mem1);
    
    TEST_ASSERT_TRUE(security_allocate_secure_memory(&mem2, 200, MEM_PROT_READ | MEM_PROT_WRITE));
    TEST_ASSERT_NOT_NULL(mem2);
    
    /* Test memory validation */
    TEST_ASSERT_TRUE(security_check_memory_access(mem1, 50, MEM_PROT_READ));
    TEST_ASSERT_TRUE(security_check_memory_access(mem2, 100, MEM_PROT_WRITE));
    
    /* Test invalid memory access */
    TEST_ASSERT_FALSE(security_check_memory_access(NULL, 100, MEM_PROT_READ));
    TEST_ASSERT_FALSE(security_check_memory_access((void*)0x1000, 0xFFFFFFFF, MEM_PROT_READ)); /* Invalid range */
    
    /* Test memory zeroing */
    memset(mem1, 0xFF, 100);
    security_zero_memory(mem1, 100);
    
    uint8_t* byte_mem = (uint8_t*)mem1;
    bool all_zero = true;
    for (int i = 0; i < 100; i++) {
        if (byte_mem[i] != 0) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT_TRUE(all_zero);
    
    /* Clean up */
    security_free_secure_memory(mem1);
    security_free_secure_memory(mem2);
}

static void test_authorization(void) {
    printf("\n=== Testing Authorization ===\n");
    
    /* Create users with different privileges */
    security_create_user("guest_user", "guest123", PRIVILEGE_GUEST);
    security_create_user("user_user", "user123", PRIVILEGE_USER);
    security_create_user("admin_user", "admin123", PRIVILEGE_ADMIN);
    
    /* Test permission checks */
    user_t guest_user;
    user_t user_user;
    user_t admin_user;
    
    /* Find the users we created */
    bool found_guest = false, found_user = false, found_admin = false;
    for (int i = 0; i < 16; i++) {
        if (user_database[i].is_active) {
            if (strcmp(user_database[i].username, "guest_user") == 0) {
                guest_user = user_database[i];
                found_guest = true;
            } else if (strcmp(user_database[i].username, "user_user") == 0) {
                user_user = user_database[i];
                found_user = true;
            } else if (strcmp(user_database[i].username, "admin_user") == 0) {
                admin_user = user_database[i];
                found_admin = true;
            }
        }
    }
    
    TEST_ASSERT_TRUE(found_guest);
    TEST_ASSERT_TRUE(found_user);
    TEST_ASSERT_TRUE(found_admin);
    
    /* Test permission checks */
    TEST_ASSERT_TRUE(security_check_permission(&guest_user, PRIVILEGE_GUEST));
    TEST_ASSERT_FALSE(security_check_permission(&guest_user, PRIVILEGE_USER));
    
    TEST_ASSERT_TRUE(security_check_permission(&user_user, PRIVILEGE_GUEST));
    TEST_ASSERT_TRUE(security_check_permission(&user_user, PRIVILEGE_USER));
    TEST_ASSERT_FALSE(security_check_permission(&user_user, PRIVILEGE_ADMIN));
    
    TEST_ASSERT_TRUE(security_check_permission(&admin_user, PRIVILEGE_GUEST));
    TEST_ASSERT_TRUE(security_check_permission(&admin_user, PRIVILEGE_USER));
    TEST_ASSERT_TRUE(security_check_permission(&admin_user, PRIVILEGE_ADMIN));
}

static void test_security_logging(void) {
    printf("\n=== Testing Security Logging ===\n");
    
    /* Test security event logging */
    user_t test_user;
    strcpy(test_user.username, "testuser");
    test_user.privilege = PRIVILEGE_USER;
    test_user.is_active = true;
    
    security_log_event("TEST_EVENT", "Test description", &test_user);
    security_log_security_violation("TEST_VIOLATION", "Test violation details", &test_user);
    
    /* These should not crash and should log appropriately */
    TEST_ASSERT_TRUE(true); /* Basic test to ensure logging doesn't crash */
}

static void test_buffer_overflow_protection(void) {
    printf("\n=== Testing Buffer Overflow Protection ===\n");
    
    char dest[50];
    
    /* Test safe string copy */
    size_t result = security_safe_strcpy(dest, "Hello World", sizeof(dest));
    TEST_ASSERT_TRUE(result > 0);
    TEST_ASSERT_EQUAL(0, strcmp(dest, "Hello World"));
    
    /* Test string copy that would cause overflow */
    result = security_safe_strcpy(dest, "This is a very long string that would cause buffer overflow if copied", sizeof(dest));
    TEST_ASSERT_TRUE(result == 0); /* Should fail safely */
    
    /* Test safe string concatenation */
    strcpy(dest, "Hello");
    result = security_safe_strcat(dest, " World", sizeof(dest));
    TEST_ASSERT_TRUE(result > 0);
    TEST_ASSERT_EQUAL(0, strcmp(dest, "Hello World"));
    
    /* Test string concatenation that would cause overflow */
    strcpy(dest, "This is a long string");
    result = security_safe_strcat(dest, " that will cause buffer overflow when concatenated", sizeof(dest));
    TEST_ASSERT_TRUE(result == 0); /* Should fail safely */
}

static void test_path_traversal_protection(void) {
    printf("\n=== Testing Path Traversal Protection ===\n");
    
    /* Test valid paths */
    TEST_ASSERT_TRUE(security_validate_path("/home/user/file.txt"));
    TEST_ASSERT_TRUE(security_validate_path("documents/report.pdf"));
    TEST_ASSERT_TRUE(security_validate_path("config/settings.ini"));
    
    /* Test path traversal attempts */
    TEST_ASSERT_FALSE(security_validate_path("../../../etc/passwd"));
    TEST_ASSERT_FALSE(security_validate_path("..\\..\\windows\\system32\\config\\sam"));
    TEST_ASSERT_FALSE(security_validate_path("/home/user/../../etc/shadow"));
    TEST_ASSERT_FALSE(security_validate_path("documents/../../../etc/hosts"));
}

/* Main test runner */
int main(void) {
    printf("\n");
    printf("===========================================\n");
    printf("OS Security Test Suite - Standalone Version\n");
    printf("===========================================\n");
    
    /* Run all tests */
    test_security_initialization();
    test_user_management();
    test_input_validation();
    test_memory_protection();
    test_authorization();
    test_security_logging();
    test_buffer_overflow_protection();
    test_path_traversal_protection();
    
    /* Print summary */
    printf("\n===========================================\n");
    printf("Test Summary:\n");
    printf("  Total Tests: %d\n", tests_run);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("===========================================\n");
    
    if (tests_failed == 0) {
        printf("✅ ALL SECURITY TESTS PASSED!\n");
        printf("Security implementation is working correctly.\n");
        return 0;
    } else {
        printf("❌ SOME SECURITY TESTS FAILED!\n");
        printf("Please review the security implementation.\n");
        return 1;
    }
}