#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "S00K OS Image Builder"
echo "===================="

BUILD_DIR="bin"
IMAGE="S00K_OS.img"

mkdir -p "$BUILD_DIR"

echo "[1/6] Assembling bootloader..."
nasm -f bin src/bootloader.asm -o "$BUILD_DIR/boot.bin"
if [[ $(stat -c%s "$BUILD_DIR/boot.bin") -ne 512 ]]; then
    echo "ERROR: Boot sector must be exactly 512 bytes."
    exit 1
fi

echo "[2/6] Compiling kernel sources..."
gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=c99 -Isrc -c src/kernel.c -o "$BUILD_DIR/kernel.o"
gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=c99 -Isrc -c src/memory_management.c -o "$BUILD_DIR/memory_management.o"
gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=c99 -Isrc -c src/io.c -o "$BUILD_DIR/io.o"
gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=c99 -Isrc -c src/file_system.c -o "$BUILD_DIR/file_system.o"
gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=c99 -Isrc -c src/string.c -o "$BUILD_DIR/string.o"
gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=c99 -Isrc -c src/paging.c -o "$BUILD_DIR/paging.o"
gcc -m32 -ffreestanding -O2 -Wall -Wextra -std=c99 -Isrc -c src/security_stubs.c -o "$BUILD_DIR/security_stubs.o"

echo "[3/6] Assembling kernel stub..."
nasm -f elf32 src/kernel.asm -o "$BUILD_DIR/kernel_asm.o"

echo "[4/6] Linking kernel..."
ld -m elf_i386 -T src/linker.ld -nostdlib -o "$BUILD_DIR/kernel.elf" \
    "$BUILD_DIR/kernel.o" "$BUILD_DIR/memory_management.o" "$BUILD_DIR/io.o" \
    "$BUILD_DIR/file_system.o" "$BUILD_DIR/string.o" "$BUILD_DIR/paging.o" \
    "$BUILD_DIR/security_stubs.o" "$BUILD_DIR/kernel_asm.o"

echo "[5/6] Converting to flat binary..."
objcopy -O binary "$BUILD_DIR/kernel.elf" "$BUILD_DIR/kernel_flat.bin"

echo "[6/6] Creating disk image..."
dd if=/dev/zero of="$IMAGE" bs=512 count=2880 2>/dev/null
dd if="$BUILD_DIR/boot.bin" of="$IMAGE" conv=notrunc 2>/dev/null
dd if="$BUILD_DIR/kernel_flat.bin" of="$IMAGE" seek=1 conv=notrunc 2>/dev/null

size=$(stat -c%s "$IMAGE")
echo "✅ Image created: $IMAGE (${size} bytes)"

echo ""
echo "Run in QEMU:"
echo "  qemu-system-i386 -drive format=raw,file=$IMAGE"