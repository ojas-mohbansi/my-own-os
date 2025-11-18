/* io.c - Basic I/O operations for keyboard input and screen output
   Provides polled keyboard input via the 8042 controller and writes to the
   VGA text buffer. Includes a timeout-capable reader and safe printing helpers. */

#include <stdint.h>
#include <stddef.h>
#include "error_codes.h"

/* External error handling function */
extern void handle_error(int32_t error_code, const char* function, const char* file, uint32_t line);

/* x86 I/O port operations */
static inline uint8_t inb(uint16_t port) {
    /* Read a byte from an I/O port */
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    /* Write a byte to an I/O port */
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

/* VGA I/O ports */
#define VGA_CTRL_REGISTER     0x3D4
#define VGA_DATA_REGISTER     0x3D5

/* Check if keyboard has data available */
static int keyboard_data_available(void) {
    /* STATUS bit0 indicates whether data is available */
    return (inb(KEYBOARD_STATUS_PORT) & 0x01) != 0;
}

/* Read keyboard status with error checking */
static int32_t read_keyboard_status(uint8_t* status) {
    /* Read status register and return ERR codes on failure */
    if (!status) {
        return ERR_NULL_POINTER;
    }
    
    *status = inb(KEYBOARD_STATUS_PORT);
    return ERR_SUCCESS;
}

/* Read keyboard data with error checking */
static int32_t read_keyboard_data(uint8_t* data) {
    /* Block until a data byte is present or report ERR_IO_DEVICE_ERROR */
    if (!data) {
        return ERR_NULL_POINTER;
    }
    
    /* Check if data is available */
    uint8_t status;
    int32_t result = read_keyboard_status(&status);
    if (result != ERR_SUCCESS) {
        return result;
    }
    
    if (!(status & 0x01)) {
        return ERR_IO_DEVICE_ERROR;  /* No data available */
    }
    
    *data = inb(KEYBOARD_DATA_PORT);
    return ERR_SUCCESS;
}

/* Read a character from keyboard with timeout and error handling */
char read_char_timeout(uint32_t timeout_ms, int32_t* error_code) {
    /* Poll keyboard with a coarse-grained timeout; maps basic scancodes to ASCII.
       Returns 0 on error/timeout with error_code set appropriately. */
    uint32_t timeout_counter = timeout_ms * 1000; /* Convert to iterations */
    uint8_t scancode;
    int32_t result;
    
    if (error_code) {
        *error_code = ERR_SUCCESS;
    }
    
    /* Wait for keyboard data with timeout */
    while (timeout_counter > 0) {
        result = read_keyboard_data(&scancode);
        if (result == ERR_SUCCESS) {
            break;  /* Data available */
        } else if (result != ERR_IO_DEVICE_ERROR) {
            /* Real error occurred */
            if (error_code) {
                *error_code = result;
            }
            return 0;
        }
        
        /* Decrement timeout counter */
        timeout_counter--;
    }
    
    /* Check for timeout */
    if (timeout_counter == 0) {
        if (error_code) {
            *error_code = ERR_IO_TIMEOUT;
        }
        return 0;
    }
    
    /* Minimal scancode->ASCII conversion for common keys */
    switch (scancode & 0x7F) {
        case 0x1C: return '\n';  /* Enter */
        case 0x0E: return '\b';  /* Backspace */
        case 0x0F: return '\t';  /* Tab */
        case 0x29: return ' ';   /* Space */
        case 0x02: return (scancode & 0x80) ? 0 : '1';  /* 1 */
        case 0x03: return (scancode & 0x80) ? 0 : '2';  /* 2 */
        case 0x04: return (scancode & 0x80) ? 0 : '3';  /* 3 */
        case 0x05: return (scancode & 0x80) ? 0 : '4';  /* 4 */
        case 0x06: return (scancode & 0x80) ? 0 : '5';  /* 5 */
        case 0x07: return (scancode & 0x80) ? 0 : '6';  /* 6 */
        case 0x08: return (scancode & 0x80) ? 0 : '7';  /* 7 */
        case 0x09: return (scancode & 0x80) ? 0 : '8';  /* 8 */
        case 0x0A: return (scancode & 0x80) ? 0 : '9';  /* 9 */
        case 0x0B: return (scancode & 0x80) ? 0 : '0';  /* 0 */
        case 0x10: return (scancode & 0x80) ? 0 : 'q';  /* Q */
        case 0x11: return (scancode & 0x80) ? 0 : 'w';  /* W */
        case 0x12: return (scancode & 0x80) ? 0 : 'e';  /* E */
        case 0x13: return (scancode & 0x80) ? 0 : 'r';  /* R */
        case 0x14: return (scancode & 0x80) ? 0 : 't';  /* T */
        case 0x15: return (scancode & 0x80) ? 0 : 'y';  /* Y */
        case 0x16: return (scancode & 0x80) ? 0 : 'u';  /* U */
        case 0x17: return (scancode & 0x80) ? 0 : 'i';  /* I */
        case 0x18: return (scancode & 0x80) ? 0 : 'o';  /* O */
        case 0x19: return (scancode & 0x80) ? 0 : 'p';  /* P */
        case 0x1E: return (scancode & 0x80) ? 0 : 'a';  /* A */
        case 0x1F: return (scancode & 0x80) ? 0 : 's';  /* S */
        case 0x20: return (scancode & 0x80) ? 0 : 'd';  /* D */
        case 0x21: return (scancode & 0x80) ? 0 : 'f';  /* F */
        case 0x22: return (scancode & 0x80) ? 0 : 'g';  /* G */
        case 0x23: return (scancode & 0x80) ? 0 : 'h';  /* H */
        case 0x24: return (scancode & 0x80) ? 0 : 'j';  /* J */
        case 0x25: return (scancode & 0x80) ? 0 : 'k';  /* K */
        case 0x26: return (scancode & 0x80) ? 0 : 'l';  /* L */
        case 0x2C: return (scancode & 0x80) ? 0 : 'z';  /* Z */
        case 0x2D: return (scancode & 0x80) ? 0 : 'x';  /* X */
        case 0x2E: return (scancode & 0x80) ? 0 : 'c';  /* C */
        case 0x2F: return (scancode & 0x80) ? 0 : 'v';  /* V */
        case 0x30: return (scancode & 0x80) ? 0 : 'b';  /* B */
        case 0x31: return (scancode & 0x80) ? 0 : 'n';  /* N */
        case 0x32: return (scancode & 0x80) ? 0 : 'm';  /* M */
        default: return 0;  /* Ignore unknown keys or key release */
    }
}

/* Original read_char function (blocking, no timeout) */
char read_char(void) {
    return read_char_timeout(0, NULL);  /* 0 timeout = infinite wait */
}

/* Print a single character to screen using VGA text buffer */
void print_char(char c) {
    /* Write a character to VGA text buffer at 0xB8000 with scrolling/backspace. */
    /* Use the existing VGA buffer at 0xB8000 */
    static uint16_t* vga_buffer = (uint16_t*)0xB8000;
    static size_t vga_row = 0;
    static size_t vga_col = 0;
    static const size_t VGA_WIDTH = 80;
    static const size_t VGA_HEIGHT = 25;
    static const uint8_t VGA_COLOR_WHITE_ON_BLACK = 0x0F;
    
    /* Validate VGA buffer */
    if (!vga_buffer) {
        return; /* Cannot print without VGA buffer */
    }
    
    /* Handle special characters */
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else if (c == '\b') {  /* Backspace */
        if (vga_col > 0) {
            vga_col--;
            vga_buffer[vga_row * VGA_WIDTH + vga_col] = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
        }
    } else {
        /* Print regular character */
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = (VGA_COLOR_WHITE_ON_BLACK << 8) | (uint8_t)c;
        vga_col++;
    }
    
    /* Handle wrapping and scrolling */
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }
    if (vga_row >= VGA_HEIGHT) {
        /* Scroll screen up */
        for (size_t y = 0; y < VGA_HEIGHT - 1; ++y) {
            for (size_t x = 0; x < VGA_WIDTH; ++x) {
                vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
            }
        }
        /* Clear bottom row */
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
        }
        vga_row = VGA_HEIGHT - 1;
    }
}

/* Print character with error checking */
int32_t print_char_safe(char c) {
    /* Validate character */
    if (c < 0 || c > 127) {
        return ERR_INVALID_PARAMETER;
    }
    
    /* Print the character */
    print_char(c);
    return ERR_SUCCESS;
}

/* Print string with error checking */
int32_t print_string_safe(const char* str) {
    if (!str) {
        return ERR_NULL_POINTER;
    }
    
    /* Print each character */
    while (*str) {
        int32_t result = print_char_safe(*str);
        if (result != ERR_SUCCESS) {
            return result;
        }
        str++;
    }
    
    return ERR_SUCCESS;
}

/* Clear the screen */
void clear_screen(void) {
    /* Fill VGA buffer with spaces to visually clear the screen */
    uint16_t* vga_buffer = (uint16_t*)0xB8000;
    const size_t VGA_WIDTH = 80;
    const size_t VGA_HEIGHT = 25;
    const uint8_t VGA_COLOR_WHITE_ON_BLACK = 0x0F;
    
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga_buffer[i] = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
    }
}