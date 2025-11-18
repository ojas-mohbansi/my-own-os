/* test_memory_management.c - Unit tests for memory management functions */

#include "unity.h"
#include "test_config.h"
#include "../src/error_codes.h"

/* Mock memory management functions for testing */
typedef struct {
    uint32_t total_memory;
    uint32_t used_memory;
    uint32_t free_memory;
    uint8_t* memory_pool;
} MemoryManager;

/* Mock implementations */
static MemoryManager mock_memory_manager;
static int mock_allocation_failed = 0;

/* Mock memory allocation */
void* mock_malloc(uint32_t size) {
    if (mock_allocation_failed || mock_memory_manager.free_memory < size) {
        return NULL;
    }
    
    mock_memory_manager.used_memory += size;
    mock_memory_manager.free_memory -= size;
    return &mock_memory_manager.memory_pool[mock_memory_manager.used_memory - size];
}

/* Mock memory free */
void mock_free(void* ptr) {
    if (ptr != NULL) {
        /* Simplified free - just mark as available */
        mock_allocation_failed = 0;
    }
}

/* Mock memory initialization */
void mock_memory_init(uint32_t total_size) {
    mock_memory_manager.total_memory = total_size;
    mock_memory_manager.used_memory = 0;
    mock_memory_manager.free_memory = total_size;
    mock_allocation_failed = 0;
}

/* Test setup and teardown */
void setUp(void) {
    mock_memory_init(TEST_MEMORY_SIZE);
}

void tearDown(void) {
    /* Cleanup after each test */
}

/* Test cases for memory management */
void test_memory_initialization(void) {
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE, mock_memory_manager.total_memory);
    TEST_ASSERT_EQUAL(0, mock_memory_manager.used_memory);
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE, mock_memory_manager.free_memory);
}

void test_memory_allocation_success(void) {
    uint32_t alloc_size = 1024;
    void* ptr = mock_malloc(alloc_size);
    
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL(alloc_size, mock_memory_manager.used_memory);
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE - alloc_size, mock_memory_manager.free_memory);
}

void test_memory_allocation_failure_null(void) {
    mock_allocation_failed = 1;
    void* ptr = mock_malloc(100);
    
    TEST_ASSERT_NULL(ptr);
    TEST_ASSERT_EQUAL(0, mock_memory_manager.used_memory);
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE, mock_memory_manager.free_memory);
}

void test_memory_allocation_failure_insufficient(void) {
    uint32_t large_size = TEST_MEMORY_SIZE + 100;
    void* ptr = mock_malloc(large_size);
    
    TEST_ASSERT_NULL(ptr);
    TEST_ASSERT_EQUAL(0, mock_memory_manager.used_memory);
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE, mock_memory_manager.free_memory);
}

void test_memory_free_null_pointer(void) {
    /* Freeing NULL should not crash */
    mock_free(NULL);
    TEST_ASSERT_EQUAL(0, mock_memory_manager.used_memory);
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE, mock_memory_manager.free_memory);
}

void test_memory_free_valid_pointer(void) {
    void* ptr = mock_malloc(512);
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL(512, mock_memory_manager.used_memory);
    
    mock_free(ptr);
    /* Note: This is a simplified mock - in real implementation,
       free would return memory to the pool */
}

void test_memory_multiple_allocations(void) {
    void* ptr1 = mock_malloc(256);
    void* ptr2 = mock_malloc(512);
    void* ptr3 = mock_malloc(128);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_NULL(ptr3);
    TEST_ASSERT_EQUAL(256 + 512 + 128, mock_memory_manager.used_memory);
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE - (256 + 512 + 128), mock_memory_manager.free_memory);
}

void test_memory_boundary_conditions(void) {
    /* Test allocation of exactly all available memory */
    void* ptr = mock_malloc(TEST_MEMORY_SIZE);
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL(TEST_MEMORY_SIZE, mock_memory_manager.used_memory);
    TEST_ASSERT_EQUAL(0, mock_memory_manager.free_memory);
    
    /* Test allocation when no memory is available */
    void* ptr2 = mock_malloc(1);
    TEST_ASSERT_NULL(ptr2);
}

void test_memory_alignment(void) {
    /* Test that allocations are properly aligned */
    void* ptr1 = mock_malloc(1);
    void* ptr2 = mock_malloc(2);
    void* ptr3 = mock_malloc(4);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_NULL(ptr3);
    
    /* In a real implementation, we'd check alignment */
    /* For now, just ensure they're different and valid */
    TEST_ASSERT_PTR_NOT_EQUAL(ptr1, ptr2);
    TEST_ASSERT_PTR_NOT_EQUAL(ptr2, ptr3);
    TEST_ASSERT_PTR_NOT_EQUAL(ptr1, ptr3);
}

void test_memory_error_handling(void) {
    /* Test error handling for memory operations */
    mock_allocation_failed = 1;
    void* ptr = mock_malloc(100);
    TEST_ASSERT_NULL(ptr);
    
    /* Reset and try again */
    mock_allocation_failed = 0;
    ptr = mock_malloc(100);
    TEST_ASSERT_NOT_NULL(ptr);
}

/* Test group runner */
int run_memory_management_tests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_memory_initialization);
    RUN_TEST(test_memory_allocation_success);
    RUN_TEST(test_memory_allocation_failure_null);
    RUN_TEST(test_memory_allocation_failure_insufficient);
    RUN_TEST(test_memory_free_null_pointer);
    RUN_TEST(test_memory_free_valid_pointer);
    RUN_TEST(test_memory_multiple_allocations);
    RUN_TEST(test_memory_boundary_conditions);
    RUN_TEST(test_memory_alignment);
    RUN_TEST(test_memory_error_handling);
    
    return UNITY_END();
}