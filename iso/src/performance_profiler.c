/* performance_profiler.c - Performance profiling implementation */

#include "performance_profiler.h"
#include <string.h>

/* Global profiler session */
profiler_session_t g_profiler_session = {0};

/* Memory and I/O profiling statistics */
static struct {
    uint64_t total_allocations;
    uint64_t total_deallocations;
    uint64_t total_allocated_bytes;
    uint64_t total_deallocated_bytes;
    uint64_t peak_memory_usage;
    uint64_t current_memory_usage;
} memory_stats = {0};

static struct {
    uint64_t total_io_operations;
    uint64_t total_io_bytes;
    uint64_t total_io_time_ns;
    uint64_t read_operations;
    uint64_t write_operations;
    uint64_t read_bytes;
    uint64_t write_bytes;
} io_stats = {0};

/* Initialize the profiler */
void profiler_init(void) {
    memset(&g_profiler_session, 0, sizeof(profiler_session_t));
    memset(&memory_stats, 0, sizeof(memory_stats));
    memset(&io_stats, 0, sizeof(io_stats));
    g_profiler_session.session_start_time = profiler_get_current_time_ns();
    g_profiler_session.profiling_enabled = 1;
}

/* Enable profiling */
void profiler_enable(void) {
    g_profiler_session.profiling_enabled = 1;
}

/* Disable profiling */
void profiler_disable(void) {
    g_profiler_session.profiling_enabled = 0;
}

/* Reset profiler data */
void profiler_reset(void) {
    profiler_init();
}

/* Get current time in nanoseconds using CPU timestamp counter */
uint64_t profiler_get_current_time_ns(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Get CPU cycles */
uint64_t profiler_get_cpu_cycles(void) {
    return profiler_get_current_time_ns();
}

/* Register a function for profiling */
uint32_t profiler_register_function(const char* function_name) {
    if (g_profiler_session.function_count >= MAX_PROFILE_FUNCTIONS) {
        return (uint32_t)-1;
    }
    
    uint32_t id = g_profiler_session.function_count++;
    performance_metric_t* metric = &g_profiler_session.metrics[id];
    
    metric->function_name = function_name;
    metric->function_id = id;
    metric->total_calls = 0;
    metric->total_time_ns = 0;
    metric->min_time_ns = UINT64_MAX;
    metric->max_time_ns = 0;
    metric->avg_time_ns = 0;
    
    return id;
}

/* Start timing a function */
void profiler_start_function(uint32_t function_id) {
    if (!g_profiler_session.profiling_enabled || function_id >= g_profiler_session.function_count) {
        return;
    }
    
    performance_metric_t* metric = &g_profiler_session.metrics[function_id];
    uint64_t current_time = profiler_get_current_time_ns();
    
    /* Store start time in a simple way - we'll calculate duration on end */
    metric->total_time_ns = current_time; /* Temporary storage */
}

/* End timing a function */
void profiler_end_function(uint32_t function_id) {
    if (!g_profiler_session.profiling_enabled || function_id >= g_profiler_session.function_count) {
        return;
    }
    
    performance_metric_t* metric = &g_profiler_session.metrics[function_id];
    uint64_t current_time = profiler_get_current_time_ns();
    uint64_t start_time = metric->total_time_ns; /* Retrieve start time */
    uint64_t duration = current_time - start_time;
    
    /* Update statistics */
    metric->total_calls++;
    metric->total_time_ns += duration;
    
    if (duration < metric->min_time_ns) {
        metric->min_time_ns = duration;
    }
    if (duration > metric->max_time_ns) {
        metric->max_time_ns = duration;
    }
    
    /* Calculate average */
    if (metric->total_calls > 0) {
        metric->avg_time_ns = metric->total_time_ns / metric->total_calls;
    }
}

/* Record memory allocation */
void profiler_record_memory_allocation(uint32_t size, uint32_t count) {
    memory_stats.total_allocations += count;
    memory_stats.total_allocated_bytes += size * count;
    memory_stats.current_memory_usage += size * count;
    
    if (memory_stats.current_memory_usage > memory_stats.peak_memory_usage) {
        memory_stats.peak_memory_usage = memory_stats.current_memory_usage;
    }
}

/* Record memory deallocation */
void profiler_record_memory_deallocation(uint32_t size, uint32_t count) {
    memory_stats.total_deallocations += count;
    memory_stats.total_deallocated_bytes += size * count;
    memory_stats.current_memory_usage -= size * count;
}

/* Record I/O operation */
void profiler_record_io_operation(const char* operation, uint32_t bytes, uint64_t time_ns) {
    io_stats.total_io_operations++;
    io_stats.total_io_bytes += bytes;
    io_stats.total_io_time_ns += time_ns;
    
    if (operation && operation[0] == 'r') {
        io_stats.read_operations++;
        io_stats.read_bytes += bytes;
    } else if (operation && operation[0] == 'w') {
        io_stats.write_operations++;
        io_stats.write_bytes += bytes;
    }
}

/* Print profiling report */
void profiler_print_report(void) {
    /* Simple print function for now - will integrate with kernel print later */
    /* This would typically use the kernel's print function */
}

/* Print top functions by execution time */
void profiler_print_top_functions(uint32_t count) {
    /* Sort functions by total execution time and print top N */
    /* Implementation would go here */
}

/* Print memory statistics */
void profiler_print_memory_stats(void) {
    /* Print memory profiling statistics */
    /* Implementation would go here */
}

/* Print I/O statistics */
void profiler_print_io_stats(void) {
    /* Print I/O profiling statistics */
    /* Implementation would go here */
}