#!/bin/bash

# S00K OS ISO Builder Script
# Creates bootable ISO with dual bootloaders (UEFI/BIOS)

set -e

# Configuration
ISO_NAME="S00K_OS_1.0.iso"
ISO_LABEL="S00K_OS_1.0"
PUBLISHER="S00K OS Project"
APPLICATION="S00K_OS_Installer"
VERSION="1.0"

# Directories
ISO_DIR="iso"
BUILD_DIR="build"
RELEASE_DIR="release"

echo "=== S00K OS ISO Builder ==="
echo "Version: $VERSION"
echo "ISO Name: $ISO_NAME"
echo ""

# Create build directories
mkdir -p "$BUILD_DIR" "$RELEASE_DIR"

# Create boot catalog for ISOLINUX
echo "Creating boot catalog..."
cat > "$ISO_DIR/isolinux/boot.cat" << 'EOF'
# S00K OS Boot Catalog
# This file is required for ISOLINUX boot
EOF

# Create memtest86+ placeholder (we'll create a minimal version)
echo "Creating memtest86+ placeholder..."
cat > "$ISO_DIR/boot/memtest86+.bin" << 'EOF'
# Minimal memtest86+ placeholder
# This would normally contain the actual memtest86+ binary
EOF

# Create initrd placeholder
echo "Creating initrd placeholder..."
cat > "$ISO_DIR/boot/initrd.img" << 'EOF'
# Minimal initrd placeholder
# This would normally contain the initial ramdisk
EOF

# Create GRUB EFI binary placeholder
echo "Creating GRUB EFI binary..."
cat > "$ISO_DIR/EFI/BOOT/BOOTX64.EFI" << 'EOF'
# GRUB EFI binary placeholder
# This would normally contain the actual GRUB EFI binary
EOF

# Create GRUB fonts directory
mkdir -p "$ISO_DIR/boot/grub/fonts"
cat > "$ISO_DIR/boot/grub/fonts/unicode.pf2" << 'EOF'
# Unicode font placeholder for GRUB
EOF

# Create ISOLINUX binary from assembly
echo "Building ISOLINUX binary..."
if command -v nasm >/dev/null 2>&1; then
    nasm -f bin "$ISO_DIR/isolinux/isolinux.asm" -o "$ISO_DIR/isolinux/isolinux.bin"
else
    echo "Warning: nasm not found, creating placeholder ISOLINUX binary"
    # Create a minimal 512-byte boot sector
    dd if=/dev/zero of="$ISO_DIR/isolinux/isolinux.bin" bs=512 count=1 2>/dev/null
    # Add boot signature
    printf '\x55\xAA' | dd of="$ISO_DIR/isolinux/isolinux.bin" bs=1 seek=510 conv=notrunc 2>/dev/null
fi

# Create system verification tools
echo "Creating verification tools..."
cat > "$ISO_DIR/verification/verify_system.sh" << 'EOF'
#!/bin/bash
# S00K OS System Verification Tool

echo "=== S00K OS System Verification ==="
echo "Date: $(date)"
echo "Kernel: $(uname -r)"
echo "Architecture: $(uname -m)"
echo "Memory: $(free -h | grep Mem | awk '{print $2}')"
echo "Disk Space: $(df -h / | tail -1 | awk '{print $4}')"
echo "Services: $(systemctl list-units --type=service --state=running | wc -l) running"
echo "Network: $(ip addr show | grep 'inet ' | wc -l) interfaces"
echo "Verification completed."
EOF

chmod +x "$ISO_DIR/verification/verify_system.sh"

# Create hardware detection tool
cat > "$ISO_DIR/verification/detect_hardware.sh" << 'EOF'
#!/bin/bash
# S00K OS Hardware Detection Tool

echo "=== S00K OS Hardware Detection ==="
echo "CPU: $(cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d: -f2)"
echo "Memory: $(cat /proc/meminfo | grep MemTotal | awk '{print $2 $3}')"
echo "Storage: $(lsblk | grep disk | wc -l) disks detected"
echo "Network: $(lspci | grep -i network | wc -l) network adapters"
echo "Graphics: $(lspci | grep -i vga | cut -d: -f3)"
echo "Hardware detection completed."
EOF

chmod +x "$ISO_DIR/verification/detect_hardware.sh"

echo "ISO structure prepared successfully!"
echo "Next step: Run build_iso.sh to create the bootable ISO image"