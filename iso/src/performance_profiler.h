/* performance_profiler.h - Performance profiling infrastructure for OS optimization */

#ifndef PERFORMANCE_PROFILER_H
#define PERFORMANCE_PROFILER_H

#include <stdint.h>
#include <stddef.h>

/* Maximum number of functions we can profile */
#define MAX_PROFILE_FUNCTIONS 64
#define MAX_FUNCTION_NAME_LENGTH 64

/* Performance metrics structure */
typedef struct {
    uint64_t total_calls;
    uint64_t total_time_ns;
    uint64_t min_time_ns;
    uint64_t max_time_ns;
    uint64_t avg_time_ns;
    const char* function_name;
    uint32_t function_id;
} performance_metric_t;

/* Profiling session structure */
typedef struct {
    performance_metric_t metrics[MAX_PROFILE_FUNCTIONS];
    uint32_t function_count;
    uint64_t session_start_time;
    uint64_t session_end_time;
    uint8_t profiling_enabled;
} profiler_session_t;

/* Timer structure for high-precision timing */
typedef struct {
    uint64_t start_time;
    uint64_t end_time;
    uint32_t function_id;
    const char* function_name;
    uint8_t active;
} function_timer_t;

/* Global profiler session */
extern profiler_session_t g_profiler_session;

/* Function prototypes */
void profiler_init(void);
void profiler_enable(void);
void profiler_disable(void);
void profiler_reset(void);
uint32_t profiler_register_function(const char* function_name);
void profiler_start_function(uint32_t function_id);
void profiler_end_function(uint32_t function_id);
void profiler_print_report(void);
void profiler_print_top_functions(uint32_t count);
uint64_t profiler_get_current_time_ns(void);
uint64_t profiler_get_cpu_cycles(void);

/* Memory profiling functions */
void profiler_record_memory_allocation(uint32_t size, uint32_t count);
void profiler_record_memory_deallocation(uint32_t size, uint32_t count);
void profiler_print_memory_stats(void);

/* I/O profiling functions */
void profiler_record_io_operation(const char* operation, uint32_t bytes, uint64_t time_ns);
void profiler_print_io_stats(void);

/* Macro for easy function profiling */
#define PROFILE_FUNCTION(name) \
    static uint32_t __profile_id = 0; \
    if (__profile_id == 0) __profile_id = profiler_register_function(name); \
    profiler_start_function(__profile_id); \
    { \
        /* Function body will be wrapped here */

#define PROFILE_FUNCTION_END \
    } \
    profiler_end_function(__profile_id);

/* Convenience macros for timing */
#define PROFILE_START(name) profiler_start_function(profiler_register_function(name))
#define PROFILE_END(name) profiler_end_function(profiler_register_function(name))

/* Performance optimization macros */
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#define PREFETCH(addr)  __builtin_prefetch(addr, 0, 3)

#endif /* PERFORMANCE_PROFILER_H */