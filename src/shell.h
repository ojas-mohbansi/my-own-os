/* shell.h - Command Line Interface for the operating system */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

#define MAX_COMMAND_LENGTH 128
#define MAX_ARGS 16
#define MAX_ARG_LENGTH 32

/* Command handler function type */
typedef void (*command_handler_t)(int argc, char *argv[]);

/* Command structure */
typedef struct {
    const char *name;
    command_handler_t handler;
    const char *description;
} command_t;

/* Shell functions */
void run_shell(void);
void start_shell(void);

/* Command handlers */
void cmd_help(int argc, char *argv[]);
void cmd_exit(int argc, char *argv[]);
void cmd_clear(int argc, char *argv[]);
void cmd_echo(int argc, char *argv[]);
void cmd_panic(int argc, char *argv[]);

/* Utility functions */
void parse_command(char *input, char *argv[], int *argc);
void execute_command(char *command_line);

#endif /* SHELL_H */