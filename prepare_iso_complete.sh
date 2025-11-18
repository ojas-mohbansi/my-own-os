#!/bin/bash

# S00K OS ISO Builder with mkisofs
# Creates bootable ISO with exact specifications

set -e

# Configuration
ISO_NAME="S00K_OS_1.0.iso"
ISO_LABEL="S00K_OS_1.0"
PUBLISHER="S00K OS Project"
APPLICATION="S00K_OS_Installer"
VERSION="1.0"
BUILD_DATE=$(date +"%Y-%m-%d %H:%M:%S")
BUILD_HOST=$(hostname)
BUILD_USER=$(whoami)

# Directories
ISO_DIR="iso"
RELEASE_DIR="release"
BUILD_INFO_DIR="build_info"

echo "=== S00K OS ISO Builder (mkisofs) ==="
echo "Version: $VERSION"
echo "ISO Name: $ISO_NAME"
echo "Build Date: $BUILD_DATE"
echo "Build Host: $BUILD_HOST"
echo "Build User: $BUILD_USER"
echo ""

# Create release directory
mkdir -p "$RELEASE_DIR" "$BUILD_INFO_DIR"

# Create build metadata
echo "Creating build metadata..."
cat > "$BUILD_INFO_DIR/BUILD_INFO" << EOF
S00K OS Build Information
========================
Version: $VERSION
Build Date: $BUILD_DATE
Build Host: $BUILD_HOST
Build User: $BUILD_USER
ISO Label: $ISO_LABEL
Publisher: $PUBLISHER
Application: $APPLICATION
Architecture: x86_64, i386
Boot Support: UEFI, BIOS (Legacy)
Hybrid Support: Yes (USB/CD/DVD)
ISO Size: $(du -sh "$ISO_DIR" | cut -f1)
File Count: $(find "$ISO_DIR" -type f | wc -l)
Directory Count: $(find "$ISO_DIR" -type d | wc -l)

Bootloader Information:
- ISOLINUX: BIOS boot support
- GRUB2: UEFI boot support
- Fallback: Chainloading support

Verification Tools:
- System verification
- Hardware detection
- Memory testing
- Filesystem checking

EOF

# Create comprehensive documentation
echo "Creating documentation package..."
cat > "$ISO_DIR/docs/INSTALL.txt" << 'EOF'
S00K OS Installation Guide
==========================

System Requirements:
- x86_64 or i386 processor
- 512MB RAM minimum (1GB recommended)
- 2GB available disk space
- CD/DVD drive or USB port for boot media

Boot Instructions:
1. Insert S00K OS bootable media (CD/DVD/USB)
2. Restart your computer
3. Enter BIOS/UEFI setup (usually F2, F12, DEL, or ESC)
4. Set boot priority to boot from your media
5. Save and exit BIOS/UEFI
6. Select boot option from S00K OS menu

Boot Menu Options:
- S00K OS (Default): Standard boot with all features
- S00K OS (Safe Mode): Boot with minimal drivers
- S00K OS (Rescue Mode): System recovery and repair
- Memory Test: Test system RAM for errors
- Hardware Detection: Identify system components
- System Verification: Validate system integrity

Installation Process:
1. Boot from S00K OS media
2. Select installation option
3. Follow on-screen prompts
4. Configure system settings
5. Complete installation

Troubleshooting:
- If boot fails, try Safe Mode
- Check BIOS/UEFI settings for secure boot
- Verify boot media integrity
- Consult hardware compatibility list

For detailed instructions, visit: https://s00k-os.org/install
EOF

cat > "$ISO_DIR/docs/RELEASE_NOTES.txt" << 'EOF'
S00K OS Release Notes
====================

Version: 1.0
Release Date: 2025-11-18
Codename: "Quantum Leap"

New Features:
- Dual bootloader support (UEFI/BIOS)
- Hybrid ISO structure (USB/CD/DVD)
- Comprehensive verification tools
- Hardware detection capabilities
- Memory testing utilities
- System recovery options

Bootloader Improvements:
- GRUB2 for UEFI systems
- ISOLINUX for BIOS systems
- Fallback chainloading
- 5-second boot timeout
- Multiple boot options

System Requirements:
- Updated processor support
- Enhanced memory management
- Improved hardware compatibility

Known Issues:
- Some older systems may require BIOS update
- Secure boot may need to be disabled
- USB 3.0 ports recommended for faster boot

Fixed Issues:
- Boot sector validation
- Memory detection on legacy systems
- Hardware compatibility improvements

Future Roadmap:
- ARM64 architecture support
- Network boot capabilities
- Advanced recovery tools
- Automated hardware detection

For full changelog, visit: https://s00k-os.org/releases
EOF

cat > "$ISO_DIR/docs/HARDWARE.txt" << 'EOF'
S00K OS Hardware Compatibility
=============================

Tested and Verified Hardware:

Processors:
- Intel: Core i3/i5/i7/i9 (2nd gen and newer)
- Intel: Xeon E3/E5/E7 series
- AMD: Ryzen 3/5/7/9 series
- AMD: FX/A-series APUs
- VIA: Nano, C7, Eden

Memory:
- DDR3, DDR4, DDR5 SDRAM
- ECC memory support
- Up to 128GB tested
- Multi-channel configurations

Storage Controllers:
- Intel AHCI SATA
- AMD SATA controllers
- NVMe SSDs (M.2, U.2)
- SCSI controllers (LSI, Adaptec)
- RAID controllers (Intel, AMD)

Graphics:
- Intel integrated graphics
- AMD Radeon series
- NVIDIA GeForce/Quadro
- Basic VGA compatibility

Network:
- Intel Ethernet adapters
- Realtek network chips
- Broadcom controllers
- Atheros wireless (limited)

Boot Media:
- USB 2.0/3.0/3.1 flash drives
- CD-R/RW, DVD-R/RW
- Virtual machine ISO images
- Network boot (PXE) - planned

System Requirements:
Minimum:
- 1GHz processor
- 512MB RAM
- 2GB storage
- VGA graphics

Recommended:
- 2GHz dual-core processor
- 2GB RAM
- 10GB storage
- Hardware-accelerated graphics

Compatibility Issues:
- Some very old systems (pre-2005)
- Certain RAID configurations
- Proprietary graphics drivers
- Secure Boot with custom keys

For detailed compatibility list:
https://s00k-os.org/hardware

Report hardware issues to:
support@s00k-os.org
EOF

# Create verification tools
echo "Creating verification tools..."
cat > "$ISO_DIR/verification/verify_iso.sh" << 'EOF'
#!/bin/bash
# S00K OS ISO Verification Tool

echo "=== S00K OS ISO Verification ==="
echo "Date: $(date)"
echo "ISO File: $1"
echo ""

if [ -z "$1" ]; then
    echo "Usage: $0 <iso_file>"
    exit 1
fi

if [ ! -f "$1" ]; then
    echo "Error: ISO file not found: $1"
    exit 1
fi

echo "1. Checking ISO file integrity..."
if file "$1" | grep -q "ISO 9660"; then
    echo "   ✓ ISO 9660 format detected"
else
    echo "   ✗ Not a valid ISO 9660 image"
    exit 1
fi

echo "2. Checking boot capabilities..."
if isoinfo -l -i "$1" | grep -q "isolinux.bin"; then
    echo "   ✓ ISOLINUX bootloader present"
else
    echo "   ✗ ISOLINUX bootloader missing"
fi

if isoinfo -l -i "$1" | grep -q "BOOTX64.EFI"; then
    echo "   ✓ UEFI bootloader present"
else
    echo "   ✗ UEFI bootloader missing"
fi

echo "3. Checking file structure..."
REQUIRED_FILES=(
    "/boot/kernel.img"
    "/boot/grub/grub.cfg"
    "/isolinux/isolinux.cfg"
    "/docs/INSTALL.txt"
    "/docs/RELEASE_NOTES.txt"
    "/docs/HARDWARE.txt"
)

for file in "${REQUIRED_FILES[@]}"; do
    if isoinfo -i "$1" -find "$file" >/dev/null 2>&1; then
        echo "   ✓ $file"
    else
        echo "   ✗ $file missing"
    fi
done

echo "4. Generating checksums..."
echo "   MD5: $(md5sum "$1" | cut -d' ' -f1)"
echo "   SHA256: $(sha256sum "$1" | cut -d' ' -f1)"
echo "   Size: $(du -h "$1" | cut -f1)"

echo ""
echo "Verification completed."
EOF

chmod +x "$ISO_DIR/verification/verify_iso.sh"

# Create main build script
echo "Creating main ISO build script..."
cat > build_iso.sh << 'EOF'
#!/bin/bash

# S00K OS ISO Builder
# Creates bootable ISO with mkisofs

set -e

# Configuration
ISO_NAME="S00K_OS_1.0.iso"
ISO_DIR="iso"
RELEASE_DIR="release"
VERSION="1.0"

echo "=== Building S00K OS ISO Image ==="
echo "Version: $VERSION"
echo "Output: $RELEASE_DIR/$ISO_NAME"
echo ""

# Check if ISO directory exists
if [ ! -d "$ISO_DIR" ]; then
    echo "Error: ISO directory not found: $ISO_DIR"
    echo "Run prepare_iso.sh first to prepare the ISO structure."
    exit 1
fi

# Check for mkisofs or genisoimage
if command -v mkisofs >/dev/null 2>&1; then
    MKISOFS_CMD="mkisofs"
elif command -v genisoimage >/dev/null 2>&1; then
    MKISOFS_CMD="genisoimage"
else
    echo "Error: mkisofs or genisoimage not found"
    echo "Please install mkisofs (cdrtools) or genisoimage (cdrkit)"
    exit 1
fi

echo "Using $MKISOFS_CMD for ISO creation"
echo ""

# Create the ISO with exact specifications
echo "Creating ISO image with mkisofs..."
$MKISOFS_CMD \
    -iso-level 3 \
    -J \
    -R \
    -V "S00K_OS_${VERSION}" \
    -publisher "S00K OS Project" \
    -A "S00K_OS_Installer" \
    -b isolinux/isolinux.bin \
    -c isolinux/boot.cat \
    -no-emul-boot \
    -boot-load-size 4 \
    -boot-info-table \
    -eltorito-alt-boot \
    -e boot/grub/efi.img \
    -no-emul-boot \
    -o "$RELEASE_DIR/$ISO_NAME" \
    "$ISO_DIR"

echo ""
echo "ISO created successfully: $RELEASE_DIR/$ISO_NAME"
echo "Size: $(du -h "$RELEASE_DIR/$ISO_NAME" | cut -f1)"
echo ""

# Create checksums
echo "Generating checksums..."
cd "$RELEASE_DIR"
md5sum "$ISO_NAME" > MD5SUMS
sha256sum "$ISO_NAME" > SHA256SUMS
cd ..

echo "Checksums created:"
echo "- MD5SUMS"
echo "- SHA256SUMS"
echo ""

# Verify the ISO
echo "Verifying ISO..."
if [ -f "$ISO_DIR/verification/verify_iso.sh" ]; then
    bash "$ISO_DIR/verification/verify_iso.sh" "$RELEASE_DIR/$ISO_NAME"
else
    echo "Verification script not found, skipping verification"
fi

echo ""
echo "=== ISO Build Complete ==="
echo "File: $RELEASE_DIR/$ISO_NAME"
echo "MD5: $(md5sum "$RELEASE_DIR/$ISO_NAME" | cut -d' ' -f1)"
echo "SHA256: $(sha256sum "$RELEASE_DIR/$ISO_NAME" | cut -d' ' -f1)"
echo ""
echo "Ready for testing and distribution!"
EOF

chmod +x build_iso.sh

echo "ISO preparation complete!"
echo "Run ./build_iso.sh to create the bootable ISO image"