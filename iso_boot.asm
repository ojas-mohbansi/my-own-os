; Simple boot sector for S00K OS
; Creates a bootable ISO that can load our kernel

BITS 16
ORG 0x7C00

start:
    ; Set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Print boot message
    mov si, boot_msg
    call print_string

    ; Try to load kernel from sector 2
    mov ah, 0x02        ; Read sectors
    mov al, 15          ; Read 15 sectors (7.5KB)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2
    mov dh, 0           ; Head 0
    mov bx, 0x1000      ; Load at 0x1000
    int 0x13
    jc disk_error

    ; Print success message
    mov si, success_msg
    call print_string

    ; Jump to kernel
    jmp 0x1000:0000

disk_error:
    mov si, error_msg
    call print_string
    hlt

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp print_string
.done:
    ret

boot_msg: db "S00K OS ISO Booting...", 0x0D, 0x0A, 0
success_msg: db "Kernel loaded successfully!", 0x0D, 0x0A, 0
error_msg: db "Disk read error! System halted.", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55