[BITS 32]

global kernel_main
extern kernel_main_c

section .text
kernel_main:
    ; Simple kernel entry point
    ; Write 'K' to VGA text buffer to show we're running
    mov byte [0xB8000], 'K'
    mov byte [0xB8001], 0x0F
    
    ; Call C kernel main function
    call kernel_main_c
    
    ; Halt if we return
    cli
.hang:
    hlt
    jmp .hang