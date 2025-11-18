/* test_runner.c - Main test runner for all unit tests */

#include "unity.h"
#include "test_config.h"

/* External test runners */
extern int run_kernel_tests(void);
extern int run_memory_management_tests(void);
extern int run_io_tests(void);
extern int run_file_system_tests(void);
extern int run_scalability_tests(void);

/* Test summary */
static int total_tests = 0;
static int failed_tests = 0;

/* Main function */
int main(void) {
    printf("========================================\n");
    printf("Operating System Unit Tests\n");
    printf("========================================\n\n");
    
    int result;
    
    /* Run kernel tests */
    printf("Running Kernel Tests...\n");
    printf("------------------------\n");
    result = run_kernel_tests();
    if (result != 0) {
        failed_tests++;
    }
    total_tests++;
    printf("\n");
    
    /* Run memory management tests */
    printf("Running Memory Management Tests...\n");
    printf("-----------------------------------\n");
    result = run_memory_management_tests();
    if (result != 0) {
        failed_tests++;
    }
    total_tests++;
    printf("\n");
    
    /* Run I/O tests */
    printf("Running I/O Tests...\n");
    printf("---------------------\n");
    result = run_io_tests();
    if (result != 0) {
        failed_tests++;
    }
    total_tests++;
    printf("\n");
    
    /* Run file system tests */
    printf("Running File System Tests...\n");
    printf("-----------------------------\n");
    result = run_file_system_tests();
    if (result != 0) {
        failed_tests++;
    }
    total_tests++;
    printf("\n");

    /* Run scalability tests */
    printf("Running Scalability Tests...\n");
    printf("----------------------------\n");
    result = run_scalability_tests();
    if (result != 0) {
        failed_tests++;
    }
    total_tests++;
    printf("\n");
    
    /* Print summary */
    printf("========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Total Test Suites: %d\n", total_tests);
    printf("Failed Test Suites: %d\n", failed_tests);
    printf("Passed Test Suites: %d\n", total_tests - failed_tests);
    
    if (failed_tests == 0) {
        printf("\n✅ ALL TESTS PASSED! ✅\n");
        return 0;
    } else {
        printf("\n❌ SOME TESTS FAILED ❌\n");
        return 1;
    }
}