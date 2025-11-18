## Objectives
- Create a 16-bit BIOS bootloader that initializes stack and CPU state
- Load a small kernel from disk into memory and jump to its entry
- Build a raw disk image and run in QEMU

## Files
- `src/bootloader.asm`: boot sector (512 bytes, ends with 0xAA55)
- `src/kernel.asm`: minimal 16-bit kernel stub

## Bootloader
```asm
[BITS 16]
[ORG 0x7C00]
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    mov [BOOT_DRIVE], dl
    mov si, bootmsg
    mov ah, 0x0E
    mov bx, 0x0007
.print:
    lodsb
    test al, al
    jz .load
    int 0x10
    jmp .print
.load:
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 4
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error
    jmp 0x1000:0x0000
.disk_error:
    mov si, diskmsg
    mov ah, 0x0E
    mov bx, 0x0004
.dprint:
    lodsb
    test al, al
    jz .hang
    int 0x10
    jmp .dprint
.hang:
    cli
.halt:
    hlt
    jmp .halt
BOOT_DRIVE db 0
bootmsg db "Bootloader: loading kernel...", 0
diskmsg db "Disk read error", 0
times 510 - ($ - $$) db 0
DW 0xAA55
```

## Kernel Stub
```asm
[BITS 16]
[ORG 0x0000]
start:
    mov si, msg
    mov ah, 0x0E
    mov bx, 0x0007
.print:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .print
.done:
    cli
.hang:
    hlt
    jmp .hang
msg db "Kernel: started successfully", 0
```

## Build & Run (Linux/WSL)
- `nasm -f bin src/bootloader.asm -o bin/boot.bin`
- `nasm -f bin src/kernel.asm -o bin/kernel.bin`
- `dd if=/dev/zero of=bin/disk.img bs=512 count=2880`
- `dd if=bin/boot.bin of=bin/disk.img conv=notrunc`
- `dd if=bin/kernel.bin of=bin/disk.img bs=512 seek=1 conv=notrunc`
- `qemu-system-i386 -drive format=raw,file=bin/disk.img`

## Build & Run (Windows)
- `nasm -f bin src\bootloader.asm -o bin\boot.bin`
- `nasm -f bin src\kernel.asm -o bin\kernel.bin`
- `copy /b bin\boot.bin + bin\kernel.bin bin\disk.img`
- `qemu-system-i386 -drive format=raw,file=bin\disk.img`

## Acceptance Criteria
- Initializes CPU/stack in bootloader prologue
- Loads kernel via BIOS `int 13h` into `0x1000:0000`
- Jumps to kernel entry, kernel prints message and halts

## Next Steps
- Make load size dynamic via kernel header
- Add second-stage loader and transition to protected mode