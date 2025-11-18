/* security.c - Security implementation for the OS
   Implements a minimal authentication/authorization system and event logging.
   NOTE: Password hashing is intentionally simple and for demonstration only. */

#include "security.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

/* Simple hash function for passwords (not cryptographically secure, for demonstration) */
static uint32_t simple_hash(const char* str) {
    /* djb2-style rolling hash; not cryptographically secure. */
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    
    return hash;
}

/* Global security state */
static security_state_t security_state = {
    /* Global security state; toggles for kernel/memory protections and current user. */
    .is_initialized = false,
    .current_user = NULL,
    .security_events_logged = 0,
    .security_violations_logged = 0,
    .kernel_protection_enabled = false,
    .memory_protection_enabled = false,
    .authentication_enabled = true
};

/* User database (simplified) */
static user_t user_database[16];
static uint32_t user_count = 0;

/* Security log (circular buffer) */
#define SECURITY_LOG_SIZE 64
typedef struct {
    char event_type[32];
    char description[128];
    uint32_t timestamp;
    user_t* user;
} security_log_entry_t;

static security_log_entry_t security_log[SECURITY_LOG_SIZE]; /* circular buffer */
static uint32_t log_index = 0;
static uint32_t log_count = 0;

/* Forward declarations */
static void add_security_log_entry(const char* event_type, const char* description, user_t* user);
static user_t* find_user_by_username(const char* username);
static bool validate_user_credentials(const user_t* user, const char* password);

/* Initialize security subsystem */
bool security_init(void) {
    /* Initialize users, log buffers, and defaults; create admin/guest accounts. */
    if (security_state.is_initialized) {
        return true; /* Already initialized */
    }
    
    /* Clear user database */
    for (uint32_t i = 0; i < 16; i++) {
        memset(&user_database[i], 0, sizeof(user_t));
        user_database[i].is_active = false;
    }
    user_count = 0;
    
    /* Clear security log */
    for (uint32_t i = 0; i < SECURITY_LOG_SIZE; i++) {
        memset(&security_log[i], 0, sizeof(security_log_entry_t));
    }
    log_index = 0;
    log_count = 0;
    
    /* Create default admin user */
    security_create_user("admin", "admin123", PRIVILEGE_ADMIN);
    
    /* Create default guest user */
    security_create_user("guest", "guest", PRIVILEGE_GUEST);
    
    security_state.is_initialized = true;
    security_state.security_events_logged = 0;
    security_state.security_violations_logged = 0;
    
    add_security_log_entry("SECURITY_INIT", "Security subsystem initialized", NULL);
    
    return true;
}

/* Initialize memory protection */
bool security_init_memory_protection(void) {
    if (!security_state.is_initialized) {
        return false;
    }
    
    security_state.memory_protection_enabled = true;
    add_security_log_entry("MEMORY_PROTECTION_INIT", "Memory protection initialized", NULL);
    
    return true;
}

/* Initialize kernel protection */
bool security_init_kernel_protection(void) {
    if (!security_state.is_initialized) {
        return false;
    }
    
    security_state.kernel_protection_enabled = true;
    add_security_log_entry("KERNEL_PROTECTION_INIT", "Kernel protection initialized", NULL);
    
    return true;
}

/* Get security state */
security_state_t* security_get_state(void) {
    return &security_state;
}

/* Get current user */
user_t* security_get_current_user(void) {
    return security_state.current_user;
}

/* Create user */
bool security_create_user(const char* username, const char* password, privilege_level_t privilege) {
    if (!security_state.is_initialized) {
        return false;
    }
    
    if (!username || !password) {
        return false;
    }
    
    if (strlen(username) >= MAX_USERNAME_LENGTH || strlen(password) >= MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    if (user_count >= 16) {
        return false;
    }
    
    /* Check if user already exists */
    if (find_user_by_username(username) != NULL) {
        return false;
    }
    
    /* Create new user */
    user_t* new_user = &user_database[user_count];
    
    /* Copy username */
    char* dest = new_user->username;
    const char* src = username;
    while (*src && (dest - new_user->username) < MAX_USERNAME_LENGTH - 1) {
        *dest++ = *src++;
    }
    *dest = '\0';
    
    /* Hash password */
    uint32_t hash = simple_hash(password);
    
    /* Store password hash (simplified) */
    uint8_t* hash_ptr = (uint8_t*)&hash;
    for (int i = 0; i < sizeof(uint32_t) && i < MAX_PASSWORD_LENGTH; i++) {
        new_user->password_hash[i] = hash_ptr[i];
    }
    
    new_user->privilege = privilege;
    new_user->is_active = true;
    new_user->session_id = 0;
    
    user_count++;
    
    add_security_log_entry("USER_CREATED", username, NULL);
    
    return true;
}

/* Authenticate user */
bool security_authenticate_user(const char* username, const char* password) {
    if (!security_state.is_initialized) {
        return false;
    }
    
    if (!username || !password) {
        return false;
    }
    
    user_t* user = find_user_by_username(username);
    if (!user || !user->is_active) {
        add_security_log_entry("AUTH_FAILED", username, NULL);
        return false;
    }
    
    if (validate_user_credentials(user, password)) {
        /* Generate session ID (simplified) */
        static uint32_t session_counter = 1;
        user->session_id = session_counter++;
        
        security_state.current_user = user;
        add_security_log_entry("USER_LOGIN", username, user);
        return true;
    } else {
        add_security_log_entry("AUTH_FAILED", username, NULL);
        return false;
    }
}

/* Check user permission */
bool security_check_permission(user_t* user, privilege_level_t required_privilege) {
    if (!user || !user->is_active) {
        return false;
    }
    
    return user->privilege >= required_privilege;
}

/* Logout user */
bool security_logout_user(user_t* user) {
    if (!user || !security_state.is_initialized) {
        return false;
    }
    
    if (security_state.current_user == user) {
        security_state.current_user = NULL;
    }
    
    user->session_id = 0;
    add_security_log_entry("USER_LOGOUT", user->username, user);
    
    return true;
}

/* Validate input */
bool security_validate_input(const char* input, size_t max_length) {
    if (!input) {
        return false;
    }
    
    size_t length = 0;
    const char* ptr = input;
    
    while (*ptr && length < max_length) {
        char c = *ptr;
        
        /* Check for printable characters only */
        if (c < 32 || c > 126) {
            if (c != '\n' && c != '\r' && c != '\t') {
                return false;
            }
        }
        
        ptr++;
        length++;
    }
    
    /* Check if string is properly null-terminated */
    if (*ptr != '\0') {
        return false;
    }
    
    return true;
}

/* Validate filename */
bool security_validate_filename(const char* filename) {
    if (!filename) {
        return false;
    }
    
    size_t length = 0;
    const char* ptr = filename;
    
    while (*ptr && length < MAX_FILE_NAME_LENGTH) {
        char c = *ptr;
        
        /* Allow alphanumeric, dot, underscore, hyphen */
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')) {
            return false;
        }
        
        ptr++;
        length++;
    }
    
    /* Check for null termination */
    if (*ptr != '\0') {
        return false;
    }
    
    return length > 0; /* Non-empty filename */
}

/* Validate path */
bool security_validate_path(const char* path) {
    if (!path) {
        return false;
    }
    
    size_t length = 0;
    const char* ptr = path;
    
    while (*ptr && length < MAX_PATH_LENGTH) {
        char c = *ptr;
        
        /* Allow alphanumeric, dot, underscore, hyphen, forward slash */
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-' || c == '/')) {
            return false;
        }
        
        ptr++;
        length++;
    }
    
    /* Check for null termination */
    if (*ptr != '\0') {
        return false;
    }
    
    return length > 0; /* Non-empty path */
}

/* Validate command */
bool security_validate_command(const char* command) {
    if (!command) {
        return false;
    }
    
    size_t length = 0;
    const char* ptr = command;
    
    while (*ptr && length < MAX_COMMAND_LENGTH) {
        char c = *ptr;
        
        /* Allow alphanumeric, space, hyphen, underscore */
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_')) {
            return false;
        }
        
        ptr++;
        length++;
    }
    
    /* Check for null termination */
    if (*ptr != '\0') {
        return false;
    }
    
    return length > 0; /* Non-empty command */
}

/* Safe string copy */
size_t security_safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return 0;
    }
    
    size_t i = 0;
    while (i < dest_size - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    
    dest[i] = '\0';
    return i;
}

/* Safe string concatenate */
size_t security_safe_strcat(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return 0;
    }
    
    size_t dest_len = 0;
    while (dest_len < dest_size && dest[dest_len] != '\0') {
        dest_len++;
    }
    
    if (dest_len >= dest_size) {
        return 0; /* No space available */
    }
    
    size_t i = 0;
    while (dest_len + i < dest_size - 1 && src[i] != '\0') {
        dest[dest_len + i] = src[i];
        i++;
    }
    
    dest[dest_len + i] = '\0';
    return dest_len + i;
}

/* Check buffer bounds */
bool security_check_buffer_bounds(const void* buffer, size_t size, const void* ptr) {
    if (!buffer || !ptr) {
        return false;
    }
    
    const uint8_t* buffer_start = (const uint8_t*)buffer;
    const uint8_t* buffer_end = buffer_start + size;
    const uint8_t* ptr_byte = (const uint8_t*)ptr;
    
    return ptr_byte >= buffer_start && ptr_byte < buffer_end;
}

/* Check memory access */
bool security_check_memory_access(const void* address, size_t size, memory_protection_t access_type) {
    /* Simplified implementation - in real OS this would check page tables */
    if (!address || size == 0) {
        return false;
    }
    
    /* Check for overflow in address calculation */
    uint32_t start_addr = (uint32_t)address;
    uint32_t end_addr = start_addr + size;
    
    if (end_addr < start_addr) {
        return false; /* Overflow detected */
    }
    
    /* Check against kernel memory boundaries */
    if (start_addr < 0x100000) { /* Below 1MB */
        return false; /* Kernel space */
    }
    
    return true;
}

/* Validate kernel pointer */
bool security_validate_kernel_pointer(const void* ptr) {
    if (!ptr) {
        return false;
    }
    
    uint32_t addr = (uint32_t)ptr;
    
    /* Check if pointer is in kernel space */
    if (addr < 0x100000 || addr >= 0x1000000) {
        return false;
    }
    
    return true;
}

/* Check kernel access */
bool security_check_kernel_access(void) {
    user_t* current_user = security_get_current_user();
    if (!current_user) {
        return false;
    }
    
    return current_user->privilege >= PRIVILEGE_KERNEL;
}

/* Enter critical section */
void security_enter_critical_section(void) {
    /* Simplified implementation - in real OS this would disable interrupts */
    add_security_log_entry("CRITICAL_SECTION_ENTER", "Entered critical section", security_get_current_user());
}

/* Exit critical section */
void security_exit_critical_section(void) {
    /* Simplified implementation - in real OS this would restore interrupts */
    add_security_log_entry("CRITICAL_SECTION_EXIT", "Exited critical section", security_get_current_user());
}

/* Log security event */
void security_log_event(const char* event_type, const char* description, user_t* user) {
    add_security_log_entry(event_type, description, user);
}

/* Log security violation */
void security_log_security_violation(const char* violation_type, const char* details, user_t* user) {
    add_security_log_entry(violation_type, details, user);
    security_state.security_violations_logged++;
}

/* Display security status */
void security_display_security_status(void) {
    /* This would be implemented to display security status on screen */
    /* For now, just log the status */
    char status_msg[256];
    
    /* Build status message */
    char* ptr = status_msg;
    const char* prefix = "Security Status - Events: ";
    while (*prefix) *ptr++ = *prefix++;
    
    /* Convert event count to string */
    uint32_t events = security_state.security_events_logged;
    char num_str[16];
    int i = 0;
    
    if (events == 0) {
        num_str[i++] = '0';
    } else {
        while (events > 0) {
            num_str[i++] = '0' + (events % 10);
            events /= 10;
        }
    }
    
    /* Reverse number string */
    for (int j = i - 1; j >= 0; j--) {
        *ptr++ = num_str[j];
    }
    
    const char* violations_prefix = ", Violations: ";
    while (*violations_prefix) *ptr++ = *violations_prefix++;
    
    /* Convert violations count to string */
    uint32_t violations = security_state.security_violations_logged;
    i = 0;
    
    if (violations == 0) {
        num_str[i++] = '0';
    } else {
        while (violations > 0) {
            num_str[i++] = '0' + (violations % 10);
            violations /= 10;
        }
    }
    
    /* Reverse number string */
    for (int j = i - 1; j >= 0; j--) {
        *ptr++ = num_str[j];
    }
    
    *ptr = '\0';
    
    add_security_log_entry("SECURITY_STATUS", status_msg, NULL);
}

/* Hash password */
uint32_t security_hash_password(const char* password) {
    if (!password) {
        return 0;
    }
    
    return simple_hash(password);
}

/* Constant time comparison */
bool security_constant_time_compare(const void* a, const void* b, size_t length) {
    if (!a || !b) {
        return false;
    }
    
    const uint8_t* a_bytes = (const uint8_t*)a;
    const uint8_t* b_bytes = (const uint8_t*)b;
    uint8_t result = 0;
    
    for (size_t i = 0; i < length; i++) {
        result |= a_bytes[i] ^ b_bytes[i];
    }
    
    return result == 0;
}

/* Zero memory securely */
void security_zero_memory(void* ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }
    
    volatile uint8_t* volatile_ptr = (volatile uint8_t*)ptr;
    
    for (size_t i = 0; i < size; i++) {
        volatile_ptr[i] = 0;
    }
}

/* Check if string is printable */
bool security_is_printable_string(const char* str, size_t max_length) {
    if (!str) {
        return false;
    }
    
    size_t length = 0;
    const char* ptr = str;
    
    while (*ptr && length < max_length) {
        char c = *ptr;
        
        if (c < 32 || c > 126) {
            if (c != '\n' && c != '\r' && c != '\t') {
                return false;
            }
        }
        
        ptr++;
        length++;
    }
    
    return *ptr == '\0'; /* Must be null-terminated */
}

/* Helper functions */

static void add_security_log_entry(const char* event_type, const char* description, user_t* user) {
    if (!event_type || !description) {
        return;
    }
    
    security_log_entry_t* entry = &security_log[log_index];
    
    /* Copy event type */
    char* dest = entry->event_type;
    const char* src = event_type;
    size_t i = 0;
    while (*src && i < sizeof(entry->event_type) - 1) {
        *dest++ = *src++;
        i++;
    }
    *dest = '\0';
    
    /* Copy description */
    dest = entry->description;
    src = description;
    i = 0;
    while (*src && i < sizeof(entry->description) - 1) {
        *dest++ = *src++;
        i++;
    }
    *dest = '\0';
    
    entry->user = user;
    entry->timestamp = security_state.security_events_logged; /* Simplified timestamp */
    
    /* Update log indices */
    log_index = (log_index + 1) % SECURITY_LOG_SIZE;
    if (log_count < SECURITY_LOG_SIZE) {
        log_count++;
    }
    
    security_state.security_events_logged++;
}

static user_t* find_user_by_username(const char* username) {
    if (!username) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < user_count; i++) {
        if (user_database[i].is_active) {
            const char* user_name = user_database[i].username;
            const char* input_name = username;
            
            bool match = true;
            while (*user_name && *input_name) {
                if (*user_name != *input_name) {
                    match = false;
                    break;
                }
                user_name++;
                input_name++;
            }
            
            if (match && *user_name == '\0' && *input_name == '\0') {
                return &user_database[i];
            }
        }
    }
    
    return NULL;
}

static bool validate_user_credentials(const user_t* user, const char* password) {
    if (!user || !password) {
        return false;
    }
    
    /* Hash the provided password */
    uint32_t provided_hash = simple_hash(password);
    
    /* Compare with stored hash */
    const uint8_t* stored_hash = user->password_hash;
    const uint8_t* provided_hash_bytes = (const uint8_t*)&provided_hash;
    
    return security_constant_time_compare(stored_hash, provided_hash_bytes, sizeof(uint32_t));
}