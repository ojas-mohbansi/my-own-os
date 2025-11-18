#include "unity.h"
#include "test_config.h"
#include "../src/scalability.h"

static int steps[SC_MAX_THREADS];
static int targets[SC_MAX_THREADS];

static void worker(void* arg) {
    int tid = current_thread_id();
    steps[tid]++;
    if (steps[tid] >= targets[tid]) {
        complete_current_thread();
        return;
    }
    yield();
}

static void setUp(void) {}
static void tearDown(void) {}

static void run_scheduler_until_done(void) {
    int guard = 0;
    while (get_thread_count() > 0 && guard < 100000) {
        schedule_process();
        if ((guard % 32) == 0) load_balance();
        guard++;
    }
}

static void test_concurrency_12_threads(void) {
    init_scheduler(4);
    for (int i = 0; i < 12; i++) {
        steps[i] = 0;
        targets[i] = 100;
        create_thread(worker, NULL, 1);
    }
    run_scheduler_until_done();
    for (int i = 0; i < 12; i++) {
        TEST_ASSERT_EQUAL_INT(targets[i], steps[i]);
    }
    TEST_ASSERT_EQUAL_INT(0, get_thread_count());
}

static void test_load_balancing_distribution(void) {
    init_scheduler(4);
    for (int i = 0; i < 16; i++) {
        steps[i] = 0;
        targets[i] = 10;
        create_thread(worker, NULL, 1);
    }
    for (int i = 0; i < 64; i++) load_balance();
    int min = get_cpu_load(0), max = get_cpu_load(0);
    for (int c = 1; c < 4; c++) {
        int l = get_cpu_load(c);
        if (l < min) min = l;
        if (l > max) max = l;
    }
    TEST_ASSERT_TRUE(max - min <= 1);
}

int run_scalability_tests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_concurrency_12_threads);
    RUN_TEST(test_load_balancing_distribution);
    return UNITY_END();
}