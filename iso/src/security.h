/* security.h - Security best practices and validation functions for the OS */

#ifndef SECURITY_H
#define SECURITY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Security-related error codes */
#define ERR_SECURITY_VIOLATION      -100
#define ERR_INVALID_INPUT           -101
#define ERR_BUFFER_OVERFLOW         -102
#define ERR_UNAUTHORIZED_ACCESS     -103
#define ERR_AUTHENTICATION_FAILED   -104
#define ERR_ACCESS_DENIED           -105
#define ERR_INVALID_PERMISSIONS     -106
#define ERR_MEMORY_VIOLATION      -107

/* Maximum security constants */
#define MAX_USERNAME_LENGTH     32
#define MAX_PASSWORD_LENGTH     64
#define MAX_INPUT_LENGTH        256
#define MAX_PATH_LENGTH         256
#define MAX_COMMAND_LENGTH      128
#define MAX_ARGS                16
#define MAX_FILE_NAME_LENGTH    64

/* User privilege levels */
typedef enum {
    PRIVILEGE_GUEST = 0,
    PRIVILEGE_USER = 1,
    PRIVILEGE_ADMIN = 2,
    PRIVILEGE_KERNEL = 3
} privilege_level_t;

/* User structure for authentication */
typedef struct {
    char username[MAX_USERNAME_LENGTH];
    uint8_t password_hash[MAX_PASSWORD_LENGTH];
    privilege_level_t privilege;
    bool is_active;
    uint32_t session_id;
} user_t;

/* Memory protection flags */
typedef enum {
    MEM_PROT_NONE = 0,
    MEM_PROT_READ = 1,
    MEM_PROT_WRITE = 2,
    MEM_PROT_EXECUTE = 4,
    MEM_PROT_ALL = 7
} memory_protection_t;

/* Memory region structure */
typedef struct {
    void* base_address;
    size_t size;
    memory_protection_t protection;
    user_t* owner;
    bool is_allocated;
} memory_region_t;

/* Security validation functions */

/* Input validation */
bool security_validate_input(const char* input, size_t max_length);
bool security_validate_filename(const char* filename);
bool security_validate_path(const char* path);
bool security_validate_command(const char* command);
bool security_sanitize_input(char* output, const char* input, size_t max_length);

/* Buffer overflow protection */
size_t security_safe_strcpy(char* dest, const char* src, size_t dest_size);
size_t security_safe_strcat(char* dest, const char* src, size_t dest_size);
bool security_check_buffer_bounds(const void* buffer, size_t size, const void* ptr);

/* User authentication and authorization */
bool security_init_authentication(void);
bool security_create_user(const char* username, const char* password, privilege_level_t privilege);
bool security_authenticate_user(const char* username, const char* password);
bool security_check_permission(user_t* user, privilege_level_t required_privilege);
bool security_logout_user(user_t* user);
user_t* security_get_current_user(void);

/* Memory protection */
bool security_init_memory_protection(void);
bool security_protect_memory_region(void* address, size_t size, memory_protection_t protection);
bool security_check_memory_access(const void* address, size_t size, memory_protection_t access_type);
bool security_allocate_secure_memory(void** ptr, size_t size, memory_protection_t protection);
bool security_free_secure_memory(void* ptr);

/* Kernel security functions */
bool security_init_kernel_protection(void);
bool security_validate_kernel_pointer(const void* ptr);
bool security_check_kernel_access(void);
void security_enter_critical_section(void);
void security_exit_critical_section(void);

/* Audit and logging */
void security_log_event(const char* event_type, const char* description, user_t* user);
void security_log_security_violation(const char* violation_type, const char* details, user_t* user);
void security_display_security_status(void);

/* Utility functions */
uint32_t security_hash_password(const char* password);
bool security_constant_time_compare(const void* a, const void* b, size_t length);
void security_zero_memory(void* ptr, size_t size);
bool security_is_printable_string(const char* str, size_t max_length);

/* Security macros for common operations */
#define SECURITY_VALIDATE_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            security_log_security_violation("NULL_POINTER", "Null pointer detected", NULL); \
            return false; \
        } \
    } while(0)

#define SECURITY_VALIDATE_INPUT_LENGTH(input, max_len) \
    do { \
        if (!security_validate_input((input), (max_len))) { \
            security_log_security_violation("INVALID_INPUT", "Input validation failed", security_get_current_user()); \
            return false; \
        } \
    } while(0)

#define SECURITY_CHECK_PERMISSION(user, required_priv) \
    do { \
        if (!security_check_permission((user), (required_priv))) { \
            security_log_security_violation("PERMISSION_DENIED", "Insufficient privileges", (user)); \
            return false; \
        } \
    } while(0)

#define SECURITY_SAFE_COPY(dest, src, dest_size) \
    do { \
        if (security_safe_strcpy((dest), (src), (dest_size)) >= (dest_size)) { \
            security_log_security_violation("BUFFER_OVERFLOW", "Potential buffer overflow prevented", security_get_current_user()); \
            return false; \
        } \
    } while(0)

/* Global security state */
typedef struct {
    bool is_initialized;
    user_t* current_user;
    uint32_t security_events_logged;
    uint32_t security_violations_logged;
    bool kernel_protection_enabled;
    bool memory_protection_enabled;
    bool authentication_enabled;
} security_state_t;

/* Global security state accessor */
security_state_t* security_get_state(void);

/* Initialization function */
bool security_init(void);

#endif /* SECURITY_H */