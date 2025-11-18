# S00K OS User Guide

## Table of Contents
- [Introduction](#introduction)
- [System Requirements](#system-requirements)
- [Installation Guide](#installation-guide)
- [Getting Started](#getting-started)
- [Using the Shell](#using-the-shell)
- [File System Operations](#file-system-operations)
- [Memory Management](#memory-management)
- [Security Features](#security-features)
- [Performance Monitoring](#performance-monitoring)
- [Scalability Features](#scalability-features)
- [Branding Assets](#branding-assets)
- [Troubleshooting](#troubleshooting)
- [Advanced Usage](#advanced-usage)

## Introduction

S00K OS is a lightweight, educational operating system designed to demonstrate fundamental OS concepts including memory management, file systems, process management, and security. This guide will help you install, configure, and effectively use the operating system.

### Key Features
- **Interactive Shell**: Command-line interface with built-in commands
- **Memory Management**: Dynamic memory allocation and paging support
- **File System**: In-memory file system with basic operations
- **Security**: User authentication, input validation, and access control
- **Error Handling**: Comprehensive error reporting and recovery
- **Performance Monitoring**: Built-in profiling and optimization tools
- **Testing Framework**: Comprehensive test suite for validation

## System Requirements

### Minimum Requirements
- **Processor**: x86 or x86_64 compatible CPU
- **Memory**: 512MB RAM minimum, 1GB recommended
- **Storage**: 100MB free disk space
- **Graphics**: VGA compatible display (text mode)
- **Input**: Standard keyboard

### Development Requirements
- **Compiler**: GCC with C99 support
- **Build Tools**: Make (for Unix-like systems) or Windows batch files
- **Emulator**: QEMU, Bochs, or VirtualBox (for testing)
- **Debugger**: GDB (optional, for debugging)

## Installation Guide

### Method 1: Using Pre-built Images

1. **Download the latest release** from the project repository
2. **Extract the archive** to your desired location
3. **Run the OS** using an emulator:
   ```bash
   qemu-system-i386 -fda my_os.img
   ```

### Method 2: Building from Source

#### Windows Build
1. **Clone the repository**:
   ```cmd
   git clone https://github.com/yourusername/my-own-os.git
   cd my-own-os
   ```

2. **Build the OS**:
   ```cmd
   build.bat
   ```

3. **Run tests**:
   ```cmd
   test_framework.bat
   ```

4. **Launch in emulator**:
   ```cmd
   run_qemu.bat
   ```

#### Linux/Unix Build
1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/my-own-os.git
   cd my-own-os
   ```

2. **Build the OS**:
   ```bash
   make
   ```

3. **Run tests**:
   ```bash
   make test-all
   ```

4. **Launch in emulator**:
   ```bash
   make run
   ```

## Getting Started

### First Boot

When you first boot S00K OS, you'll see:

```
S00K OS v1.0
Initializing subsystems...
✓ Security subsystem initialized
✓ Memory management initialized
✓ File system initialized
✓ I/O system initialized

S00K OS Shell v1.0
Type 'help' for a list of commands.
os> 
```

### User Authentication

The OS includes a security system with user authentication:

1. **Default users**:
   - Username: `admin`, Password: `admin123` (Administrator privileges)
   - Username: `guest`, Password: `guest` (Guest privileges)

2. **Login process**:
   ```
   Username: admin
   Password: ********
   Authentication successful. Welcome, admin!
   ```

3. **Creating new users** (admin only):
   ```
   os> createuser username password privilege_level
   ```

## Using the Shell

### Basic Commands

#### `help` - Display available commands
```
os> help
Available commands:
  help              - Show this help message
  clear             - Clear the screen
  echo <text>       - Display text
  ls                - List files in current directory
  cat <filename>    - Display file contents
  touch <filename>  - Create a new file
  rm <filename>     - Remove a file
  mkdir <dirname>   - Create a directory
  rmdir <dirname>   - Remove a directory
  meminfo           - Display memory information
  panic             - Test error handling (emergency)
  exit              - Exit the shell
```

#### `clear` - Clear the screen
```
os> clear
[Screen clears]
```

#### `echo` - Display text
```
os> echo Hello, World!
Hello, World!
```

#### `ls` - List files
```
os> ls
Directory listing for root:
  README.txt
  config.ini
  documents/
  programs/
```

### File Operations

#### `cat` - Display file contents
```
os> cat README.txt
Welcome to S00K OS!
This is a sample text file.
```

#### `touch` - Create a new file
```
os> touch myfile.txt
File 'myfile.txt' created successfully.
```

#### `rm` - Remove a file
```
os> rm myfile.txt
File 'myfile.txt' removed successfully.
```

#### `mkdir` - Create a directory
```
os> mkdir myfolder
Directory 'myfolder' created successfully.
```

#### `rmdir` - Remove a directory
```
os> rmdir myfolder
Directory 'myfolder' removed successfully.
```

### System Commands

#### `meminfo` - Display memory information
```
os> meminfo
Memory Information:
Total Memory: 16 MB
Used Memory: 2.3 MB
Free Memory: 13.7 MB
Allocation Count: 47
Free Count: 12
Fragmentation: 3%
```

#### `panic` - Test error handling
```
os> panic
Triggering kernel panic for testing...
ERROR: Kernel panic initiated by user
Location: shell.c:156
Severity: FATAL
System halted.
```

## File System Operations

### Directory Structure
The file system uses a hierarchical structure:
```
/
├── README.txt
├── config.ini
├── documents/
│   ├── notes.txt
│   └── projects/
│       └── project1.txt
└── programs/
    └── calculator.bin
```

### File System Limits
- **Maximum file size**: 64KB
- **Maximum filename length**: 64 characters
- **Maximum files per directory**: 128
- **Maximum directory depth**: 16 levels

### Advanced File Operations

#### Creating files with content
```
os> echo "My content here" > myfile.txt
File 'myfile.txt' created with content.
```

#### Appending to files
```
os> echo "Additional line" >> myfile.txt
Content appended to 'myfile.txt'.
```

#### File permissions (for multi-user systems)
```
os> chmod 755 myfile.txt
Permissions for 'myfile.txt' set to 755.
```

## Memory Management

### Memory Commands

#### `meminfo` - Detailed memory statistics
```
os> meminfo
Memory Information:
Total Memory: 16 MB
Used Memory: 2.3 MB
Free Memory: 13.7 MB
Allocation Count: 47
Free Count: 12
Fragmentation: 3%
Largest Free Block: 8.2 MB
```

#### `malloc` - Allocate memory (debugging)
```
os> malloc 1024
Allocated 1024 bytes at address 0x00124000
```

#### `free` - Free memory (debugging)
```
os> free 0x00124000
Memory at 0x00124000 freed successfully.
```

### Memory Protection
The OS includes memory protection features:
- Bounds checking for all memory operations
- Prevention of buffer overflows
- Secure memory allocation with validation
- Memory zeroing for sensitive data

## Security Features

### User Management

#### Creating users (admin only)
```
os> createuser john secretpass user
User 'john' created with 'user' privileges.
```

#### Changing passwords
```
os> passwd
Current password: ********
New password: ********
Confirm password: ********
Password changed successfully.
```

#### Viewing user information
```
os> whoami
Current user: admin (Administrator)
```

### Security Validation
The OS automatically validates:
- **Input sanitization**: Prevents command injection
- **Buffer bounds**: Prevents buffer overflows
- **Path traversal**: Blocks malicious file paths
- **Memory access**: Validates all memory operations
- **User permissions**: Enforces privilege levels

### Security Logging
Security events are automatically logged:
```
os> security_log
Recent security events:
[2024-01-15 10:30:45] User 'admin' authenticated successfully
[2024-01-15 10:31:02] File 'config.ini' accessed by 'admin'
[2024-01-15 10:31:15] Memory protection violation blocked
```

## Performance Monitoring

### Performance Commands

#### `perf` - Show performance metrics
```
os> perf
Performance Metrics:
CPU Usage: 15%
Memory Usage: 14%
Disk I/O: 23 operations/sec
Network I/O: Not available
Context Switches: 142/sec
```

#### `profile` - Start performance profiling
```
os> profile start
Performance profiling started.
Run 'profile stop' to view results.
```

#### `profile stop` - Stop profiling and show results
```
os> profile stop
Performance Profile Results:
Function: kernel_main, Calls: 1, Time: 0.45ms
Function: allocate_memory, Calls: 47, Time: 0.12ms
Function: fs_create_file, Calls: 5, Time: 0.08ms
```

### Optimization Features
- **Memory optimization**: Automatic defragmentation
- **I/O optimization**: Buffered operations
- **CPU optimization**: Efficient scheduling
- **Performance regression testing**: Built-in benchmarks

## Scalability Features

### Overview
The OS includes a cooperative scheduler and basic load balancer designed to handle concurrent tasks efficiently across multiple logical CPUs.

### Enabling the Scheduler
```
init_scheduler 4
```
Initializes the scheduler with 4 logical CPUs.

### Creating Threads
```
create_thread <function> <arg> <priority>
```
Creates a runnable thread bound to the least loaded CPU.

### Yielding Execution
```
yield
```
Signals the scheduler to switch to the next runnable thread.

### Load Balancing
```
load_balance
```
Rebalances READY threads across CPUs to keep workloads even.

### Example: Running 12 Tasks
```
init_scheduler 4
for i in 0..11: create_thread worker NULL 1
loop until done:
  schedule_process
  every 32 ticks: load_balance
```
Supports at least 10 concurrent tasks without significant degradation.

## Branding Assets

### Logo and Icons
- Vector logo: `assets/branding/s00k-os-logo.svg` (scales cleanly to 512×512+)
- Icon source: `assets/branding/s00k-os-icon.svg` (export PNGs at 16/32/48/64/128/256/512)

### Wallpapers
- Dark: `assets/branding/s00k-os-wallpaper-dark.svg`
- Light: `assets/branding/s00k-os-wallpaper-light.svg`
- Export to 4K PNG (3840×2160) for desktop backgrounds

### Boot Experience
- Text-mode boot shows `S00K OS` banner and phase progress with a spinner
- Phases: Security, Paging, Memory, File System, I/O, Shell

## Troubleshooting

### Common Issues

#### "File not found" error
```
os> cat nonexistent.txt
ERROR: File 'nonexistent.txt' not found
```
**Solution**: Check the filename spelling and ensure the file exists using `ls`.

#### "Permission denied" error
```
os> rm protected.txt
ERROR: Permission denied for file 'protected.txt'
```
**Solution**: Ensure you have appropriate user privileges or contact the system administrator.

#### "Memory allocation failed" error
```
os> malloc 999999999
ERROR: Memory allocation failed
```
**Solution**: Request a smaller amount of memory or free some memory first.

#### "Invalid command" error
```
os> invalid_command
ERROR: Unknown command 'invalid_command'
```
**Solution**: Use `help` to see available commands.

### System Recovery

#### Safe Mode
If the system becomes unstable, boot into safe mode:
```
boot: safe
```

#### Emergency Shell
For system recovery, use the emergency shell:
```
boot: emergency
```

#### Factory Reset
To reset the system to default state:
```
os> factory_reset
WARNING: This will erase all data. Continue? (y/N): y
System reset complete. Please reboot.
```

## Advanced Usage

### Scripting

#### Creating shell scripts
```
os> echo "#!/bin/os" > script.os
os> echo "echo Starting backup..." >> script.os
os> echo "ls > file_list.txt" >> script.os
os> chmod +x script.os
```

#### Running scripts
```
os> ./script.os
Starting backup...
```

### System Configuration

#### Editing configuration files
```
os> edit config.ini
[system]
memory_limit=16MB
max_files=1024
security_level=high
```

#### Environment variables
```
os> set PATH=/bin:/usr/bin
os> set HOME=/home/user
os> echo $PATH
/bin:/usr/bin
```

### Network Configuration (if available)
```
os> netconfig
Network Configuration:
IP Address: 192.168.1.100
Subnet Mask: 255.255.255.0
Gateway: 192.168.1.1
DNS: 8.8.8.8
```

### Debugging

#### Debug mode
```
os> debug on
Debug mode enabled.
```

#### View system logs
```
os> dmesg
[Kernel] System booted successfully
[Memory] Paging initialized
[FS] File system mounted
[Security] Security subsystem activated
```

#### Core dump analysis
```
os> coredump analyze
Analyzing core dump...
Segmentation fault at address 0x00123000
Process: shell
Thread: main
Call stack:
  0x00100000: kernel_main
  0x00102000: start_shell
  0x00102340: process_command
```

## Getting Help

### Online Resources
- **Project Repository**: https://github.com/yourusername/my-own-os
- **Issue Tracker**: https://github.com/yourusername/my-own-os/issues
- **Documentation**: https://github.com/yourusername/my-own-os/wiki

### Community Support
- **Discussion Forum**: https://forum.my-own-os.org
- **Chat Channel**: #my-own-os on IRC
- **Email Support**: support@my-own-os.org

### Contributing
We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

---

**Version**: 1.0.0  
**Last Updated**: January 2024  
**License**: MIT License