/* Security stubs for minimal kernel build */
#include "security.h"

/* Stub implementations for security functions */
user_t* security_get_current_user(void) {
    static user_t kernel_user = { .username = "kernel", .privilege = PRIVILEGE_KERNEL };
    return &kernel_user;
}

void security_log_security_violation(const char* violation_type, const char* details, user_t* user) {
    /* No-op for minimal build */
}

void security_log_event(const char* event_type, const char* description, user_t* user) {
    /* No-op for minimal build */
}