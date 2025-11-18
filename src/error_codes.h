/* error_codes.h - Comprehensive error codes for the operating system
   Grouped by subsystem (general, I/O, FS, memory, shell, kernel) with
   a severity enum for structured reporting. */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <stdint.h>

/* Success code */
#define ERR_SUCCESS             0

/* General system errors */
#define ERR_NULL_POINTER        -1
#define ERR_INVALID_PARAMETER   -2
#define ERR_OUT_OF_MEMORY       -3
#define ERR_BUFFER_OVERFLOW     -4
#define ERR_INVALID_STATE       -5

/* I/O errors */
#define ERR_IO_TIMEOUT          -10
#define ERR_IO_DEVICE_ERROR     -11
#define ERR_IO_INVALID_PORT     -12
#define ERR_IO_BUSY             -13

/* File system errors */
#define ERR_FILE_NOT_FOUND          -20
#define ERR_FILE_EXISTS             -21
#define ERR_FILE_TOO_LARGE          -22
#define ERR_INVALID_FILENAME        -23
#define ERR_DIRECTORY_FULL          -24
#define ERR_DISK_FULL               -25
#define ERR_PERMISSION_DENIED       -26
#define ERR_NOT_A_DIRECTORY         -27
#define ERR_IS_A_DIRECTORY          -28
#define ERR_FILE_NAME_TOO_LONG      -29
#define ERR_FILE_CORRUPTED          -30
#define ERR_INVALID_FILE_HANDLE     -31
#define ERR_NOT_A_FILE              -32
#define ERR_INVALID_DIRECTORY       -33
#define ERR_DIRECTORY_NOT_EMPTY     -34
#define ERR_FILE_SYSTEM_FULL        -35
#define ERR_OUT_OF_SPACE            -36
#define ERR_FILE_SYSTEM_INIT_FAILED -37

/* Memory errors */
#define ERR_INVALID_ADDRESS     -50
#define ERR_MEMORY_CORRUPTION   -51
#define ERR_PAGE_FAULT          -52
#define ERR_STACK_OVERFLOW      -53

/* Shell/CLI errors */
#define ERR_UNKNOWN_COMMAND     -60
#define ERR_INVALID_ARGUMENT    -61
#define ERR_COMMAND_TOO_LONG    -62
#define ERR_TOO_MANY_ARGUMENTS  -63

/* Kernel errors */
#define ERR_KERNEL_PANIC        -70
#define ERR_INVALID_SYSCALL     -71
#define ERR_DIVISION_BY_ZERO    -72
#define ERR_INVALID_OPCODE      -73

/* Error severity levels */
typedef enum {
    ERR_LEVEL_INFO,
    ERR_LEVEL_WARNING,
    ERR_LEVEL_ERROR,
    ERR_LEVEL_CRITICAL,
    ERR_LEVEL_FATAL
} error_level_t;

/* Error information structure */
typedef struct {
    int32_t code;
    error_level_t level;
    const char* message;
    const char* function;
    const char* file;
    uint32_t line;
} error_info_t;

#endif /* ERROR_CODES_H */