#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "S00K OS Environment Setup"
echo "=========================="
echo ""

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

OS=$(detect_os)
echo "Detected OS: $OS"
echo ""

# Install dependencies based on OS
install_dependencies() {
    echo "Installing dependencies..."
    
    case "$OS" in
        "linux")
            if command_exists apt-get; then
                echo "Using apt-get package manager..."
                sudo apt-get update
                sudo apt-get install -y build-essential nasm gcc gdb qemu-system-i386
            elif command_exists yum; then
                echo "Using yum package manager..."
                sudo yum install -y gcc nasm gdb qemu-system-i386
            elif command_exists pacman; then
                echo "Using pacman package manager..."
                sudo pacman -S --needed base-devel nasm gcc gdb qemu-system-i386
            else
                echo "❌ Unsupported Linux distribution. Please install manually:"
                echo "   - GCC (32-bit cross-compiler)"
                echo "   - NASM assembler"
                echo "   - GDB debugger"
                echo "   - QEMU system emulator"
                exit 1
            fi
            ;;
        "macos")
            if command_exists brew; then
                echo "Using Homebrew package manager..."
                brew install nasm i686-elf-gcc qemu
            else
                echo "❌ Homebrew not found. Please install Homebrew first:"
                echo "   /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
                exit 1
            fi
            ;;
        "windows")
            echo "Windows detected. Please ensure you have the following installed:"
            echo "   - MinGW-w64 (32-bit) or Cygwin"
            echo "   - NASM assembler"
            echo "   - QEMU for Windows"
            echo ""
            echo "You can also use the provided qemu-setup.exe installer."
            ;;
        *)
            echo "❌ Unsupported operating system: $OS"
            exit 1
            ;;
    esac
}

# Verify installation
verify_installation() {
    echo ""
    echo "Verifying installation..."
    
    local missing_deps=()
    
    if ! command_exists nasm; then
        missing_deps+=("nasm")
    fi
    
    if ! command_exists gcc; then
        missing_deps+=("gcc")
    fi
    
    if ! command_exists qemu-system-i386; then
        missing_deps+=("qemu-system-i386")
    fi
    
    if [ ${#missing_deps[@]} -eq 0 ]; then
        echo "✅ All dependencies are installed!"
    else
        echo "❌ Missing dependencies: ${missing_deps[*]}"
        echo "Please install the missing dependencies manually."
        return 1
    fi
}

# Build the OS
build_os() {
    echo ""
    echo "Building S00K OS..."
    echo "==================="
    
    if [ -f "build_image.sh" ]; then
        chmod +x build_image.sh
        ./build_image.sh
    else
        echo "❌ build_image.sh not found!"
        exit 1
    fi
}

# Create development directories
create_directories() {
    echo ""
    echo "Creating development directories..."
    
    mkdir -p logs
    mkdir -p build
    mkdir -p docs/generated
    
    echo "✅ Development directories created"
}

# Set up development environment
setup_dev_environment() {
    echo ""
    echo "Setting up development environment..."
    
    # Create .gitignore if it doesn't exist
    if [ ! -f ".gitignore" ]; then
        cat > .gitignore << 'EOF'
# Build artifacts
bin/
build/
*.o
*.bin
*.elf
*.img
*.log

# IDE files
.vscode/
.idea/
*.swp
*.swo

# OS specific
.DS_Store
Thumbs.db

# Test executables
*.exe
test_*
my_os_test.exe

# QEMU logs
qemu.log
EOF
        echo "✅ Created .gitignore"
    fi
    
    # Create development configuration
    cat > dev_config.sh << 'EOF'
#!/usr/bin/env bash
# S00K OS Development Configuration

export S00K_OS_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export PATH="$S00K_OS_ROOT/bin:$PATH"

# QEMU configuration
export QEMU_FLAGS="-m 128M -cpu pentium -vga std"

# Development aliases
alias s00k-build="$S00K_OS_ROOT/build_image.sh"
alias s00k-run="qemu-system-i386 \$QEMU_FLAGS -drive format=raw,file=S00K_OS.img"
alias s00k-debug="qemu-system-i386 \$QEMU_FLAGS -s -S -drive format=raw,file=S00K_OS.img"

echo "S00K OS Development Environment Loaded"
echo "Available commands:"
echo "  s00k-build  - Build the OS image"
echo "  s00k-run    - Run the OS in QEMU"
echo "  s00k-debug  - Run with GDB debugging"
EOF
    
    chmod +x dev_config.sh
    echo "✅ Created development configuration (dev_config.sh)"
}

# Main setup process
main() {
    echo "Starting S00K OS environment setup..."
    echo ""
    
    # Check if we're in the right directory
    if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
        echo "❌ Error: This doesn't appear to be the S00K OS project directory."
        echo "Please run this script from the project root directory."
        exit 1
    fi
    
    # Install dependencies
    install_dependencies
    
    # Verify installation
    if verify_installation; then
        echo ""
        echo "✅ Dependencies verified successfully!"
    else
        echo ""
        echo "⚠️  Some dependencies are missing. Continuing with build anyway..."
    fi
    
    # Create directories
    create_directories
    
    # Setup development environment
    setup_dev_environment
    
    # Build the OS
    build_os
    
    echo ""
    echo "🎉 S00K OS environment setup complete!"
    echo ""
    echo "Next steps:"
    echo "1. Run the OS: qemu-system-i386 -drive format=raw,file=S00K_OS.img"
    echo "2. Source dev environment: source dev_config.sh"
    echo "3. Use aliases: s00k-build, s00k-run, s00k-debug"
    echo ""
    echo "For more information, see the documentation in docs/"
}

# Run main function
main "$@"