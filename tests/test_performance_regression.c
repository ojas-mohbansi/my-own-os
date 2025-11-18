#include "../src/performance_profiler.h"
#include "../src/kernel_optimized.h"
#include "../src/memory_management_optimized.h"
#include "../src/io_optimized.h"
#include "../src/kernel.h"
#include "../src/memory_management.h"
#include "../src/io.h"
#include "test_framework.h"
#include <stdio.h>
#include <string.h>

#define PERFORMANCE_THRESHOLD_NS 1000000  /* 1ms threshold for performance regressions */
#define ITERATIONS 1000

typedef struct {
    const char* test_name;
    uint64_t baseline_time_ns;
    uint64_t optimized_time_ns;
    double improvement_factor;
    int passed;
} performance_test_result_t;

static performance_test_result_t results[50];
static int result_count = 0;

static void add_performance_result(const char* test_name, uint64_t baseline, uint64_t optimized) {
    if (result_count >= 50) return;
    
    results[result_count].test_name = test_name;
    results[result_count].baseline_time_ns = baseline;
    results[result_count].optimized_time_ns = optimized;
    results[result_count].improvement_factor = (double)baseline / (double)optimized;
    results[result_count].passed = (optimized <= baseline) || 
                                   ((baseline - optimized) < PERFORMANCE_THRESHOLD_NS);
    result_count++;
}

static void print_performance_summary(void) {
    printf("\n=== PERFORMANCE REGRESSION TEST SUMMARY ===\n");
    printf("%-40s %15s %15s %10s %8s\n", 
           "Test Name", "Baseline (ns)", "Optimized (ns)", "Speedup", "Status");
    printf("%-40s %15s %15s %10s %8s\n", 
           "----------------------------------------",
           "---------------", "---------------", "----------", "--------");
    
    for (int i = 0; i < result_count; i++) {
        printf("%-40s %15llu %15llu %9.2fx %8s\n",
               results[i].test_name,
               results[i].baseline_time_ns,
               results[i].optimized_time_ns,
               results[i].improvement_factor,
               results[i].passed ? "PASS" : "FAIL");
    }
    
    int passed = 0;
    for (int i = 0; i < result_count; i++) {
        if (results[i].passed) passed++;
    }
    
    printf("\nTotal: %d/%d tests passed (%.1f%%)\n", 
           passed, result_count, (100.0 * passed) / result_count);
}

/* Test kernel VGA scroll performance */
static void test_kernel_vga_scroll_performance(void) {
    printf("Testing kernel VGA scroll performance...\n");
    
    /* Baseline measurement */
    profiler_start_session("baseline_vga_scroll");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_clear_screen();
        kernel_scroll_screen();
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_vga_scroll");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_clear_screen_optimized();
        kernel_scroll_screen_optimized();
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("VGA Scroll", baseline_time, optimized_time);
}

/* Test memory allocation performance */
static void test_memory_allocation_performance(void) {
    printf("Testing memory allocation performance...\n");
    
    /* Baseline measurement */
    profiler_start_session("baseline_memory_alloc");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        void* ptr = kernel_malloc(64);
        if (ptr) kernel_free(ptr);
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_memory_alloc");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        void* ptr = kernel_malloc_optimized(64);
        if (ptr) kernel_free_optimized(ptr);
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("Memory Allocation", baseline_time, optimized_time);
}

/* Test page allocation performance */
static void test_page_allocation_performance(void) {
    printf("Testing page allocation performance...\n");
    
    /* Baseline measurement */
    profiler_start_session("baseline_page_alloc");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS / 10; i++) {
        uint32_t page = kernel_alloc_page();
        if (page != INVALID_PAGE) kernel_free_page(page);
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_page_alloc");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS / 10; i++) {
        uint32_t page = kernel_alloc_page_optimized();
        if (page != INVALID_PAGE) kernel_free_page_optimized(page);
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("Page Allocation", baseline_time, optimized_time);
}

/* Test keyboard input performance */
static void test_keyboard_input_performance(void) {
    printf("Testing keyboard input performance...\n");
    
    /* Baseline measurement */
    profiler_start_session("baseline_keyboard");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_handle_keyboard_interrupt();
        kernel_get_keyboard_char();
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_keyboard");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_handle_keyboard_interrupt_optimized();
        kernel_get_keyboard_char_optimized();
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("Keyboard Input", baseline_time, optimized_time);
}

/* Test VGA text output performance */
static void test_vga_text_output_performance(void) {
    printf("Testing VGA text output performance...\n");
    
    /* Baseline measurement */
    profiler_start_session("baseline_vga_output");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_print_string("Hello, World!");
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_vga_output");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_print_string_optimized("Hello, World!");
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("VGA Text Output", baseline_time, optimized_time);
}

/* Test memory copy performance */
static void test_memory_copy_performance(void) {
    printf("Testing memory copy performance...\n");
    
    char src[1024];
    char dst[1024];
    memset(src, 'A', sizeof(src));
    
    /* Baseline measurement */
    profiler_start_session("baseline_memcpy");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_memcpy(dst, src, sizeof(src));
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_memcpy");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_memcpy_optimized(dst, src, sizeof(src));
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("Memory Copy", baseline_time, optimized_time);
}

/* Test string operations performance */
static void test_string_operations_performance(void) {
    printf("Testing string operations performance...\n");
    
    const char* test_str = "Hello, World! This is a test string.";
    
    /* Baseline measurement */
    profiler_start_session("baseline_strlen");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_strlen(test_str);
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_strlen");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS; i++) {
        kernel_strlen_optimized(test_str);
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("String Length", baseline_time, optimized_time);
}

/* Comprehensive system performance test */
static void test_system_performance(void) {
    printf("Testing overall system performance...\n");
    
    /* Baseline measurement */
    profiler_start_session("baseline_system");
    uint64_t baseline_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS / 10; i++) {
        kernel_clear_screen();
        kernel_print_string("System Performance Test");
        void* ptr = kernel_malloc(256);
        if (ptr) {
            kernel_memcpy(ptr, "Test data", 10);
            kernel_free(ptr);
        }
        kernel_handle_keyboard_interrupt();
    }
    
    uint64_t baseline_end = profiler_get_current_time_ns();
    uint64_t baseline_time = baseline_end - baseline_start;
    profiler_end_session();
    
    /* Optimized measurement */
    profiler_start_session("optimized_system");
    uint64_t optimized_start = profiler_get_current_time_ns();
    
    for (int i = 0; i < ITERATIONS / 10; i++) {
        kernel_clear_screen_optimized();
        kernel_print_string_optimized("System Performance Test");
        void* ptr = kernel_malloc_optimized(256);
        if (ptr) {
            kernel_memcpy_optimized(ptr, "Test data", 10);
            kernel_free_optimized(ptr);
        }
        kernel_handle_keyboard_interrupt_optimized();
    }
    
    uint64_t optimized_end = profiler_get_current_time_ns();
    uint64_t optimized_time = optimized_end - optimized_start;
    profiler_end_session();
    
    add_performance_result("Overall System", baseline_time, optimized_time);
}

/* Function correctness test to ensure optimizations didn't break functionality */
static void test_function_correctness(void) {
    printf("Testing function correctness after optimization...\n");
    
    /* Test memory operations */
    char test_src[100] = "Hello, World!";
    char test_dst1[100] = {0};
    char test_dst2[100] = {0};
    
    kernel_memcpy(test_dst1, test_src, 14);
    kernel_memcpy_optimized(test_dst2, test_src, 14);
    
    ASSERT(strcmp(test_dst1, test_dst2) == 0, "Memory copy optimization broke functionality");
    
    /* Test string operations */
    size_t len1 = kernel_strlen(test_src);
    size_t len2 = kernel_strlen_optimized(test_src);
    
    ASSERT(len1 == len2, "String length optimization broke functionality");
    
    /* Test memory allocation */
    void* ptr1 = kernel_malloc(64);
    void* ptr2 = kernel_malloc_optimized(64);
    
    ASSERT(ptr1 != NULL, "Baseline malloc failed");
    ASSERT(ptr2 != NULL, "Optimized malloc failed");
    
    kernel_free(ptr1);
    kernel_free_optimized(ptr2);
    
    printf("Function correctness tests passed!\n");
}

/* Main performance regression test runner */
void run_performance_regression_tests(void) {
    printf("=== STARTING PERFORMANCE REGRESSION TESTS ===\n\n");
    
    /* Initialize profiling system */
    profiler_init();
    
    /* Run correctness tests first */
    test_function_correctness();
    printf("\n");
    
    /* Run performance tests */
    test_kernel_vga_scroll_performance();
    printf("\n");
    
    test_memory_allocation_performance();
    printf("\n");
    
    test_page_allocation_performance();
    printf("\n");
    
    test_keyboard_input_performance();
    printf("\n");
    
    test_vga_text_output_performance();
    printf("\n");
    
    test_memory_copy_performance();
    printf("\n");
    
    test_string_operations_performance();
    printf("\n");
    
    test_system_performance();
    printf("\n");
    
    /* Print summary */
    print_performance_summary();
    
    /* Generate profiling report */
    profiler_generate_report("performance_regression_report.txt");
    
    printf("\nPerformance regression tests completed!\n");
    printf("Check 'performance_regression_report.txt' for detailed profiling data.\n");
}

/* Entry point for performance regression tests */
int main(void) {
    run_performance_regression_tests();
    return 0;
}