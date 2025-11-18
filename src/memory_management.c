/* memory_management.c - basic page frame allocator and identity paging with security enhancements
   Implements a simple page-frame bitmap allocator and registers per-user memory regions
   for access control. Allocation is page-granular (4 KiB) and ownership is tracked to
   enforce permissions via the security subsystem. */

#include <stddef.h>
#include <stdint.h>
#include "security.h"
#include "scalability.h"

#define PAGE_SIZE        4096
#define KERNEL_END       0x100000     /* 1 MiB, adjust as needed */
#define PHYS_MEMORY_END  0x1000000    /* 16 MiB for now */
#define BITMAP_SIZE      (PHYS_MEMORY_END / PAGE_SIZE / 8)
#define MAX_MEMORY_REGIONS  1024

static uint8_t page_bitmap[BITMAP_SIZE] __attribute__((aligned(PAGE_SIZE))); /* 1 bit per page */
static uint32_t next_free_page = 0;
static sc_lock_t mm_lock = 0;

/* Security tracking for memory regions */
static memory_region_t memory_regions[MAX_MEMORY_REGIONS];
static uint32_t region_count = 0;
static bool memory_protection_enabled = false;

/* Forward declarations for security functions */
static bool validate_memory_access(const void* address, size_t size, memory_protection_t access_type); /* bounds/overflow/ownership */
static bool register_memory_region(void* address, size_t size, memory_protection_t protection, user_t* owner);
static bool unregister_memory_region(const void* address);
static void log_memory_security_event(const char* event, const char* details, const void* address);

/* Security validation function for memory access */
static bool validate_memory_access(const void* address, size_t size, memory_protection_t access_type) {
    /* Central guard for all memory operations. Prevents overflow in address math,
       rejects kernel-space accesses, enforces alignment and per-region permissions/ownership. */
    if (!memory_protection_enabled) {
        return true; /* Protection disabled, allow access */
    }
    
    if (!address || size == 0) {
        log_memory_security_event("INVALID_ACCESS", "Null address or zero size", address);
        return false;
    }
    
    /* Detect overflow in address arithmetic */
    uint32_t start_addr = (uint32_t)address;
    uint32_t end_addr;
    
    /* Check for overflow in address + size calculation */
    if (start_addr + size < start_addr) {
        log_memory_security_event("ADDRESS_OVERFLOW", "Address calculation overflow", address);
        return false;
    }
    end_addr = start_addr + size;
    
    /* Enforce user-space bounds */
    if (start_addr < KERNEL_END || end_addr > PHYS_MEMORY_END) {
        log_memory_security_event("OUT_OF_BOUNDS", "Memory access out of bounds", address);
        return false;
    }
    
    /* Require page alignment for page-based operations */
    if (start_addr % PAGE_SIZE != 0) {
        log_memory_security_event("MISALIGNED_ACCESS", "Misaligned memory access", address);
        return false;
    }
    
    /* Validate against registered regions (permissions + owner) */
    user_t* current_user = security_get_current_user();
    for (uint32_t i = 0; i < region_count; i++) {
        memory_region_t* region = &memory_regions[i];
        
        if (start_addr >= (uint32_t)region->base_address && 
            end_addr <= (uint32_t)region->base_address + region->size) {
            
            /* Check access permissions */
            if (!(region->protection & access_type)) {
                log_memory_security_event("PERMISSION_DENIED", "Insufficient permissions for memory access", address);
                return false;
            }
            
            /* Check ownership */
            if (region->owner && region->owner != current_user) {
                log_memory_security_event("WRONG_OWNER", "Memory access by wrong user", address);
                return false;
            }
            
            return true; /* Valid access */
        }
    }
    
    log_memory_security_event("UNREGISTERED_REGION", "Access to unregistered memory region", address);
    return false;
}

/* Register memory region for access control */
static bool register_memory_region(void* address, size_t size, memory_protection_t protection, user_t* owner) {
    /* Track an allocated region to enforce access checks later. */
    if (region_count >= MAX_MEMORY_REGIONS) {
        return false;
    }
    
    memory_regions[region_count].base_address = address;
    memory_regions[region_count].size = size;
    memory_regions[region_count].protection = protection;
    memory_regions[region_count].owner = owner;
    memory_regions[region_count].is_allocated = true;
    
    region_count++;
    return true;
}

/* Unregister memory region */
static bool unregister_memory_region(const void* address) {
    /* Remove region tracking when memory is freed. Compact array by shifting. */
    for (uint32_t i = 0; i < region_count; i++) {
        if (memory_regions[i].base_address == address) {
            /* Shift remaining regions */
            for (uint32_t j = i; j < region_count - 1; j++) {
                memory_regions[j] = memory_regions[j + 1];
            }
            region_count--;
            return true;
        }
    }
    return false;
}

/* Log memory security events */
static void log_memory_security_event(const char* event, const char* details, const void* address) {
    /* Compose a detail string including the address and forward to security log. */
    user_t* current_user = security_get_current_user();
    char full_details[256];
    
    /* Build detailed message with address */
    char addr_str[32];
    uint32_t addr_val = (uint32_t)address;
    int i = 0;
    
    /* Convert address to hex string */
    do {
        uint8_t nibble = addr_val & 0xF;
        addr_str[i++] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
        addr_val >>= 4;
    } while (addr_val > 0 && i < 8);
    
    /* Reverse string */
    for (int j = 0; j < i / 2; j++) {
        char temp = addr_str[j];
        addr_str[j] = addr_str[i - 1 - j];
        addr_str[i - 1 - j] = temp;
    }
    addr_str[i] = '\0';
    
    /* Build full details */
    char* ptr = full_details;
    const char* prefix = details;
    while (*prefix) *ptr++ = *prefix++;
    const char* addr_prefix = " at 0x";
    while (*addr_prefix) *ptr++ = *addr_prefix++;
    char* addr_ptr = addr_str;
    while (*addr_ptr) *ptr++ = *addr_ptr++;
    *ptr = '\0';
    
    security_log_security_violation(event, full_details, current_user);
}

/* set bit in bitmap */
static inline void bitmap_set(int bit) {
    /* Mark page frame as used */
    page_bitmap[bit >> 3] |= (1 << (bit & 7));
}

/* clear bit in bitmap */
static inline void bitmap_clear(int bit) {
    /* Mark page frame as free */
    page_bitmap[bit >> 3] &= ~(1 << (bit & 7));
}

/* test bit in bitmap */
static inline int bitmap_test(int bit) {
    /* Query page frame allocation state */
    return page_bitmap[bit >> 3] & (1 << (bit & 7));
}

/* find first free page frame */
static uint32_t find_free_page(void) {
    /* Linear scan from last hint; returns page index or -1 if none. */
    for (uint32_t i = next_free_page; i < (PHYS_MEMORY_END / PAGE_SIZE); ++i) {
        if (!bitmap_test(i)) {
            next_free_page = i + 1;
            return i;
        }
    }
    return (uint32_t)-1; /* out of memory */
}

/* Enhanced identity map a 4 KiB page with security checks */
static void map_page(uint32_t phys_addr, uint32_t virt_addr) {
    /* Simplified identity mapping: mark the physical page as used.
       In a full implementation, this would populate page tables. */
    /* Security validation */
    if (!validate_memory_access((const void*)phys_addr, PAGE_SIZE, MEM_PROT_WRITE)) {
        return; /* Invalid access */
    }
    
    /* simplified: assumes page tables already exist and identity map 0-4 MiB */
    /* for now we just mark the physical page as used */
    uint32_t frame = phys_addr / PAGE_SIZE;
    if (frame < (PHYS_MEMORY_END / PAGE_SIZE)) {
        bitmap_set(frame);
    }
}

/* Enhanced initialize memory management with security */
void init_memory_management(void) {
    /* Initialize allocator state and enable protection. Kernel region is registered
       as readable/writable/executable to reflect code/data in low memory. */
    /* zero bitmap */
    for (int i = 0; i < BITMAP_SIZE; ++i) {
        page_bitmap[i] = 0;
    }
    
    /* zero memory regions */
    for (int i = 0; i < MAX_MEMORY_REGIONS; i++) {
        memory_regions[i].base_address = NULL;
        memory_regions[i].size = 0;
        memory_regions[i].protection = MEM_PROT_NONE;
        memory_regions[i].owner = NULL;
        memory_regions[i].is_allocated = false;
    }
    region_count = 0;
    
    /* mark kernel pages (0 - KERNEL_END) as used */
    uint32_t kernel_pages = (KERNEL_END + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t p = 0; p < kernel_pages; ++p) {
        bitmap_set(p);
    }
    next_free_page = kernel_pages;
    
    /* Register kernel memory region */
    register_memory_region((void*)0, KERNEL_END, MEM_PROT_READ | MEM_PROT_WRITE | MEM_PROT_EXECUTE, NULL);
    
    /* Enable memory protection after initialization */
    memory_protection_enabled = true;
}

/* Enhanced allocate one 4 KiB page with security checks */
void* allocate_memory(size_t size) {
    sc_lock_acquire(&mm_lock);
    user_t* current_user = security_get_current_user();
    if (!current_user) {
        log_memory_security_event("NO_USER", "Memory allocation attempted without authenticated user", NULL);
        sc_lock_release(&mm_lock);
        return NULL;
    }
    
    if (size != PAGE_SIZE) {
        log_memory_security_event("INVALID_SIZE", "Invalid memory allocation size", NULL);
        sc_lock_release(&mm_lock);
        return NULL;
    }
    
    uint32_t frame = find_free_page();
    if (frame == (uint32_t)-1) {
        log_memory_security_event("OUT_OF_MEMORY", "No free pages available", NULL);
        sc_lock_release(&mm_lock);
        return NULL;
    }
    
    bitmap_set(frame);
    void* allocated_address = (void*)(frame * PAGE_SIZE);
    
    /* Register the allocated region */
    if (!register_memory_region(allocated_address, PAGE_SIZE, MEM_PROT_READ | MEM_PROT_WRITE, current_user)) {
        log_memory_security_event("REGION_REGISTRATION_FAILED", "Failed to register memory region", allocated_address);
        bitmap_clear(frame);
        sc_lock_release(&mm_lock);
        return NULL;
    }
    
    log_memory_security_event("MEMORY_ALLOCATED", "Memory page allocated successfully", allocated_address);
    sc_lock_release(&mm_lock);
    return allocated_address;
}

/* Enhanced free a previously allocated page with security checks */
void free_memory(void* ptr) {
    sc_lock_acquire(&mm_lock);
    if (!ptr) {
        log_memory_security_event("NULL_POINTER_FREE", "Attempted to free null pointer", NULL);
        sc_lock_release(&mm_lock);
        return;
    }
    
    /* Validate memory access before freeing */
    if (!validate_memory_access(ptr, PAGE_SIZE, MEM_PROT_WRITE)) {
        log_memory_security_event("INVALID_FREE", "Invalid memory access during free", ptr);
        sc_lock_release(&mm_lock);
        return;
    }
    
    /* Check user ownership */
    user_t* current_user = security_get_current_user();
    if (!current_user) {
        log_memory_security_event("NO_USER_FREE", "Memory free attempted without authenticated user", ptr);
        sc_lock_release(&mm_lock);
        return;
    }
    
    uint32_t frame = (uint32_t)ptr / PAGE_SIZE;
    if (frame < (PHYS_MEMORY_END / PAGE_SIZE)) {
        bitmap_clear(frame);
        if (frame < next_free_page) next_free_page = frame;
        
        /* Unregister the memory region */
        unregister_memory_region(ptr);
        
        log_memory_security_event("MEMORY_FREED", "Memory page freed successfully", ptr);
        sc_lock_release(&mm_lock);
    } else {
        log_memory_security_event("INVALID_FRAME", "Invalid page frame during free", ptr);
        sc_lock_release(&mm_lock);
    }
}