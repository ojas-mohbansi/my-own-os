/* kernel32.asm - 32-bit kernel entry, callable from bootloader */

[BITS 32]
[EXTERN kernel_main]
global init_paging
global init_memory_management

; entry point called by bootloader far jump
pm_entry:
    ; set up data segments
    mov ax, 0x10        ; data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; high stack
    call kernel_main
    jmp $

; wrapper to call C paging init
init_paging:
    call paging_init_c
    ret

; wrapper to call C memory management init
init_memory_management:
    call memory_init_c
    ret

; C implementations (provided by paging.c and memory_management.c)
[EXTERN paging_init_c]
[EXTERN memory_init_c]