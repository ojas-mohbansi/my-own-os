/* shell.c - Command Line Interface implementation with security enhancements
   Provides a simple authenticated shell with built-in commands. Input is validated,
   sanitized, and checked for common injection patterns before execution. */

#include "shell.h"
#include "error_codes.h"
#include "kernel.h"
#include "string.h"
#include "security.h"

/* Forward declarations for helper functions */
static void int_to_string(int32_t value, char *buffer);
static void display_error(const char* operation, int32_t error_code);

/* Security helper functions */
static bool validate_shell_input(const char* input, size_t max_length);
static bool sanitize_shell_command(char* output, const char* input, size_t max_length);
static bool check_command_injection(const char* input);
static void log_shell_security_event(const char* event, const char* details);

/* External I/O functions from kernel.c */
extern void print_char(char c);
extern void print(const char* str);
extern char read_char(void);
extern void clear_screen(void);
extern void panic(const char* msg);

/* Command table */
static const command_t commands[] = {
    {"help", cmd_help, "Display available commands"},
    {"exit", cmd_exit, "Exit the shell"},
    {"clear", cmd_clear, "Clear the screen"},
    {"echo", cmd_echo, "Display text on screen"},
    {"panic", cmd_panic, "Trigger a kernel panic (for testing)"},
    {NULL, NULL, NULL} /* End of table */
};

#include "brand.h"
static const char *PROMPT = "s00k> ";

/* Security validation function for shell input */
static bool validate_shell_input(const char* input, size_t max_length) {
    /* Ensure printable ASCII, enforce length, and reject suspicious characters. */
    if (!input) {
        log_shell_security_event("NULL_INPUT", "Null input pointer detected");
        return false;
    }
    
    size_t length = 0;
    const char* ptr = input;
    
    /* Check length and validate characters */
    while (*ptr && length < max_length) {
        char c = *ptr;
        
        /* Allow printable ASCII characters only */
        if (c < 32 || c > 126) {
            if (c != '\n' && c != '\r' && c != '\t') {
                log_shell_security_event("INVALID_CHARACTER", "Non-printable character detected");
                return false;
            }
        }
        
        /* Check for suspicious patterns */
        if (c == ';' || c == '|' || c == '&' || c == '`' || c == '$') {
            log_shell_security_event("SUSPICIOUS_PATTERN", "Potential command injection detected");
            return false;
        }
        
        ptr++;
        length++;
    }
    
    /* Check for buffer overflow attempt */
    if (*ptr != '\0') {
        log_shell_security_event("BUFFER_OVERFLOW_ATTEMPT", "Input exceeds maximum length");
        return false;
    }
    
    return true;
}

/* Sanitize shell command input */
static bool sanitize_shell_command(char* output, const char* input, size_t max_length) {
    /* Copy only safe characters into output; stop on newline. */
    if (!output || !input) {
        return false;
    }
    
    size_t out_idx = 0;
    const char* in_ptr = input;
    
    while (*in_ptr && out_idx < max_length - 1) {
        char c = *in_ptr;
        
        /* Allow safe characters only */
        if ((c >= 'a' && c <= 'z') || 
            (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || 
            c == ' ' || c == '-' || c == '_' || c == '.' || c == '/') {
            output[out_idx++] = c;
        } else if (c == '\n' || c == '\r') {
            break; /* End of command */
        }
        /* Skip dangerous characters silently */
        
        in_ptr++;
    }
    
    output[out_idx] = '\0';
    return out_idx > 0;
}

/* Check for command injection patterns */
static bool check_command_injection(const char* input) {
    /* Detect common metacharacters used in injection. */
    if (!input) return false;
    
    const char* dangerous_patterns[] = {
        ";", "|", "&&", "||", "`", "$(", "<", ">", "&", NULL
    };
    
    for (int i = 0; dangerous_patterns[i] != NULL; i++) {
        if (strstr(input, dangerous_patterns[i]) != NULL) {
            return true; /* Dangerous pattern found */
        }
    }
    
    return false; /* No dangerous patterns */
}

/* Log security events in shell */
static void log_shell_security_event(const char* event, const char* details) {
    user_t* current_user = security_get_current_user();
    security_log_security_violation(event, details, current_user);
}

/* Enhanced parse command with security validation */
void parse_command(char *input, char *argv[], int *argc) {
    *argc = 0;
    
    /* Security validation */
    if (!validate_shell_input(input, MAX_COMMAND_LENGTH)) {
        print("Error: Invalid command input detected\n");
        return;
    }
    
    if (check_command_injection(input)) {
        print("Error: Command injection attempt detected\n");
        log_shell_security_event("COMMAND_INJECTION", "Potential command injection in parse_command");
        return;
    }
    
    char *token = input;
    
    while (*token && *argc < MAX_ARGS) {
        /* Skip whitespace */
        while (*token == ' ' || *token == '\t') {
            token++;
        }
        
        if (*token == '\0') {
            break;
        }
        
        /* Found argument */
        argv[*argc] = token;
        (*argc)++;
        
        /* Find end of argument */
        while (*token && *token != ' ' && *token != '\t') {
            token++;
        }
        
        if (*token) {
            *token = '\0';
            token++;
        }
    }
}

/* Execute a command with enhanced security */
void execute_command(char *command_line) {
    /* Validate/sanitize/parse then dispatch built-in commands with permission checks. */
    char *argv[MAX_ARGS];
    int argc;
    char sanitized_command[MAX_COMMAND_LENGTH];
    
    /* Security check: validate current user permissions */
    user_t* current_user = security_get_current_user();
    if (!current_user) {
        print("Error: No authenticated user. Please log in first.\n");
        return;
    }
    
    /* Validate input */
    if (!command_line) {
        print("Error: Invalid command input\n");
        log_shell_security_event("NULL_COMMAND", "Null command line pointer");
        return;
    }
    
    /* Sanitize input */
    if (!sanitize_shell_command(sanitized_command, command_line, MAX_COMMAND_LENGTH)) {
        print("Error: Command sanitization failed\n");
        log_shell_security_event("SANITIZATION_FAILED", "Failed to sanitize command");
        return;
    }
    
    /* Parse command */
    parse_command(sanitized_command, argv, &argc);
    
    if (argc == 0) {
        return; /* Empty command */
    }
    
    /* Validate argument count */
    if (argc >= MAX_ARGS) {
        print("Error: Too many arguments (maximum is ");
        char buffer[16];
        int_to_string(MAX_ARGS - 1, buffer);
        print(buffer);
        print(")\n");
        log_shell_security_event("TOO_MANY_ARGS", "Exceeded maximum argument count");
        return;
    }
    
    /* Validate each argument */
    for (int i = 0; i < argc; i++) {
        if (!validate_shell_input(argv[i], MAX_COMMAND_LENGTH)) {
            print("Error: Invalid argument detected\n");
            log_shell_security_event("INVALID_ARGUMENT", "Invalid command argument");
            return;
        }
    }
    
    /* Check user permissions for command execution */
    if (!security_check_permission(current_user, PRIVILEGE_USER)) {
        print("Error: Insufficient privileges to execute commands\n");
        log_shell_security_event("PERMISSION_DENIED", "User lacks permission to execute commands");
        return;
    }
    
    /* Find and execute command */
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            /* Log command execution */
            security_log_event("COMMAND_EXECUTION", argv[0], current_user);
            commands[i].handler(argc, argv);
            return;
        }
    }
    
    /* Command not found */
    print("Error: Unknown command '");
    print(argv[0]);
    print("'\nType 'help' for available commands.\n");
    log_shell_security_event("UNKNOWN_COMMAND", argv[0]);
}

/* Help command */
void cmd_help(int argc, char *argv[]) {
    (void)argc;  /* Suppress unused parameter warning */
    (void)argv;  /* Suppress unused parameter warning */
    
    print("Available commands:\n");
    print("==================\n");
    
    for (int i = 0; commands[i].name != NULL; i++) {
        print("  ");
        print(commands[i].name);
        print(" - ");
        print(commands[i].description);
        print("\n");
    }
    
    print("\n");
}

/* Exit command */
void cmd_exit(int argc, char *argv[]) {
    (void)argc;  /* Suppress unused parameter warning */
    (void)argv;  /* Suppress unused parameter warning */
    
    print("Exiting shell...\n");
    print("Goodbye!\n");
    /* This will cause the shell loop to exit */
}

/* Clear command */
void cmd_clear(int argc, char *argv[]) {
    (void)argc;  /* Suppress unused parameter warning */
    (void)argv;  /* Suppress unused parameter warning */
    
    clear_screen();
}

/* Echo command */
void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        print(argv[i]);
        if (i < argc - 1) {
            print(" ");
        }
    }
    print("\n");
}

/* Panic command (for testing) */
void cmd_panic(int argc, char *argv[]) {
    (void)argc;  /* Suppress unused parameter warning */
    (void)argv;  /* Suppress unused parameter warning */
    
    print("Triggering kernel panic...\n");
    panic("Shell panic command triggered");
}

/* Enhanced main shell loop with security features */
void run_shell(void) {
    /* Authenticate user, then run a prompt-driven REPL with secure input handling. */
    char command_buffer[MAX_COMMAND_LENGTH];
    int buffer_index = 0;
    char c;
    
    /* Initialize security subsystem */
    if (!security_init()) {
        print("Error: Failed to initialize security subsystem\n");
        return;
    }
    
    /* Display security banner */
    print("\n");
    print(OS_NAME);
    print(" Shell - Secure Edition\n");
    print("=====================================\n");
    print("Security features enabled:\n");
    print("- Input validation\n");
    print("- Command injection protection\n");
    print("- User authentication required\n");
    print("- Memory protection\n");
    print("Type 'help' for available commands\n\n");
    
    /* User authentication loop */
    bool authenticated = false;
    int auth_attempts = 0;
    const int MAX_AUTH_ATTEMPTS = 3;
    
    while (!authenticated && auth_attempts < MAX_AUTH_ATTEMPTS) {
        print("Please authenticate to access the system\n");
        
        /* Get username */
        print("Username: ");
        char username[MAX_USERNAME_LENGTH];
        buffer_index = 0;
        
        while (1) {
            c = read_char();
            if (c == '\n' || c == '\r') {
                username[buffer_index] = '\0';
                print("\n");
                break;
            }
            if (buffer_index < MAX_USERNAME_LENGTH - 1 && c >= 32 && c <= 126) {
                username[buffer_index++] = c;
                print_char(c);
            }
        }
        
        /* Get password */
        print("Password: ");
        char password[MAX_PASSWORD_LENGTH];
        buffer_index = 0;
        
        while (1) {
            c = read_char();
            if (c == '\n' || c == '\r') {
                password[buffer_index] = '\0';
                print("\n");
                break;
            }
            if (buffer_index < MAX_PASSWORD_LENGTH - 1 && c >= 32 && c <= 126) {
                password[buffer_index++] = c;
                print_char('*'); /* Hide password */
            }
        }
        
        /* Authenticate user */
        if (security_authenticate_user(username, password)) {
            authenticated = true;
            print("Authentication successful. Welcome to the system!\n");
            security_log_event("USER_LOGIN", username, security_get_current_user());
        } else {
            auth_attempts++;
            print("Authentication failed. ");
            if (auth_attempts < MAX_AUTH_ATTEMPTS) {
                print("Attempts remaining: ");
                char buffer[16];
                int_to_string(MAX_AUTH_ATTEMPTS - auth_attempts, buffer);
                print(buffer);
                print("\n");
            }
            security_log_security_violation("AUTH_FAILED", username, NULL);
        }
        
        /* Clear sensitive data */
        for (int i = 0; i < MAX_USERNAME_LENGTH; i++) username[i] = 0;
        for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) password[i] = 0;
    }
    
    if (!authenticated) {
        print("Maximum authentication attempts exceeded. Access denied.\n");
        security_log_security_violation("MAX_AUTH_ATTEMPTS_EXCEEDED", "Too many failed login attempts", NULL);
        return;
    }
    
    /* Main shell loop */
    while (1) {
        /* Display prompt with user information */
        user_t* current_user = security_get_current_user();
        if (current_user) {
            print(current_user->username);
            print("@s00k> ");
        } else {
            print("s00k> ");
        }
        
        /* Reset buffer */
        buffer_index = 0;
        
        /* Read command line with enhanced security */
        int32_t read_error = ERR_SUCCESS;
        while (1) {
            c = read_char_timeout(10000, &read_error);  /* 10 second timeout */
            
            if (read_error != ERR_SUCCESS) {
                if (read_error == ERR_IO_TIMEOUT) {
                    print("\nError: Command input timeout. Please try again.\n");
                    security_log_security_violation("INPUT_TIMEOUT", "Command input timeout", current_user);
                    break;
                } else {
                    print("\nError: Failed to read input. Restarting shell...\n");
                    security_log_security_violation("INPUT_ERROR", "Failed to read input", current_user);
                    break;
                }
            }
            
            /* Handle backspace */
            if (c == '\b' && buffer_index > 0) {
                buffer_index--;
                print_char('\b');
                print_char(' ');
                print_char('\b');
                continue;
            }
            
            /* Handle Enter */
            if (c == '\n' || c == '\r') {
                command_buffer[buffer_index] = '\0';
                print("\n");
                break;
            }
            
            /* Handle normal characters with validation */
            if (buffer_index < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
                /* Additional security check for suspicious characters */
                if (c == ';' || c == '|' || c == '&' || c == '`' || c == '$' || c == '<' || c == '>') {
                    print("\nError: Suspicious character detected. Command rejected.\n");
                    security_log_security_violation("SUSPICIOUS_CHARACTER", "Dangerous character in input", current_user);
                    buffer_index = 0;
                    break;
                }
                command_buffer[buffer_index++] = c;
                print_char(c);
            }
            
            /* Handle buffer overflow */
            if (buffer_index >= MAX_COMMAND_LENGTH - 1) {
                print("\nError: Command too long (maximum is ");
                char buffer[16];
                int_to_string(MAX_COMMAND_LENGTH - 1, buffer);
                print(buffer);
                print(" characters)\n");
                security_log_security_violation("BUFFER_OVERFLOW_ATTEMPT", "Command too long", current_user);
                command_buffer[buffer_index] = '\0';
                break;
            }
        }
        
        /* Skip processing if there was an input error */
        if (read_error != ERR_SUCCESS) {
            continue;
        }
        
        /* Validate final command */
        if (!validate_shell_input(command_buffer, MAX_COMMAND_LENGTH)) {
            print("Error: Command validation failed. Command rejected.\n");
            continue;
        }
        
        /* Check for exit command */
        if (strcmp(command_buffer, "exit") == 0) {
            security_log_event("USER_LOGOUT", "User logged out", current_user);
            cmd_exit(1, NULL);
            break;
        }
        
        /* Execute command */
        execute_command(command_buffer);
        
        /* Clear command buffer for security */
        for (int i = 0; i < MAX_COMMAND_LENGTH; i++) {
            command_buffer[i] = 0;
        }
    }
}

/* Start shell function */
void start_shell(void) {
    run_shell();
}

/* Helper function to convert integer to string */
static void int_to_string(int32_t value, char *buffer) {
    if (!buffer) return;
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    int i = 0;
    int is_negative = 0;
    
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (is_negative) {
        buffer[i++] = '-';
    }
    
    buffer[i] = '\0';
    
    /* Reverse the string */
    int j = 0;
    int k = i - 1;
    while (j < k) {
        char temp = buffer[j];
        buffer[j] = buffer[k];
        buffer[k] = temp;
        j++;
        k--;
    }
}

/* Display error message with context */
static void display_error(const char* operation, int32_t error_code) {
    print("Error: ");
    print(operation);
    print(" failed - ");
    
    switch (error_code) {
        case ERR_NULL_POINTER:
            print("null pointer");
            break;
        case ERR_INVALID_PARAMETER:
            print("invalid parameter");
            break;
        case ERR_FILE_NOT_FOUND:
            print("file not found");
            break;
        case ERR_FILE_EXISTS:
            print("file already exists");
            break;
        case ERR_FILE_NAME_TOO_LONG:
            print("file name too long");
            break;
        case ERR_FILE_TOO_LARGE:
            print("file too large");
            break;
        case ERR_FILE_CORRUPTED:
            print("file corrupted");
            break;
        case ERR_INVALID_FILE_HANDLE:
            print("invalid file handle");
            break;
        case ERR_NOT_A_FILE:
            print("not a file");
            break;
        case ERR_NOT_A_DIRECTORY:
            print("not a directory");
            break;
        case ERR_INVALID_DIRECTORY:
            print("invalid directory");
            break;
        case ERR_DIRECTORY_NOT_EMPTY:
            print("directory not empty");
            break;
        case ERR_FILE_SYSTEM_FULL:
            print("file system full");
            break;
        case ERR_OUT_OF_SPACE:
            print("out of space");
            break;
        case ERR_FILE_SYSTEM_INIT_FAILED:
            print("file system initialization failed");
            break;
        case ERR_IO_TIMEOUT:
            print("I/O timeout");
            break;
        case ERR_OUT_OF_MEMORY:
            print("out of memory");
            break;
        default:
            print("unknown error");
            break;
    }
    print("\n");
}