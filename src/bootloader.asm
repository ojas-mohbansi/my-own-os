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
    
    ; Print boot message
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
    ; Load kernel from disk (sector 2, 4 sectors)
    mov ax, 0x1000      ; Load address 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02        ; Read sectors
    mov al, 4           ; Number of sectors to read
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Sector 2 (sector 1 is bootloader)
    mov dh, 0           ; Head 0
    mov dl, [BOOT_DRIVE]
    int 0x13            ; BIOS disk read
    jc .disk_error
    
    ; Switch to protected mode
    cli
    call load_gdt
    call enable_protected_mode
    
    ; Far jump to flush pipeline and load 32-bit code segment
    jmp 0x08:pm_entry
    
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

; Data
BOOT_DRIVE db 0
bootmsg db "S00K OS Bootloader...", 0
diskmsg db "Disk read error!", 0

; GDT and protected-mode routines
; GDT descriptors
gdt_start:
    dq 0x0000000000000000       ; null descriptor
gdt_code:
    dw 0xFFFF                   ; limit 0:15
    dw 0x0000                   ; base 0:15
    db 0x00                     ; base 16:23
    db 10011010b                ; access: present, ring 0, code segment, executable, readable
    db 11001111b                ; flags: 4K pages, 32-bit, limit 16:19 = 0xF
    db 0x00                     ; base 24:31
gdt_data:
    dw 0xFFFF                   ; limit 0:15
    dw 0x0000                   ; base 0:15
    db 0x00                     ; base 16:23
    db 10010010b                ; access: present, ring 0, data segment, writable
    db 11001111b                ; flags: 4K pages, 32-bit, limit 16:19 = 0xF
    db 0x00                     ; base 24:31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; size
    dd gdt_start                ; offset

; selectors
code_seg equ gdt_code - gdt_start
data_seg equ gdt_data - gdt_start

load_gdt:
    lgdt [gdt_descriptor]
    ret

enable_protected_mode:
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    ret

; 32-bit code segment entry
[BITS 32]
pm_entry:
    mov ax, data_seg
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000      ; high stack
    
    ; Jump to kernel at 0x1000
    jmp 0x1000

times 510 - ($ - $$) db 0
DW 0xAA55