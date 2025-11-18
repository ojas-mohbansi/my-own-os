/* kernel_optimized.c - Optimized kernel with performance profiling */

#include <stdint.h>
#include <stddef.h>
#include "performance_profiler.h"

/* provided by memory_management.c */
extern void init_memory_management(void);
extern void* allocate_memory(size_t size);
extern void free_memory(void* ptr);

/* provided by paging.c */
extern void init_paging(void);

/* provided by io.c */
extern void print_char(char c);
extern char read_char(void);
extern void clear_screen(void);

/* provided by file_system.c */
#include "file_system.h"

/* provided by shell.c */
#include "shell.h"

/* provided by error_codes.h */
#include "error_codes.h"

/* Convenience macro for error handling */
#define HANDLE_ERROR(code) handle_error(code, __FUNCTION__, __FILE__, __LINE__)
#define CHECK_NULL(ptr) do { if (UNLIKELY((ptr) == NULL)) { HANDLE_ERROR(ERR_NULL_POINTER); return; } } while(0)
#define CHECK_ERROR(code) do { if (UNLIKELY(code != ERR_SUCCESS)) { HANDLE_ERROR(code); return code; } } while(0)

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

static uint16_t* vga_buffer = (uint16_t*)VGA_BUFFER;
static size_t vga_row = 0;
static size_t vga_col = 0;

/* Performance optimization: Use register variables and prefetching */
static inline void optimized_vga_scroll(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_vga_scroll");
    profiler_start_function(__profile_id);
    
    register uint16_t* dst = vga_buffer;
    register uint16_t* src = vga_buffer + VGA_WIDTH;
    register size_t copy_size = (VGA_HEIGHT - 1) * VGA_WIDTH;
    
    /* Prefetch data for better cache performance */
    for (register size_t i = 0; i < copy_size; i += 16) {
        PREFETCH(src + i);
    }
    
    /* Optimized memory copy with loop unrolling */
    register size_t i = 0;
    for (; i < copy_size - 7; i += 8) {
        dst[i] = src[i];
        dst[i+1] = src[i+1];
        dst[i+2] = src[i+2];
        dst[i+3] = src[i+3];
        dst[i+4] = src[i+4];
        dst[i+5] = src[i+5];
        dst[i+6] = src[i+6];
        dst[i+7] = src[i+7];
    }
    
    /* Handle remaining elements */
    for (; i < copy_size; i++) {
        dst[i] = src[i];
    }
    
    /* Clear bottom row - optimized memset */
    register uint16_t clear_value = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
    register uint16_t* bottom_row = vga_buffer + (VGA_HEIGHT - 1) * VGA_WIDTH;
    
    for (register size_t x = 0; x < VGA_WIDTH; ++x) {
        bottom_row[x] = clear_value;
    }
    
    profiler_end_function(__profile_id);
}

/* Optimized VGA character output with reduced branching */
static inline void optimized_vga_putchar(char c) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_vga_putchar");
    profiler_start_function(__profile_id);
    
    register uint16_t* vga_ptr = vga_buffer + vga_row * VGA_WIDTH + vga_col;
    register uint16_t char_value = (VGA_COLOR_WHITE_ON_BLACK << 8) | (uint8_t)c;
    
    /* Use switch statement for better branch prediction */
    switch (c) {
        case '\n':
            vga_col = 0;
            vga_row++;
            break;
        case '\r':
            vga_col = 0;
            break;
        case '\t':
            vga_col = (vga_col + 8) & ~7;
            break;
        default:
            *vga_ptr = char_value;
            vga_col++;
            break;
    }
    
    /* Handle wrapping and scrolling with reduced branching */
    if (UNLIKELY(vga_col >= VGA_WIDTH)) {
        vga_col = 0;
        vga_row++;
    }
    if (UNLIKELY(vga_row >= VGA_HEIGHT)) {
        optimized_vga_scroll();
        vga_row = VGA_HEIGHT - 1;
    }
    
    profiler_end_function(__profile_id);
}

/* Optimized string printing with bulk operations */
void optimized_print(const char* str) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_print");
    profiler_start_function(__profile_id);
    
    CHECK_NULL(str);
    
    /* Use register variables for better performance */
    register const char* s = str;
    register size_t len = 0;
    
    /* Pre-calculate string length for better optimization */
    while (LIKELY(*s)) {
        optimized_vga_putchar(*s++);
        len++;
    }
    
    profiler_record_io_operation("write", len, __profile_id);
    profiler_end_function(__profile_id);
}

/* Optimized strlen using word-wise comparison */
static inline size_t optimized_strlen(const char* str) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_strlen");
    profiler_start_function(__profile_id);
    
    CHECK_NULL(str);
    
    register const char* s = str;
    
    /* Check for word alignment and use word-wise comparison if possible */
    if (LIKELY(((uintptr_t)s & 3) == 0)) {
        /* Word-aligned string - use 32-bit comparison */
        register const uint32_t* w = (const uint32_t*)s;
        
        while (LIKELY(*w != 0)) {
            /* Check each byte in the word */
            if (UNLIKELY((*w & 0xFF) == 0)) { profiler_end_function(__profile_id); return (const char*)w - str; }
            if (UNLIKELY((*w & 0xFF00) == 0)) { profiler_end_function(__profile_id); return (const char*)w - str + 1; }
            if (UNLIKELY((*w & 0xFF0000) == 0)) { profiler_end_function(__profile_id); return (const char*)w - str + 2; }
            if (UNLIKELY((*w & 0xFF000000) == 0)) { profiler_end_function(__profile_id); return (const char*)w - str + 3; }
            w++;
        }
        
        /* Find exact position in last word */
        s = (const char*)w;
    }
    
    /* Handle remaining bytes or non-aligned strings */
    while (LIKELY(*s)) {
        s++;
    }
    
    size_t result = s - str;
    profiler_end_function(__profile_id);
    return result;
}

/* Optimized hex printing for error codes */
static inline void optimized_print_hex(uint32_t value) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_print_hex");
    profiler_start_function(__profile_id);
    
    /* Use lookup table for faster hex conversion */
    static const char hex_chars[] = "0123456789ABCDEF";
    char buffer[9]; /* 8 hex digits + null terminator */
    buffer[8] = '\0';
    
    /* Fill buffer from right to left */
    for (int i = 7; i >= 0; --i) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    optimized_print(buffer);
    profiler_end_function(__profile_id);
}

/* Optimized error handling with reduced string operations */
void optimized_handle_error(int32_t error_code, const char* function, const char* file, uint32_t line) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_handle_error");
    profiler_start_function(__profile_id);
    
    error_level_t level;
    const char* error_msg;
    
    /* Use lookup table for faster error code processing */
    static const struct {
        int32_t code;
        error_level_t level;
        const char* message;
    } error_table[] = {
        {ERR_SUCCESS, ERR_LEVEL_INFO, "Success"},
        {ERR_NULL_POINTER, ERR_LEVEL_ERROR, "Null pointer error"},
        {ERR_OUT_OF_MEMORY, ERR_LEVEL_CRITICAL, "Out of memory"},
        {ERR_FILE_NOT_FOUND, ERR_LEVEL_WARNING, "File not found"},
        {ERR_FILE_EXISTS, ERR_LEVEL_WARNING, "File already exists"},
        {ERR_INVALID_PARAMETER, ERR_LEVEL_ERROR, "Invalid parameter"},
        {ERR_BUFFER_OVERFLOW, ERR_LEVEL_ERROR, "Buffer overflow"},
        {ERR_IO_TIMEOUT, ERR_LEVEL_WARNING, "I/O timeout"},
        {ERR_IO_DEVICE_ERROR, ERR_LEVEL_ERROR, "I/O device error"},
        {ERR_UNKNOWN_COMMAND, ERR_LEVEL_INFO, "Unknown command"},
        {ERR_INVALID_ARGUMENT, ERR_LEVEL_WARNING, "Invalid argument"},
        {ERR_COMMAND_TOO_LONG, ERR_LEVEL_WARNING, "Command too long"},
        {ERR_TOO_MANY_ARGUMENTS, ERR_LEVEL_WARNING, "Too many arguments"},
        {ERR_KERNEL_PANIC, ERR_LEVEL_FATAL, "Kernel panic"},
    };
    
    /* Fast lookup using binary search or direct indexing */
    int found = 0;
    for (int i = 0; i < sizeof(error_table) / sizeof(error_table[0]); i++) {
        if (error_table[i].code == error_code) {
            level = error_table[i].level;
            error_msg = error_table[i].message;
            found = 1;
            break;
        }
    }
    
    if (!found) {
        level = ERR_LEVEL_ERROR;
        error_msg = "Unknown error";
    }
    
    /* Optimized error printing */
    if (level == ERR_LEVEL_FATAL) {
        optimized_print("\n*** KERNEL PANIC ***\n");
        optimized_print(error_msg);
        optimized_print("\nSystem halted.\n");
        
        /* Halt the system */
        while (1) {
            __asm__ volatile ("hlt");
        }
    }
    
    profiler_end_function(__profile_id);
}

/* Optimized system call handler */
void optimized_sys_call_handler(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_sys_call_handler");
    profiler_start_function(__profile_id);
    
    /* TODO: inspect eax for call number, dispatch */
    optimized_print("[syscall]\n");
    
    profiler_end_function(__profile_id);
}

/* Optimized kernel entry point */
void kernel_main_optimized(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("kernel_main_optimized");
    profiler_start_function(__profile_id);
    
    /* Initialize profiling */
    profiler_init();
    
    /* clear screen using new I/O function */
    clear_screen();
    
    /* Welcome message */
    optimized_print("Hello, World!\n");
    optimized_print("S00K OS kernel running.\n");
    optimized_print("Performance profiling enabled.\n");
    
    /* initialize memory subsystem */
    init_paging();
    init_memory_management();
    
    /* simple alloc/free demo */
    void* p = allocate_memory(4096);
    if (p) {
        optimized_print("Allocated one page at 0x");
        /* crude hex print */
        uint32_t addr = (uint32_t)p;
        optimized_print_hex(addr);
        optimized_print("\n");
        free_memory(p);
        optimized_print("Page freed.\n");
    }
    
    /* Initialize file system */
    optimized_print("\n--- File System Demo ---\n");
    
    /* Allocate memory for file system */
    void* fs_memory = allocate_memory(64 * 1024);  /* 64KB for file system */
    if (!fs_memory) {
        optimized_print("Failed to allocate memory for file system\n");
    } else {
        FileSystem* fs = (FileSystem*)allocate_memory(sizeof(FileSystem));
        if (fs) {
            uint8_t* data_memory = (uint8_t*)fs_memory;
            
            /* Initialize file system */
            fs_init(fs, data_memory, 64 * 1024);
            optimized_print("File system initialized\n");
            
            /* Create some files */
            int hello_file = fs_create_file(fs, "hello.txt", 0);
            if (hello_file >= 0) {
                optimized_print("Created file: hello.txt\n");
                
                /* Write data to file */
                const char* content = "Hello from optimized file system!\nThis is a test file.\n";
                int bytes_written = fs_write_file(fs, hello_file, (const uint8_t*)content, optimized_strlen(content), 0);
                if (bytes_written > 0) {
                    optimized_print("Data written to file: ");
                    uint32_t addr = (uint32_t)bytes_written;
                    optimized_print_hex(addr);
                    optimized_print(" bytes\n");
                }
                
                /* Read data back from file */
                char read_buffer[256];
                int bytes_read = fs_read_file(fs, hello_file, (uint8_t*)read_buffer, sizeof(read_buffer), 0);
                if (bytes_read > 0) {
                    read_buffer[bytes_read] = '\0';
                    optimized_print("Read from file: \"");
                    optimized_print(read_buffer);
                    optimized_print("\"\n");
                }
            }
            
            /* Clean up */
            free_memory(fs);
            free_memory(fs_memory);
            optimized_print("File system memory freed\n");
        }
    }
    
    optimized_print("\n--- Performance Report ---\n");
    profiler_print_report();
    
    optimized_print("\nKernel demo complete. System halted.\n");
    
    /* hang */
    while (1) {
        __asm__ volatile ("hlt");
    }
    
    profiler_end_function(__profile_id);
}