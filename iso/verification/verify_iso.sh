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
