/* test_config.h - Configuration for Unity testing framework */

#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

/* Unity configuration */
#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_PRECISION 1e-12

/* Mock definitions for OS functions */
#define TEST_MOCK

/* Test memory size */
#define TEST_MEMORY_SIZE 8192

/* Test file system size */
#define TEST_FS_SIZE 4096

/* Test buffer sizes */
#define TEST_BUFFER_SIZE 1024
#define TEST_FILENAME_MAX 64

#endif /* TEST_CONFIG_H */