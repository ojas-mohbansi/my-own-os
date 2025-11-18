/* memory_management_optimized.c - Optimized memory management with performance profiling */

#include <stddef.h>
#include <stdint.h>
#include "performance_profiler.h"

#define PAGE_SIZE        4096
#define KERNEL_END       0x100000     /* 1 MiB, adjust as needed */
#define PHYS_MEMORY_END  0x1000000    /* 16 MiB for now */
#define BITMAP_SIZE      (PHYS_MEMORY_END / PAGE_SIZE / 8)
#define CACHE_LINE_SIZE  64
#define PREFETCH_DISTANCE 8

static uint8_t page_bitmap[BITMAP_SIZE] __attribute__((aligned(PAGE_SIZE)));
static uint32_t next_free_page = 0;

/* Performance optimization: Use bit scan operations for faster free page finding */
static inline int find_first_zero_bit(uint8_t byte) {
    if (byte == 0xFF) return -1;
    
    /* Use built-in bit scan reverse for faster operation */
    return __builtin_ctz(~byte);
}

/* Optimized bitmap operations with prefetching */
static inline void bitmap_set(int bit) {
    page_bitmap[bit >> 3] |= (1 << (bit & 7));
}

static inline void bitmap_clear(int bit) {
    page_bitmap[bit >> 3] &= ~(1 << (bit & 7));
}

static inline int bitmap_test(int bit) {
    return page_bitmap[bit >> 3] & (1 << (bit & 7));
}

/* Optimized find free page with bit scan operations and prefetching */
static uint32_t optimized_find_free_page(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_find_free_page");
    profiler_start_function(__profile_id);
    
    register uint32_t start_page = next_free_page;
    register uint32_t total_pages = PHYS_MEMORY_END / PAGE_SIZE;
    
    /* Prefetch bitmap data for better cache performance */
    for (register uint32_t i = start_page >> 3; i < (start_page >> 3) + PREFETCH_DISTANCE && i < BITMAP_SIZE; i++) {
        PREFETCH(&page_bitmap[i]);
    }
    
    /* Search in chunks of 8 bytes (64 pages) for better cache utilization */
    register uint32_t byte_index = start_page >> 3;
    register uint32_t bit_index = start_page & 7;
    
    for (; byte_index < BITMAP_SIZE; byte_index++) {
        register uint8_t current_byte = page_bitmap[byte_index];
        
        if (LIKELY(current_byte != 0xFF)) {
            /* Found a byte with free bits */
            register int free_bit = find_first_zero_bit(current_byte);
            if (free_bit >= 0) {
                register uint32_t result = (byte_index << 3) + free_bit;
                if (LIKELY(result < total_pages)) {
                    next_free_page = result + 1;
                    profiler_end_function(__profile_id);
                    return result;
                }
            }
        }
        
        /* Prefetch next chunk */
        if (byte_index + PREFETCH_DISTANCE < BITMAP_SIZE) {
            PREFETCH(&page_bitmap[byte_index + PREFETCH_DISTANCE]);
        }
    }
    
    /* Wrap around and search from beginning */
    for (byte_index = 0; byte_index < (start_page >> 3); byte_index++) {
        register uint8_t current_byte = page_bitmap[byte_index];
        
        if (LIKELY(current_byte != 0xFF)) {
            register int free_bit = find_first_zero_bit(current_byte);
            if (free_bit >= 0) {
                register uint32_t result = (byte_index << 3) + free_bit;
                if (LIKELY(result < total_pages)) {
                    next_free_page = result + 1;
                    profiler_end_function(__profile_id);
                    return result;
                }
            }
        }
    }
    
    profiler_end_function(__profile_id);
    return (uint32_t)-1; /* out of memory */
}

/* Optimized bulk bitmap operations */
static inline void bitmap_set_range(uint32_t start_bit, uint32_t count) {
    register uint32_t start_byte = start_bit >> 3;
    register uint32_t start_bit_in_byte = start_bit & 7;
    register uint32_t end_bit = start_bit + count;
    register uint32_t end_byte = end_bit >> 3;
    register uint32_t end_bit_in_byte = end_bit & 7;
    
    /* Handle first byte with partial bits */
    if (start_bit_in_byte != 0) {
        register uint8_t mask = (0xFF << start_bit_in_byte);
        if (start_byte == end_byte) {
            mask &= (0xFF >> (8 - end_bit_in_byte));
        }
        page_bitmap[start_byte] |= mask;
        start_byte++;
    }
    
    /* Handle full bytes */
    for (register uint32_t i = start_byte; i < end_byte; i++) {
        page_bitmap[i] = 0xFF;
    }
    
    /* Handle last byte with partial bits */
    if (end_byte > start_byte && end_bit_in_byte != 0) {
        register uint8_t mask = (0xFF >> (8 - end_bit_in_byte));
        page_bitmap[end_byte] |= mask;
    }
}

/* Optimized identity map with bulk operations */
static inline void optimized_map_page(uint32_t phys_addr, uint32_t virt_addr) {
    register uint32_t frame = phys_addr / PAGE_SIZE;
    register uint32_t total_frames = PHYS_MEMORY_END / PAGE_SIZE;
    
    if (LIKELY(frame < total_frames)) {
        bitmap_set(frame);
    }
}

/* Optimized memory initialization with bulk operations */
void optimized_init_memory_management(void) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_init_memory_management");
    profiler_start_function(__profile_id);
    
    /* Use bulk memset for faster initialization */
    register uint32_t kernel_pages = (KERNEL_END + PAGE_SIZE - 1) / PAGE_SIZE;
    
    /* Clear bitmap efficiently */
    register uint64_t* bitmap_64 = (uint64_t*)page_bitmap;
    register size_t bitmap_64_size = BITMAP_SIZE / sizeof(uint64_t);
    
    for (register size_t i = 0; i < bitmap_64_size; i++) {
        bitmap_64[i] = 0;
    }
    
    /* Handle remaining bytes */
    for (register size_t i = bitmap_64_size * sizeof(uint64_t); i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0;
    }
    
    /* Mark kernel pages as used using bulk operations */
    bitmap_set_range(0, kernel_pages);
    
    next_free_page = kernel_pages;
    
    profiler_record_memory_allocation(kernel_pages * PAGE_SIZE, 1);
    profiler_end_function(__profile_id);
}

/* Optimized memory allocation with fast path */
void* optimized_allocate_memory(size_t size) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_allocate_memory");
    profiler_start_function(__profile_id);
    
    if (UNLIKELY(size != PAGE_SIZE)) {
        profiler_end_function(__profile_id);
        return NULL; /* only single pages for now */
    }
    
    register uint32_t frame = optimized_find_free_page();
    if (UNLIKELY(frame == (uint32_t)-1)) {
        profiler_end_function(__profile_id);
        return NULL;
    }
    
    bitmap_set(frame);
    
    profiler_record_memory_allocation(PAGE_SIZE, 1);
    profiler_end_function(__profile_id);
    
    return (void*)(frame * PAGE_SIZE);
}

/* Optimized memory deallocation */
void optimized_free_memory(void* ptr) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_free_memory");
    profiler_start_function(__profile_id);
    
    if (UNLIKELY(!ptr)) {
        profiler_end_function(__profile_id);
        return;
    }
    
    register uint32_t frame = (uint32_t)ptr / PAGE_SIZE;
    register uint32_t total_frames = PHYS_MEMORY_END / PAGE_SIZE;
    
    if (LIKELY(frame < total_frames)) {
        bitmap_clear(frame);
        if (frame < next_free_page) {
            next_free_page = frame;
        }
    }
    
    profiler_record_memory_deallocation(PAGE_SIZE, 1);
    profiler_end_function(__profile_id);
}

/* Memory pool for small allocations to reduce fragmentation */
#define SMALL_ALLOC_POOL_SIZE 16384  /* 16KB pool for small allocations */
#define SMALL_ALLOC_MAX_SIZE 256     /* Maximum size for small allocations */

typedef struct small_alloc_block {
    uint32_t size;
    uint32_t used;
    struct small_alloc_block* next;
} small_alloc_block_t;

static uint8_t small_alloc_pool[SMALL_ALLOC_POOL_SIZE] __attribute__((aligned(16)));
static small_alloc_block_t* small_alloc_list = NULL;
static uint8_t small_alloc_initialized = 0;

/* Initialize small allocation pool */
static void init_small_alloc_pool(void) {
    if (small_alloc_initialized) return;
    
    small_alloc_block_t* initial_block = (small_alloc_block_t*)small_alloc_pool;
    initial_block->size = SMALL_ALLOC_POOL_SIZE - sizeof(small_alloc_block_t);
    initial_block->used = 0;
    initial_block->next = NULL;
    
    small_alloc_list = initial_block;
    small_alloc_initialized = 1;
}

/* Optimized small memory allocation */
void* optimized_allocate_small_memory(size_t size) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_allocate_small_memory");
    profiler_start_function(__profile_id);
    
    if (UNLIKELY(size > SMALL_ALLOC_MAX_SIZE)) {
        profiler_end_function(__profile_id);
        return optimized_allocate_memory(PAGE_SIZE); /* Fall back to page allocation */
    }
    
    /* Align size to 8-byte boundary */
    size = (size + 7) & ~7;
    
    init_small_alloc_pool();
    
    /* First-fit allocation strategy */
    small_alloc_block_t* current = small_alloc_list;
    small_alloc_block_t* prev = NULL;
    
    while (current) {
        if (!current->used && current->size >= size) {
            /* Found suitable block */
            if (current->size > size + sizeof(small_alloc_block_t) + 16) {
                /* Split the block */
                small_alloc_block_t* new_block = (small_alloc_block_t*)((uint8_t*)current + sizeof(small_alloc_block_t) + size);
                new_block->size = current->size - size - sizeof(small_alloc_block_t);
                new_block->used = 0;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->used = 1;
            profiler_record_memory_allocation(size, 1);
            profiler_end_function(__profile_id);
            return (void*)((uint8_t*)current + sizeof(small_alloc_block_t));
        }
        
        prev = current;
        current = current->next;
    }
    
    profiler_end_function(__profile_id);
    return NULL; /* No suitable block found */
}

/* Optimized small memory deallocation */
void optimized_free_small_memory(void* ptr) {
    uint32_t __profile_id = 0;
    if (__profile_id == 0) __profile_id = profiler_register_function("optimized_free_small_memory");
    profiler_start_function(__profile_id);
    
    if (UNLIKELY(!ptr)) {
        profiler_end_function(__profile_id);
        return;
    }
    
    small_alloc_block_t* block = (small_alloc_block_t*)((uint8_t*)ptr - sizeof(small_alloc_block_t));
    
    /* Validate block is within our pool */
    if ((uint8_t*)block < small_alloc_pool || 
        (uint8_t*)block >= small_alloc_pool + SMALL_ALLOC_POOL_SIZE) {
        /* Not our block, ignore */
        profiler_end_function(__profile_id);
        return;
    }
    
    block->used = 0;
    
    /* Try to coalesce with next block */
    if (block->next && !block->next->used) {
        block->size += sizeof(small_alloc_block_t) + block->next->size;
        block->next = block->next->next;
    }
    
    profiler_record_memory_deallocation(block->size, 1);
    profiler_end_function(__profile_id);
}