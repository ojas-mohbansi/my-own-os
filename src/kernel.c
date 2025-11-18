/* kernel.c - minimal kernel with VGA console, memory integration, and security features
   Bootstraps core subsystems (security, paging, allocator, filesystem, shell) and
   provides basic error handling/panic and VGA text output helpers. */

#include <stdint.h>
#include <stddef.h>
#include "security.h"
#include "brand.h"
#include "scalability.h"

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

/* Forward declarations */
static void boot_animation(void);

/* Convenience macro for error handling */
#define HANDLE_ERROR(code) handle_error(code, __FUNCTION__, __FILE__, __LINE__)
#define CHECK_NULL(ptr) do { if ((ptr) == NULL) { HANDLE_ERROR(ERR_NULL_POINTER); return; } } while(0)
#define CHECK_ERROR(code) do { if (code != ERR_SUCCESS) { HANDLE_ERROR(code); return code; } } while(0)

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

static uint16_t* vga_buffer = (uint16_t*)VGA_BUFFER;
static size_t vga_row = 0;
static size_t vga_col = 0;

/* scroll the screen up by one row */
static void vga_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; ++y) {
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    /* clear bottom row */
    for (size_t x = 0; x < VGA_WIDTH; ++x) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
    }
}

/* put a character at current cursor */
static void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else {
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = (VGA_COLOR_WHITE_ON_BLACK << 8) | (uint8_t)c;
        vga_col++;
    }
    /* wrap and scroll */
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }
    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
        vga_row = VGA_HEIGHT - 1;
    }
}

/* print a null-terminated string */
void print(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

static void boot_banner(void) {
    print("\n");
    print(OS_NAME);
    print(" ");
    print(OS_VERSION);
    print("\n");
}

static void boot_spinner_step(int i) {
    const char* s = "|/-\\";
    print_char(s[i & 3]);
    print_char('\r');
}

/* very basic system-call handler placeholder */
void sys_call_handler(void) {
    /* TODO: inspect eax for call number, dispatch */
    print("[syscall]\n");
}

/* Kernel panic function with security logging */
void panic(const char* msg) {
    print("\n*** KERNEL PANIC ***\n");
    print(msg);
    print("\nSystem halted.\n");
    
    /* Halt the system */
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/* Enhanced error handling function with security logging */
void handle_error(int32_t error_code, const char* function, const char* file, uint32_t line) {
    error_level_t level;
    const char* error_msg;
    
    /* Determine error level and message based on error code */
    switch (error_code) {
        case ERR_SUCCESS:
            return; /* No error to handle */
            
        case ERR_NULL_POINTER:
            level = ERR_LEVEL_ERROR;
            error_msg = "Null pointer error";
            break;
            
        case ERR_OUT_OF_MEMORY:
            level = ERR_LEVEL_CRITICAL;
            error_msg = "Out of memory";
            break;
            
        case ERR_FILE_NOT_FOUND:
            level = ERR_LEVEL_WARNING;
            error_msg = "File not found";
            break;
            
        case ERR_FILE_EXISTS:
            level = ERR_LEVEL_WARNING;
            error_msg = "File already exists";
            break;
            
        case ERR_INVALID_PARAMETER:
            level = ERR_LEVEL_ERROR;
            error_msg = "Invalid parameter";
            break;
            
        case ERR_BUFFER_OVERFLOW:
            level = ERR_LEVEL_ERROR;
            error_msg = "Buffer overflow";
            break;
            
        case ERR_IO_TIMEOUT:
            level = ERR_LEVEL_WARNING;
            error_msg = "I/O timeout";
            break;
            
        case ERR_IO_DEVICE_ERROR:
            level = ERR_LEVEL_ERROR;
            error_msg = "I/O device error";
            break;
            
        case ERR_UNKNOWN_COMMAND:
            level = ERR_LEVEL_INFO;
            error_msg = "Unknown command";
            break;
            
        case ERR_INVALID_ARGUMENT:
            level = ERR_LEVEL_WARNING;
            error_msg = "Invalid argument";
            break;
            
        case ERR_COMMAND_TOO_LONG:
            level = ERR_LEVEL_WARNING;
            error_msg = "Command too long";
            break;
            
        case ERR_TOO_MANY_ARGUMENTS:
            level = ERR_LEVEL_WARNING;
            error_msg = "Too many arguments";
            break;
            
        case ERR_KERNEL_PANIC:
            level = ERR_LEVEL_FATAL;
            error_msg = "Kernel panic";
            break;
            
        /* Security-related errors */
        case ERR_SECURITY_VIOLATION:
            level = ERR_LEVEL_FATAL;
            error_msg = "Security violation";
            break;
            
        case ERR_INVALID_INPUT:
            level = ERR_LEVEL_ERROR;
            error_msg = "Invalid input";
            break;
            

            
        case ERR_UNAUTHORIZED_ACCESS:
            level = ERR_LEVEL_ERROR;
            error_msg = "Unauthorized access";
            break;
            
        case ERR_AUTHENTICATION_FAILED:
            level = ERR_LEVEL_WARNING;
            error_msg = "Authentication failed";
            break;
            
        case ERR_ACCESS_DENIED:
            level = ERR_LEVEL_WARNING;
            error_msg = "Access denied";
            break;
            
        case ERR_INVALID_PERMISSIONS:
            level = ERR_LEVEL_ERROR;
            error_msg = "Invalid permissions";
            break;
            
        case ERR_MEMORY_VIOLATION:
            level = ERR_LEVEL_ERROR;
            error_msg = "Memory access violation";
            break;
            
        default:
            level = ERR_LEVEL_ERROR;
            error_msg = "Unknown error";
            break;
    }
    
    /* Print error information */
    print("\n[ERROR] ");
    
    /* Print error level */
    switch (level) {
        case ERR_LEVEL_INFO:
            print("INFO: ");
            break;
        case ERR_LEVEL_WARNING:
            print("WARNING: ");
            break;
        case ERR_LEVEL_ERROR:
            print("ERROR: ");
            break;
        case ERR_LEVEL_CRITICAL:
            print("CRITICAL: ");
            break;
        case ERR_LEVEL_FATAL:
            print("FATAL: ");
            break;
    }
    
    print(error_msg);
    print(" (code: ");
    
    /* Print error code in hex */
    uint32_t code = (uint32_t)(-error_code);
    for (int i = 7; i >= 0; --i) {
        uint8_t nibble = (code >> (i * 4)) & 0xF;
        print_char(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
    print(")\n");
    
    /* Print location information if provided */
    if (function && file) {
        print("  Location: ");
        print(function);
        print("() in ");
        print(file);
        print(":");
        
        /* Print line number */
        uint32_t line_num = line;
        if (line_num == 0) {
            print("unknown");
        } else {
            char line_str[16];
            int i = 0;
            do {
                line_str[i++] = '0' + (line_num % 10);
                line_num /= 10;
            } while (line_num > 0);
            
            for (int j = i - 1; j >= 0; --j) {
                print_char(line_str[j]);
            }
        }
        print("\n");
    }
    
    /* Handle fatal errors */
    if (level == ERR_LEVEL_FATAL) {
        panic("Fatal error encountered");
    }
}

/* Simple strlen implementation */
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

/* kernel entry point called by bootloader */
void kernel_main_c(void) __attribute__((externally_visible));
void kernel_main_c(void) {
    
    /* clear screen using new I/O function */
    clear_screen();

    boot_animation();

    /* Initialize memory subsystem */
    init_paging();
    init_memory_management();

    /* simple alloc/free demo */
    void* p = allocate_memory(4096);
    if (p) {
        print("Allocated one page at 0x");
        /* crude hex print */
        uint32_t addr = (uint32_t)p;
        for (int i = 7; i >= 0; --i) {
            uint8_t nibble = (addr >> (i * 4)) & 0xF;
            vga_putchar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
        }
        vga_putchar('\n');
        free_memory(p);
        print("Page freed.\n");
    }

    /* Initialize file system */
    print("\n--- File System Demo ---\n");
    
    /* Allocate memory for file system */
    void* fs_memory = allocate_memory(64 * 1024);  /* 64KB for file system */
    if (!fs_memory) {
        print("Failed to allocate memory for file system\n");
    } else {
        FileSystem* fs = (FileSystem*)allocate_memory(sizeof(FileSystem));
        if (fs) {
            uint8_t* data_memory = (uint8_t*)fs_memory;
            
            /* Initialize file system */
            fs_init(fs, data_memory, 64 * 1024);
            print("File system initialized\n");
            
            /* Create some files */
            int hello_file = fs_create_file(fs, "hello.txt", 0);
            if (hello_file >= 0) {
                print("Created file: hello.txt\n");
                
                /* Write data to file */
                const char* content = "Hello from file system!\nThis is a test file.\n";
                int bytes_written = fs_write_file(fs, hello_file, (const uint8_t*)content, strlen(content), 0);
                if (bytes_written > 0) {
                    print_char('"');
                    print(content);
                    print_char('"');
                    print("\n");
                    print("Data written to file: ");
                    uint32_t addr = (uint32_t)bytes_written;
                    for (int i = 7; i >= 0; --i) {
                        uint8_t nibble = (addr >> (i * 4)) & 0xF;
                        vga_putchar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
                    }
                    print(" bytes\n");
                }
                
                /* Read data back from file */
                char read_buffer[256];
                int bytes_read = fs_read_file(fs, hello_file, (uint8_t*)read_buffer, sizeof(read_buffer), 0);
                if (bytes_read > 0) {
                    read_buffer[bytes_read] = '\0';
                    print("Read from file: ");
                    print_char('"');
                    print(read_buffer);
                    print_char('"');
                    print("\n");
                }
            }
            
            /* Create a directory */
            int test_dir = fs_create_directory(fs, "testdir", 0);
            if (test_dir >= 0) {
                print("Created directory: testdir\n");
                
                /* Create a file in the directory */
                int dir_file = fs_create_file(fs, "dirfile.txt", test_dir);
                if (dir_file >= 0) {
                    print("Created file in directory: dirfile.txt\n");
                }
            }
            
            /* List root directory contents */
            print("\nRoot directory contents:\n");
            File entries[16];
            int count = fs_list_directory(fs, 0, entries, 16);
            for (int i = 0; i < count; i++) {
                print("  ");
                print(entries[i].type == FILE_TYPE_DIRECTORY ? "[DIR]  " : "[FILE] ");
                print(entries[i].name);
                print(" (");
                uint32_t size = entries[i].size;
                for (int j = 7; j >= 0; --j) {
                    uint8_t nibble = (size >> (j * 4)) & 0xF;
                    vga_putchar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
                }
                print(" bytes)\n");
            }
            
            /* Test error handling */
            print("\nTesting error handling:\n");
            int duplicate = fs_create_file(fs, "hello.txt", 0);
            if (duplicate < 0) {
                print("Expected error creating duplicate file: ");
                print(fs_error_string(duplicate));
                print("\n");
            }
            
            /* Clean up */
            free_memory(fs);
            free_memory(fs_memory);
            print("File system memory freed\n");
        }
    }

    print("\n--- Kernel Demo Complete ---\n");
    print("S00K OS demo complete. System halted.\n");

    /* hang */
    while (1) {
        __asm__ volatile ("hlt");
    }
}
static int brand_anim_enabled = 1;

static void boot_phase(const char* name, int spins) {
    print(name);
    print(" ");
    for (int i = 0; i < spins; i++) {
        const char* s = "|/-\\";
        print_char(s[i & 3]);
        print_char('\r');
    }
    print("\n");
}

static void boot_animation(void) {
    if (!brand_anim_enabled) return;
    print("\n");
    print(OS_NAME);
    print(" ");
    print(OS_VERSION);
    print("\n");
    boot_phase("Paging", 12);
    boot_phase("Memory", 12);
    boot_phase("File System", 12);
    boot_phase("I/O", 12);
}