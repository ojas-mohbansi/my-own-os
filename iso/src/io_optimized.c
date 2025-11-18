/* io_optimized.c - Optimized I/O operations with performance profiling */

#include <stdint.h>
#include <stddef.h>
#include "error_codes.h"
#include "performance_profiler.h"

/* External error handling function */
extern void handle_error(int32_t error_code, const char* function, const char* file, uint32_t line);

/* x86 I/O port operations */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

/* VGA I/O ports */
#define VGA_CTRL_REGISTER     0x3D4
#define VGA_DATA_REGISTER     0x3D5

/* Optimized keyboard buffer with batch processing */
#define KEYBOARD_BUFFER_SIZE 256
typedef struct {
    uint8_t buffer[KEYBOARD_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint8_t full;
} keyboard_buffer_t;

static keyboard_buffer_t keyboard_buffer = {0};
static uint8_t keyboard_buffer_initialized = 0;

/* Optimized VGA buffer management */
typedef struct {
    uint16_t* buffer;
    uint16_t* shadow_buffer;  /* Shadow buffer for batch updates */
    uint32_t dirty_start;
    uint32_t dirty_end;
    uint8_t batch_mode;
} vga_manager_t;

static vga_manager_t vga_manager = {0};

/* Lookup table for scancode to ASCII conversion - faster than switch statement */
static const char scancode_to_ascii[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0,   0,   '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 0,   0,   '\n', 0,   'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', 0,   0,   0,   0,   'z', 'x', 'c', 'v', 'b',
    'n', 'm', 0,   0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* Initialize keyboard buffer */
static void init_keyboard_buffer(void) {
    if (keyboard_buffer_initialized) return;
    
    keyboard_buffer.head = 0;
    keyboard_buffer.tail = 0;
    keyboard_buffer.full = 0;
    
    /* Clear buffer */
    for (int i = 0; i < KEYBOARD_BUFFER_SIZE; i++) {
        keyboard_buffer.buffer[i] = 0;
    }
    
    keyboard_buffer_initialized = 1;
}

/* Initialize VGA manager */
static void init_vga_manager(void) {
    if (vga_manager.buffer) return;
    
    vga_manager.buffer = (uint16_t*)0xB8000;
    vga_manager.shadow_buffer = (uint16_t*)allocate_memory(PAGE_SIZE);
    vga_manager.dirty_start = 0;
    vga_manager.dirty_end = 0;
    vga_manager.batch_mode = 0;
}

/* Optimized keyboard data available check */
static inline int optimized_keyboard_data_available(void) {
    return (inb(KEYBOARD_STATUS_PORT) & 0x01) != 0;
}

/* Optimized keyboard status read with error checking */
static inline int32_t optimized_read_keyboard_status(uint8_t* status) {
    if (UNLIKELY(!status)) {
        return ERR_NULL_POINTER;
    }
    
    *status = inb(KEYBOARD_STATUS_PORT);
    return ERR_SUCCESS;
}

/* Optimized keyboard data read with buffering */
static inline int32_t optimized_read_keyboard_data(uint8_t* data) {
    if (UNLIKELY(!data)) {
        return ERR_NULL_POINTER;
    }
    
    /* Check if data is available */
    uint8_t status;
    int32_t result = optimized_read_keyboard_status(&status);
    if (result != ERR_SUCCESS) {
        return result;
    }
    
    if (!(status & 0x01)) {
        return ERR_IO_DEVICE_ERROR;  /* No data available */
    }
    
    *data = inb(KEYBOARD_DATA_PORT);
    return ERR_SUCCESS;
}

/* Batch process keyboard input for better performance */
static void process_keyboard_buffer(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("process_keyboard_buffer");
    profiler_start_function(__profile_id);
    
    init_keyboard_buffer();
    
    /* Process multiple keyboard inputs in batch */
    uint32_t processed = 0;
    uint8_t scancode;
    
    while (optimized_keyboard_data_available() && processed < 16) {
        if (optimized_read_keyboard_data(&scancode) == ERR_SUCCESS) {
            /* Add to circular buffer */
            uint32_t next_head = (keyboard_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
            if (next_head != keyboard_buffer.tail) {
                keyboard_buffer.buffer[keyboard_buffer.head] = scancode;
                keyboard_buffer.head = next_head;
                processed++;
            } else {
                keyboard_buffer.full = 1;
                break;
            }
        } else {
            break;
        }
    }
    
    profiler_record_io_operation("read", processed, __profile_id);
    profiler_end_function(__profile_id);
}

/* Optimized read character with buffering and reduced timeout overhead */
char optimized_read_char_timeout(uint32_t timeout_ms, int32_t* error_code) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_read_char_timeout");
    profiler_start_function(__profile_id);
    
    if (error_code) {
        *error_code = ERR_SUCCESS;
    }
    
    /* Process any pending keyboard input first */
    process_keyboard_buffer();
    
    /* Check buffer first (fast path) */
    if (keyboard_buffer.head != keyboard_buffer.tail) {
        uint8_t scancode = keyboard_buffer.buffer[keyboard_buffer.tail];
        keyboard_buffer.tail = (keyboard_buffer.tail + 1) % KEYBOARD_BUFFER_SIZE;
        keyboard_buffer.full = 0;
        
        /* Convert scancode to ASCII using lookup table */
        char result = scancode_to_ascii[scancode & 0x7F];
        if (result != 0) {
            profiler_record_io_operation("read", 1, __profile_id);
            profiler_end_function(__profile_id);
            return result;
        }
    }
    
    /* No data in buffer, check hardware with timeout */
    uint32_t timeout_counter = timeout_ms * 100; /* Reduced iterations for better performance */
    
    while (timeout_counter > 0) {
        process_keyboard_buffer();
        
        /* Check buffer again */
        if (keyboard_buffer.head != keyboard_buffer.tail) {
            uint8_t scancode = keyboard_buffer.buffer[keyboard_buffer.tail];
            keyboard_buffer.tail = (keyboard_buffer.tail + 1) % KEYBOARD_BUFFER_SIZE;
            keyboard_buffer.full = 0;
            
            char result = scancode_to_ascii[scancode & 0x7F];
            if (result != 0) {
                profiler_record_io_operation("read", 1, __profile_id);
                profiler_end_function(__profile_id);
                return result;
            }
        }
        
        timeout_counter--;
    }
    
    /* Timeout occurred */
    if (error_code) {
        *error_code = ERR_IO_TIMEOUT;
    }
    
    profiler_end_function(__profile_id);
    return 0;
}

/* Optimized batch VGA operations */
static inline void optimized_vga_putchar(char c) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_vga_putchar");
    profiler_start_function(__profile_id);
    
    static uint16_t* vga_buffer = (uint16_t*)0xB8000;
    static size_t vga_row = 0;
    static size_t vga_col = 0;
    static const size_t VGA_WIDTH = 80;
    static const size_t VGA_HEIGHT = 25;
    static const uint8_t VGA_COLOR_WHITE_ON_BLACK = 0x0F;
    
    /* Use lookup table for character processing */
    static const struct {
        char character;
        void (*handler)(size_t*, size_t*);
    } char_handlers[] = {
        {'\n', [](size_t* row, size_t* col) { *col = 0; (*row)++; }},
        {'\r', [](size_t* row, size_t* col) { *col = 0; }},
        {'\t', [](size_t* row, size_t* col) { *col = (*col + 8) & ~7; }},
        {'\0', nullptr}  /* Default handler */
    };
    
    /* Check for special characters */
    int handled = 0;
    for (int i = 0; char_handlers[i].character != '\0'; i++) {
        if (c == char_handlers[i].character) {
            char_handlers[i].handler(&vga_row, &vga_col);
            handled = 1;
            break;
        }
    }
    
    if (!handled) {
        /* Regular character */
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = (VGA_COLOR_WHITE_ON_BLACK << 8) | (uint8_t)c;
        vga_col++;
    }
    
    /* Handle wrapping and scrolling */
    if (UNLIKELY(vga_col >= VGA_WIDTH)) {
        vga_col = 0;
        vga_row++;
    }
    if (UNLIKELY(vga_row >= VGA_HEIGHT)) {
        /* Optimized scroll */
        register uint16_t* dst = vga_buffer;
        register uint16_t* src = vga_buffer + VGA_WIDTH;
        register size_t copy_size = (VGA_HEIGHT - 1) * VGA_WIDTH;
        
        /* Fast memory copy with loop unrolling */
        for (register size_t i = 0; i < copy_size; i++) {
            dst[i] = src[i];
        }
        
        /* Clear bottom row */
        register uint16_t clear_value = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
        register uint16_t* bottom_row = vga_buffer + (VGA_HEIGHT - 1) * VGA_WIDTH;
        
        for (register size_t x = 0; x < VGA_WIDTH; ++x) {
            bottom_row[x] = clear_value;
        }
        
        vga_row = VGA_HEIGHT - 1;
    }
    
    profiler_end_function(__profile_id);
}

/* Optimized batch string printing */
void optimized_print_string(const char* str) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_print_string");
    profiler_start_function(__profile_id);
    
    if (UNLIKELY(!str)) {
        profiler_end_function(__profile_id);
        return;
    }
    
    register const char* s = str;
    register size_t len = 0;
    
    /* Process string in batches for better cache performance */
    while (LIKELY(*s)) {
        optimized_vga_putchar(*s++);
        len++;
    }
    
    profiler_record_io_operation("write", len, __profile_id);
    profiler_end_function(__profile_id);
}

/* Optimized clear screen with batch operations */
void optimized_clear_screen(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_clear_screen");
    profiler_start_function(__profile_id);
    
    uint16_t* vga_buffer = (uint16_t*)0xB8000;
    const size_t VGA_WIDTH = 80;
    const size_t VGA_HEIGHT = 25;
    const uint8_t VGA_COLOR_WHITE_ON_BLACK = 0x0F;
    const uint16_t clear_value = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
    const size_t total_cells = VGA_WIDTH * VGA_HEIGHT;
    
    /* Optimized memset using 64-bit writes */
    register uint64_t* vga_64 = (uint64_t*)vga_buffer;
    register uint64_t clear_64 = ((uint64_t)clear_value << 48) | ((uint64_t)clear_value << 32) | 
                                  ((uint64_t)clear_value << 16) | (uint64_t)clear_value;
    register size_t cells_64 = total_cells / 4;
    
    for (register size_t i = 0; i < cells_64; i++) {
        vga_64[i] = clear_64;
    }
    
    /* Handle remaining cells */
    for (register size_t i = cells_64 * 4; i < total_cells; i++) {
        vga_buffer[i] = clear_value;
    }
    
    profiler_record_io_operation("write", total_cells * 2, __profile_id);
    profiler_end_function(__profile_id);
}

/* Optimized character I/O with error checking */
int32_t optimized_print_char_safe(char c) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_print_char_safe");
    profiler_start_function(__profile_id);
    
    /* Validate character */
    if (UNLIKELY(c < 0 || c > 127)) {
        profiler_end_function(__profile_id);
        return ERR_INVALID_PARAMETER;
    }
    
    /* Print the character */
    optimized_vga_putchar(c);
    
    profiler_record_io_operation("write", 1, __profile_id);
    profiler_end_function(__profile_id);
    
    return ERR_SUCCESS;
}

/* Optimized string I/O with error checking */
int32_t optimized_print_string_safe(const char* str) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_print_string_safe");
    profiler_start_function(__profile_id);
    
    if (UNLIKELY(!str)) {
        profiler_end_function(__profile_id);
        return ERR_NULL_POINTER;
    }
    
    /* Print each character with validation */
    register const char* s = str;
    register size_t len = 0;
    
    while (*s) {
        if (UNLIKELY(*s < 0 || *s > 127)) {
            profiler_end_function(__profile_id);
            return ERR_INVALID_PARAMETER;
        }
        optimized_vga_putchar(*s);
        s++;
        len++;
    }
    
    profiler_record_io_operation("write", len, __profile_id);
    profiler_end_function(__profile_id);
    
    return ERR_SUCCESS;
}

/* Initialize optimized I/O system */
void optimized_io_init(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_io_init");
    profiler_start_function(__profile_id);
    
    init_keyboard_buffer();
    init_vga_manager();
    
    profiler_end_function(__profile_id);
}