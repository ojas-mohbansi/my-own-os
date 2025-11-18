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
