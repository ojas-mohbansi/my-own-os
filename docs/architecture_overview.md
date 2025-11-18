# S00K OS Architecture Overview

## Table of Contents
- [System Architecture](#system-architecture)
- [Boot Process](#boot-process)
- [Kernel Architecture](#kernel-architecture)
- [Memory Management](#memory-management)
- [File System](#file-system)
- [Input/Output System](#inputoutput-system)
- [Security Architecture](#security-architecture)
- [Shell and User Interface](#shell-and-user-interface)
- [Error Handling](#error-handling)
- [Performance and Optimization](#performance-and-optimization)
- [Testing Framework](#testing-framework)
- [Development Guidelines](#development-guidelines)
- [Scalability Architecture](#scalability-architecture)
- [Branding](#branding)

## System Architecture

### Overview
S00K OS follows a monolithic kernel architecture with modular design principles. The system is designed for educational purposes while maintaining security, performance, and reliability standards.

### High-Level Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                    User Applications                      │
├─────────────────────────────────────────────────────────────┤
│                      Shell Interface                        │
├─────────────────────────────────────────────────────────────┤
│         System Services (File System, Memory, I/O)        │
├─────────────────────────────────────────────────────────────┤
│                      Kernel Core                          │
├─────────────────────────────────────────────────────────────┤
│              Hardware Abstraction Layer (HAL)              │
├─────────────────────────────────────────────────────────────┤
│                    Hardware Layer                          │
└─────────────────────────────────────────────────────────────┘
```

### Core Components
- **Kernel Core**: System initialization, process management, system calls
- **Memory Management**: Paging, allocation, protection, and optimization
- **File System**: In-memory hierarchical file system with basic operations
- **I/O System**: Console I/O, device drivers, and interrupt handling
- **Security Subsystem**: Authentication, authorization, and access control
- **Shell Interface**: Command-line interface and user interaction
- **Error Handling**: Comprehensive error reporting and recovery mechanisms

## Boot Process

### Boot Sequence
1. **BIOS/UEFI**: Hardware initialization and boot device selection
2. **Bootloader** (`bootloader.asm`): 
   - Switch to 32-bit protected mode
   - Load kernel from disk
   - Jump to kernel entry point
3. **Kernel Entry** (`kernel.asm`):
   - Set up initial stack and segment registers
   - Call kernel main function
4. **Kernel Initialization** (`kernel.c`):
   - Initialize subsystems in order: Security → Memory → File System → I/O → Shell

### Bootloader Details
```assembly
; bootloader.asm - Primary bootloader
start:
    ; Set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    
    ; Load kernel from disk
    mov ah, 0x02    ; Read sectors
    mov al, 15      ; Number of sectors
    mov ch, 0       ; Cylinder
    mov cl, 2       ; Sector (start after bootloader)
    mov dh, 0       ; Head
    mov bx, 0x1000  ; Load address
    int 0x13
    
    ; Jump to kernel
    jmp 0x1000:0000
```

## Kernel Architecture

### Kernel Entry Point
The kernel main function orchestrates system initialization:

```c
void kernel_main(void) {
    /* Initialize security subsystem first */
    security_init();
    
    /* Initialize memory management */
    init_paging();
    init_memory_management();
    
    /* Initialize file system */
    fs_init();
    
    /* Initialize I/O system */
    init_io();
    
    /* Start shell interface */
    start_shell();
}
```

### System Call Interface
System calls provide the interface between user applications and kernel services:

```c
// System call numbers
#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_MALLOC  4
#define SYS_FREE    5

// System call handler
void sys_call_handler(uint32_t syscall_num, void* args) {
    switch (syscall_num) {
        case SYS_READ:
            handle_sys_read(args);
            break;
        case SYS_WRITE:
            handle_sys_write(args);
            break;
        // ... other system calls
    }
}
```

### Kernel Data Structures

#### Process Control Block (PCB)
```c
typedef struct {
    uint32_t pid;              // Process ID
    uint32_t state;            // Process state (READY, RUNNING, BLOCKED)
    uint32_t priority;         // Process priority
    void* stack_pointer;       // Current stack pointer
    uint32_t stack_base;       // Stack base address
    uint32_t code_segment;     // Code segment base
    uint32_t data_segment;     // Data segment base
    uint32_t registers[8];     // General purpose registers
    security_context_t security; // Security context
} process_control_block_t;
```

#### System State
```c
typedef struct {
    bool is_initialized;
    uint32_t total_memory;
    uint32_t free_memory;
    uint32_t used_memory;
    uint32_t process_count;
    uint32_t uptime_seconds;
    security_state_t security;
    file_system_state_t filesystem;
} system_state_t;
```

## Memory Management

### Memory Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                  Kernel Space (1GB)                        │
│  ┌─────────────────────────────────────────────────────┐  │
│  │              Kernel Code & Data                     │  │
│  ├─────────────────────────────────────────────────────┤  │
│  │              Kernel Stack (per CPU)                 │  │
│  ├─────────────────────────────────────────────────────┤  │
│  │              Page Tables & Directories              │  │
│  ├─────────────────────────────────────────────────────┤  │
│  │              Kernel Heap                              │  │
│  └─────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                  User Space (3GB)                          │
│  ┌─────────────────────────────────────────────────────┐  │
│  │              Application Code                        │  │
│  ├─────────────────────────────────────────────────────┤  │
│  │              Application Data                        │  │
│  ├─────────────────────────────────────────────────────┤  │
│  │              Application Stack                       │  │
│  ├─────────────────────────────────────────────────────┤  │
│  │              Application Heap                        │  │
│  ├─────────────────────────────────────────────────────┤  │
│  │              Shared Libraries                        │  │
│  └─────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### Paging System
The OS uses a two-level paging system:
- **Page Directory**: 1024 entries, each pointing to a page table
- **Page Tables**: 1024 entries each, mapping 4KB pages
- **Page Size**: 4KB standard pages

```c
// Page directory entry structure
typedef struct {
    uint32_t present    : 1;    // Page is present in memory
    uint32_t writable   : 1;    // Page is writable
    uint32_t user       : 1;    // User-mode accessible
    uint32_t accessed   : 1;    // Page has been accessed
    uint32_t dirty      : 1;    // Page has been written to
    uint32_t reserved   : 7;    // Reserved bits
    uint32_t frame      : 20;   // Physical frame address
} page_directory_entry_t;

// Page table entry structure
typedef struct {
    uint32_t present    : 1;
    uint32_t writable   : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t reserved   : 7;
    uint32_t frame      : 20;
} page_table_entry_t;
```

### Memory Allocation
The memory manager uses a hybrid allocation strategy:
- **Buddy System**: For large memory blocks (≥ 4KB)
- **Slab Allocator**: For small, frequently used objects
- **Page Frame Allocator**: For page-sized allocations

```c
// Memory block header
typedef struct memory_block {
    size_t size;                    // Block size including header
    bool is_free;                   // Allocation status
    struct memory_block* next;      // Next block in list
    struct memory_block* prev;      // Previous block in list
    uint32_t magic;                 // Magic number for validation
} memory_block_t;

// Memory allocation algorithms
typedef enum {
    ALLOC_FIRST_FIT,    // First fit algorithm
    ALLOC_BEST_FIT,     // Best fit algorithm
    ALLOC_WORST_FIT,    // Worst fit algorithm
    ALLOC_BUDDY_SYSTEM  // Buddy system algorithm
} allocation_algorithm_t;
```

### Memory Protection
Memory protection features include:
- **Bounds checking**: All memory accesses are validated
- **Access control**: Read/write/execute permissions
- **Memory isolation**: User and kernel space separation
- **Secure allocation**: Zeroing of sensitive data

## File System

### File System Architecture
The file system is an in-memory hierarchical structure with the following components:

```c
// File system superblock
typedef struct {
    uint32_t magic;                 // Filesystem magic number
    uint32_t version;               // Filesystem version
    uint32_t total_blocks;          // Total blocks in filesystem
    uint32_t free_blocks;           // Free blocks available
    uint32_t total_inodes;          // Total inodes
    uint32_t free_inodes;           // Free inodes
    block_t* block_map;             // Block allocation bitmap
    inode_t* inode_table;           // Inode table
} superblock_t;

// Inode structure
typedef struct {
    uint32_t inode_num;             // Inode number
    uint32_t size;                  // File size in bytes
    uint32_t blocks;                // Number of blocks allocated
    uint32_t permissions;           // File permissions
    uint32_t uid;                   // User ID
    uint32_t gid;                   // Group ID
    time_t created;                 // Creation time
    time_t modified;                // Last modification time
    time_t accessed;                // Last access time
    block_ptr_t direct[12];         // Direct block pointers
    block_ptr_t indirect;           // Single indirect pointer
    block_ptr_t double_indirect;    // Double indirect pointer
} inode_t;
```

### Directory Structure
Directories are implemented as special files containing directory entries:

```c
// Directory entry
typedef struct {
    uint32_t inode_num;             // Inode number
    char name[MAX_FILENAME_LENGTH]; // File name
    uint8_t type;                   // File type (regular, directory, etc.)
    uint32_t size;                  // File size
} directory_entry_t;

// Directory operations
typedef struct {
    int (*create)(const char* name, uint32_t permissions);
    int (*remove)(const char* name);
    int (*rename)(const char* old_name, const char* new_name);
    int (*list)(directory_entry_t* entries, uint32_t max_entries);
    int (*find)(const char* name, directory_entry_t* entry);
} directory_operations_t;
```

### File Operations
File operations are implemented through a virtual file system (VFS) layer:

```c
// File operations structure
typedef struct {
    int (*open)(const char* path, int flags, uint32_t mode);
    int (*close)(int fd);
    ssize_t (*read)(int fd, void* buffer, size_t count);
    ssize_t (*write)(int fd, const void* buffer, size_t count);
    off_t (*lseek)(int fd, off_t offset, int whence);
    int (*stat)(const char* path, struct stat* statbuf);
    int (*chmod)(const char* path, uint32_t mode);
} file_operations_t;
```

### File System Limits
- **Maximum file size**: 64KB
- **Maximum filename length**: 64 characters
- **Maximum files per directory**: 128
- **Maximum directory depth**: 16 levels
- **Block size**: 512 bytes

## Input/Output System

### Console I/O
The console I/O system provides basic text input and output functionality:

```c
// Console structure
typedef struct {
    uint16_t* buffer;               // VGA text buffer
    uint16_t cursor_x;              // Cursor X position
    uint16_t cursor_y;              // Cursor Y position
    uint8_t color;                  // Current text color
    uint16_t width;                 // Screen width (characters)
    uint16_t height;                // Screen height (characters)
    bool scrolling_enabled;         // Scrolling enabled flag
} console_t;

// Console operations
typedef struct {
    void (*put_char)(char c);
    void (*put_string)(const char* str);
    char (*get_char)(void);
    void (*clear)(void);
    void (*scroll)(void);
    void (*set_cursor)(uint16_t x, uint16_t y);
} console_operations_t;
```

### VGA Text Mode
The system uses VGA text mode with the following characteristics:
- **Resolution**: 80x25 characters
- **Colors**: 16 foreground colors, 8 background colors
- **Character Set**: IBM PC character set
- **Memory**: Direct memory mapped at 0xB8000

### I/O Ports
Common I/O ports used by the system:
```c
// VGA ports
#define VGA_CRT_CONTROLLER  0x3D4   // CRT controller register
#define VGA_CRT_DATA        0x3D5   // CRT data register
#define VGA_INPUT_STATUS    0x3DA   // Input status register

// Keyboard ports
#define KEYBOARD_DATA       0x60    // Keyboard data port
#define KEYBOARD_STATUS     0x64    // Keyboard status port
#define KEYBOARD_COMMAND    0x64    // Keyboard command port

// PIC ports (Programmable Interrupt Controller)
#define PIC_MASTER_COMMAND  0x20    // Master PIC command
#define PIC_MASTER_DATA     0x21    // Master PIC data
#define PIC_SLAVE_COMMAND   0xA0    // Slave PIC command
#define PIC_SLAVE_DATA      0xA1    // Slave PIC data
```

## Security Architecture

### Security Model
The security system implements a multi-layered defense model:

```c
// Privilege levels
typedef enum {
    PRIVILEGE_GUEST = 0,            // Guest user (minimal access)
    PRIVILEGE_USER = 1,             // Regular user (standard access)
    PRIVILEGE_ADMIN = 2,            // Administrator (elevated access)
    PRIVILEGE_KERNEL = 3            // Kernel (full access)
} privilege_level_t;

// Security context
typedef struct {
    user_t* current_user;           // Currently authenticated user
    privilege_level_t current_level; // Current privilege level
    bool authentication_required;    // Authentication required flag
    security_log_t* security_log;    // Security event log
    uint32_t violation_count;        // Security violation count
} security_context_t;
```

### Authentication System
The authentication system provides user verification and session management:

```c
// User structure
typedef struct {
    char username[MAX_USERNAME_LENGTH]; // Username
    uint8_t password_hash[MAX_PASSWORD_LENGTH]; // Password hash
    privilege_level_t privilege;      // User privilege level
    bool is_active;                   // Account active flag
    uint32_t session_id;              // Current session ID
    time_t last_login;                // Last login time
    uint32_t failed_attempts;         // Failed login attempts
} user_t;

// Authentication operations
typedef struct {
    bool (*authenticate)(const char* username, const char* password);
    bool (*create_user)(const char* username, const char* password, privilege_level_t privilege);
    bool (*remove_user)(const char* username);
    bool (*change_password)(const char* username, const char* new_password);
    user_t* (*get_current_user)(void);
} authentication_operations_t;
```

### Input Validation
Input validation prevents common security vulnerabilities:

```c
// Input validation functions
bool security_validate_input(const char* input, size_t max_length);
bool security_validate_filename(const char* filename);
bool security_validate_path(const char* path);
bool security_validate_command(const char* command);

// Buffer overflow protection
size_t security_safe_strcpy(char* dest, const char* src, size_t dest_size);
size_t security_safe_strcat(char* dest, const char* src, size_t dest_size);
bool security_check_buffer_bounds(const void* buffer, size_t size, const void* ptr);
```

### Memory Protection
Memory protection prevents unauthorized memory access:

```c
// Memory protection flags
typedef enum {
    MEM_PROT_NONE = 0,             // No access
    MEM_PROT_READ = 1,             // Read access
    MEM_PROT_WRITE = 2,            // Write access
    MEM_PROT_EXECUTE = 4,          // Execute access
    MEM_PROT_ALL = 7               // All access
} memory_protection_t;

// Memory region structure
typedef struct {
    void* base_address;            // Base address of region
    size_t size;                   // Size of region
    memory_protection_t protection; // Protection flags
    user_t* owner;                 // Owner user
    bool is_allocated;             // Allocation status
} memory_region_t;
```

### Security Logging
Security events are logged for audit and analysis:

```c
// Security log entry
typedef struct {
    char event_type[32];           // Type of security event
    char description[128];         // Event description
    uint32_t timestamp;            // Event timestamp
    user_t* user;                  // User associated with event
    uint32_t severity;             // Event severity level
} security_log_entry_t;

// Security logging functions
void security_log_event(const char* event_type, const char* description, user_t* user);
void security_log_security_violation(const char* violation_type, const char* details, user_t* user);
void security_display_security_status(void);
```

## Shell and User Interface

### Shell Architecture
The shell provides a command-line interface for user interaction:

```c
// Shell command structure
typedef struct {
    const char* name;              // Command name
    const char* description;       // Command description
    const char* usage;            // Command usage
    int (*handler)(int argc, char* argv[]); // Command handler
    privilege_level_t required_privilege; // Required privilege level
} shell_command_t;

// Shell context
typedef struct {
    char current_directory[MAX_PATH_LENGTH]; // Current directory
    user_t* current_user;          // Current user
    char input_buffer[MAX_INPUT_LENGTH]; // Input buffer
    char* argv[MAX_ARGS];          // Argument vector
    int argc;                      // Argument count
    bool running;                  // Shell running flag
} shell_context_t;
```

### Command Processing
Command processing follows these steps:
1. **Input Reading**: Read user input from console
2. **Parsing**: Parse input into command and arguments
3. **Validation**: Validate command and arguments
4. **Authorization**: Check user privileges
5. **Execution**: Execute command handler
6. **Output**: Display results or error messages

### Built-in Commands
The shell includes the following built-in commands:
- `help`: Display available commands
- `clear`: Clear the screen
- `echo`: Display text
- `ls`: List directory contents
- `cat`: Display file contents
- `touch`: Create a new file
- `rm`: Remove a file
- `mkdir`: Create a directory
- `rmdir`: Remove a directory
- `meminfo`: Display memory information
- `panic`: Test error handling
- `exit`: Exit the shell

## Error Handling

### Error Architecture
The error handling system provides comprehensive error reporting and recovery:

```c
// Error severity levels
typedef enum {
    SEVERITY_INFO,                 // Informational message
    SEVERITY_WARNING,              // Warning message
    SEVERITY_ERROR,                // Error message
    SEVERITY_FATAL                 // Fatal error
} error_severity_t;

// Error information structure
typedef struct {
    int32_t code;                  // Error code
    const char* message;           // Error message
    error_severity_t severity;     // Error severity
    const char* file;              // Source file
    uint32_t line;                 // Source line number
    const char* function;          // Function name
    time_t timestamp;              // Error timestamp
} error_info_t;

// Error codes
typedef enum {
    // General errors
    ERR_SUCCESS = 0,
    ERR_INVALID_PARAMETER = -1,
    ERR_OUT_OF_MEMORY = -2,
    ERR_NOT_IMPLEMENTED = -3,
    
    // I/O errors
    ERR_IO_DEVICE = -10,
    ERR_IO_TIMEOUT = -11,
    ERR_IO_INVALID = -12,
    
    // File system errors
    ERR_FS_NOT_FOUND = -20,
    ERR_FS_EXISTS = -21,
    ERR_FS_FULL = -22,
    ERR_FS_PERMISSION = -23,
    
    // Memory errors
    ERR_MEMORY_ALLOCATION = -30,
    ERR_MEMORY_DEALLOCATION = -31,
    ERR_MEMORY_VIOLATION = -32,
    
    // Security errors
    ERR_SECURITY_VIOLATION = -100,
    ERR_INVALID_INPUT = -101,
    ERR_BUFFER_OVERFLOW = -102,
    ERR_UNAUTHORIZED_ACCESS = -103,
    ERR_AUTHENTICATION_FAILED = -104
} error_code_t;
```

### Error Handling Functions
```c
// Error handling functions
void handle_error(error_code_t code, const char* file, uint32_t line, const char* function);
const char* get_error_message(error_code_t code);
error_severity_t get_error_severity(error_code_t code);
void log_error(const error_info_t* error_info);
void panic(const char* message);
```

### Error Recovery
The system implements several error recovery mechanisms:
- **Graceful Degradation**: Continue operation with reduced functionality
- **Retry Mechanisms**: Automatic retry for transient errors
- **Fallback Systems**: Alternative implementations for critical functions
- **Safe Mode**: Reduced functionality mode for system recovery

## Performance and Optimization

### Performance Architecture
The performance system provides monitoring and optimization capabilities:

```c
// Performance metrics
typedef struct {
    uint32_t cpu_usage;            // CPU usage percentage
    uint32_t memory_usage;         // Memory usage percentage
    uint32_t disk_io_operations;   // Disk I/O operations per second
    uint32_t context_switches;     // Context switches per second
    uint32_t interrupt_count;      // Interrupt count per second
    uint32_t system_calls;         // System calls per second
} performance_metrics_t;

// Performance profiling
typedef struct {
    const char* function_name;     // Function name
    uint32_t call_count;           // Number of calls
    uint64_t total_time;           // Total execution time
    uint64_t min_time;             // Minimum execution time
    uint64_t max_time;             // Maximum execution time
    uint64_t average_time;         // Average execution time
} function_profile_t;
```

### Optimization Strategies
- **Memory Optimization**: Buddy system allocation, memory pooling, garbage collection
- **I/O Optimization**: Buffering, caching, asynchronous operations
- **CPU Optimization**: Efficient algorithms, loop unrolling, inline functions
- **Compiler Optimization**: Optimization flags, link-time optimization

### Performance Monitoring
Performance monitoring includes:
- **Real-time Metrics**: CPU, memory, I/O usage
- **Function Profiling**: Execution time and call frequency
- **System Call Tracing**: System call frequency and duration
- **Memory Profiling**: Allocation patterns and leak detection

## Scalability Architecture

### Components
- Scheduler: Cooperative round-robin with per-CPU run queue
- Load Balancer: Moves READY tasks from overloaded to underloaded CPUs
- Thread Model: Lightweight runnable units with priorities and quotas

### Integration
```
┌────────────┬──────────────────────────────────────────────┐
│   CPUs     │                 Run Queues                   │
├────────────┼──────────────────────────────────────────────┤
│ CPU0       │ T0, T4, T8                                   │
│ CPU1       │ T1, T5, T9                                   │
│ CPU2       │ T2, T6, T10                                  │
│ CPU3       │ T3, T7, T11                                  │
└────────────┴──────────────────────────────────────────────┘
         ↓ periodic
   Load Balancer moves READY tasks to keep queues even
```

### Usage
- Initialize with `init_scheduler(n)`
- Create tasks with `create_thread(entry, arg, priority)`
- Drive scheduling with `schedule_process()` and `yield()`
- Rebalance with `load_balance()`

## Branding

### Identity Constants
- `OS_NAME`, `OS_VERSION`, `OS_CODENAME` in `src/brand.h`

### Assets
- Logo: `assets/branding/s00k-os-logo.svg`
- Icons: `assets/branding/s00k-os-icon.svg`
- Wallpapers: `assets/branding/s00k-os-wallpaper-dark.svg`, `assets/branding/s00k-os-wallpaper-light.svg`

### Boot Experience
- Text-mode ASCII banner and spinner integrated in `kernel.c`

## Testing Framework

### Test Architecture
The testing framework provides comprehensive testing capabilities:

```c
// Test case structure
typedef struct {
    const char* name;              // Test name
    const char* description;       // Test description
    void (*setup)(void);           // Setup function
    void (*teardown)(void);        // Teardown function
    void (*test_function)(void);   // Test function
    bool expected_result;          // Expected result
    uint32_t timeout_ms;           // Test timeout
} test_case_t;

// Test suite structure
typedef struct {
    const char* name;              // Suite name
    test_case_t* test_cases;       // Array of test cases
    uint32_t test_count;           // Number of test cases
    uint32_t passed_count;         // Passed test count
    uint32_t failed_count;         // Failed test count
    void (*suite_setup)(void);     // Suite setup
    void (*suite_teardown)(void);  // Suite teardown
} test_suite_t;
```

### Test Categories
- **Unit Tests**: Individual function testing
- **Integration Tests**: Component interaction testing
- **System Tests**: End-to-end system testing
- **Performance Tests**: Performance and benchmark testing
- **Security Tests**: Security vulnerability testing
- **Regression Tests**: Prevent regression testing

### Test Execution
Test execution follows these steps:
1. **Setup**: Initialize test environment
2. **Execution**: Run test cases
3. **Verification**: Verify test results
4. **Cleanup**: Clean up test environment
5. **Reporting**: Generate test reports

## Development Guidelines

### Code Standards
- **Naming Conventions**: Use descriptive names with appropriate prefixes
- **Code Formatting**: Consistent indentation and spacing
- **Comments**: Comprehensive documentation for all functions
- **Error Handling**: Proper error checking and handling
- **Memory Management**: Proper allocation and deallocation

### Security Guidelines
- **Input Validation**: Validate all user inputs
- **Buffer Protection**: Prevent buffer overflows
- **Access Control**: Implement proper access controls
- **Secure Coding**: Follow secure coding practices
- **Code Review**: Regular security code reviews

### Performance Guidelines
- **Efficiency**: Use efficient algorithms and data structures
- **Resource Usage**: Minimize resource consumption
- **Scalability**: Design for scalability
- **Profiling**: Regular performance profiling
- **Optimization**: Optimize critical paths

### Testing Guidelines
- **Test Coverage**: Maintain high test coverage
- **Test Automation**: Automate testing processes
- **Regression Testing**: Regular regression testing
- **Performance Testing**: Regular performance testing
- **Security Testing**: Regular security testing

---

**Version**: 1.0.0  
**Last Updated**: January 2024  
**Maintainers**: S00K OS Development Team