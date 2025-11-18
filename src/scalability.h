#ifndef SCALABILITY_H
#define SCALABILITY_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    THREAD_READY = 0,
    THREAD_RUNNING = 1,
    THREAD_BLOCKED = 2,
    THREAD_DONE = 3
} thread_state_t;

typedef void (*thread_fn)(void*);

#define SC_MAX_THREADS 64
#define SC_MAX_CPUS 8

typedef struct {
    int id;
    int cpu_id;
    int priority;
    uint32_t quota;
    thread_fn entry;
    void* arg;
    thread_state_t state;
    uint64_t ticks;
} sc_thread_t;

static sc_thread_t sc_threads[SC_MAX_THREADS];
static int sc_thread_count = 0;
static int sc_run_queue[SC_MAX_THREADS];
static int sc_rq_head = 0;
static int sc_rq_tail = 0;
static int sc_cpu_load[SC_MAX_CPUS];
static int sc_cpu_count = 1;
static int sc_current_thread = -1;

typedef volatile int sc_lock_t;

static void sc_lock_acquire(sc_lock_t* lock) {
    while (*lock) {}
    *lock = 1;
}

static void sc_lock_release(sc_lock_t* lock) {
    *lock = 0;
}

static void init_scheduler(int cpus) {
    if (cpus < 1) cpus = 1;
    if (cpus > SC_MAX_CPUS) cpus = SC_MAX_CPUS;
    sc_cpu_count = cpus;
    for (int i = 0; i < SC_MAX_CPUS; i++) sc_cpu_load[i] = 0;
    sc_thread_count = 0;
    sc_rq_head = 0;
    sc_rq_tail = 0;
    sc_current_thread = -1;
}

static int get_thread_count(void) {
    int count = 0;
    for (int i = 0; i < sc_thread_count; i++) {
        if (sc_threads[i].state != THREAD_DONE) count++;
    }
    return count;
}

static int get_cpu_load(int cpu_id) {
    if (cpu_id < 0 || cpu_id >= sc_cpu_count) return 0;
    return sc_cpu_load[cpu_id];
}

static int current_thread_id(void) {
    return sc_current_thread;
}

static int push_rq(int id) {
    int next = (sc_rq_tail + 1) % SC_MAX_THREADS;
    if (next == sc_rq_head) return -1;
    sc_run_queue[sc_rq_tail] = id;
    sc_rq_tail = next;
    return 0;
}

static int pop_rq(void) {
    if (sc_rq_head == sc_rq_tail) return -1;
    int id = sc_run_queue[sc_rq_head];
    sc_rq_head = (sc_rq_head + 1) % SC_MAX_THREADS;
    return id;
}

static int create_thread(thread_fn entry, void* arg, int priority) {
    if (!entry) return -1;
    if (sc_thread_count >= SC_MAX_THREADS) return -1;
    int best_cpu = 0;
    for (int i = 1; i < sc_cpu_count; i++) {
        if (sc_cpu_load[i] < sc_cpu_load[best_cpu]) best_cpu = i;
    }
    int id = sc_thread_count;
    sc_threads[id].id = id;
    sc_threads[id].cpu_id = best_cpu;
    sc_threads[id].priority = priority;
    sc_threads[id].quota = 1;
    sc_threads[id].entry = entry;
    sc_threads[id].arg = arg;
    sc_threads[id].state = THREAD_READY;
    sc_threads[id].ticks = 0;
    sc_thread_count++;
    sc_cpu_load[best_cpu]++;
    push_rq(id);
    return id;
}

static void yield(void) {
    if (sc_current_thread < 0) return;
    int id = sc_current_thread;
    sc_threads[id].state = THREAD_READY;
    push_rq(id);
    sc_current_thread = -1;
}

static void complete_current_thread(void) {
    if (sc_current_thread < 0) return;
    int id = sc_current_thread;
    sc_threads[id].state = THREAD_DONE;
    sc_cpu_load[sc_threads[id].cpu_id]--;
    sc_current_thread = -1;
}

static void load_balance(void) {
    int min_cpu = 0, max_cpu = 0;
    for (int i = 1; i < sc_cpu_count; i++) {
        if (sc_cpu_load[i] < sc_cpu_load[min_cpu]) min_cpu = i;
        if (sc_cpu_load[i] > sc_cpu_load[max_cpu]) max_cpu = i;
    }
    if (sc_cpu_load[max_cpu] - sc_cpu_load[min_cpu] <= 1) return;
    for (int i = 0; i < sc_thread_count; i++) {
        if (sc_threads[i].state == THREAD_READY && sc_threads[i].cpu_id == max_cpu) {
            sc_threads[i].cpu_id = min_cpu;
            sc_cpu_load[max_cpu]--;
            sc_cpu_load[min_cpu]++;
            break;
        }
    }
}

static void schedule_process(void) {
    int id = pop_rq();
    if (id < 0) return;
    if (sc_threads[id].state != THREAD_READY) {
        push_rq(id);
        return;
    }
    sc_threads[id].state = THREAD_RUNNING;
    sc_current_thread = id;
    sc_threads[id].entry(sc_threads[id].arg);
    if (sc_current_thread == id) {
        sc_threads[id].ticks++;
        sc_threads[id].state = THREAD_READY;
        push_rq(id);
        sc_current_thread = -1;
    }
}

#endif