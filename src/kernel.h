/* kernel.h - Kernel function declarations */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

/* Error handling function */
void handle_error(int32_t error_code, const char* function, const char* file, uint32_t line);

/* Panic function for fatal errors */
void panic(const char* msg);

/* Convenience macro for error handling */
#define HANDLE_ERROR(code) handle_error(code, __FUNCTION__, __FILE__, __LINE__)

/* I/O functions */
void print_char(char c);
void print(const char* str);
char read_char(void);
char read_char_timeout(uint32_t timeout_ms, int32_t* error_code);
void clear_screen(void);

/* Safe I/O functions with error checking */
int32_t print_char_safe(char c);
int32_t print_string_safe(const char* str);

#endif /* KERNEL_H */