/* linker.ld - link script for 32-bit kernel at 1 MiB */

ENTRY(_kernel_main)

SECTIONS
{
    . = 0x100000;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
    }
}