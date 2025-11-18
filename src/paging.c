/* paging.c - setup 4 KiB page tables for identity mapping first 4 MiB
   Establishes a single page directory entry pointing to the first page table,
   with identity mapping (physical == virtual) for the first 4 MiB. */

#include <stdint.h>

#define PAGE_PRESENT    0x001
#define PAGE_WRITE      0x002
#define PAGE_USER       0x004

/* align to 4 KiB */
static uint32_t page_directory[1024] __attribute__((aligned(4096))); /* CR3 points here */
static uint32_t first_page_table[1024] __attribute__((aligned(4096))); /* 1024 * 4 KiB = 4 MiB */

/* identity map first 4 MiB (0x00000000 - 0x00400000) */
void init_paging(void) {
    /* Zero PD and PT, then fill PT with identity mappings. */
    for (int i = 0; i < 1024; ++i) {
        page_directory[i] = 0;
    }
    /* Populate first page table */
    for (int i = 0; i < 1024; ++i) {
        /* map each 4 KiB page to itself with present+write */
        first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
    }
    /* Link PD[0] to first page table (present + writable) */
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_WRITE;
    /* Load PD base into CR3 */
    __asm__ volatile (
        "mov %0, %%cr3"
        : : "r" (page_directory)
    );
    /* Enable paging: set CR0.PG (bit 31) */
    uint32_t cr0;
    __asm__ volatile (
        "mov %%cr0, %0"
        : "=r" (cr0)
    );
    cr0 |= 0x80000000;
    __asm__ volatile (
        "mov %0, %%cr0"
        : : "r" (cr0)
    );
}