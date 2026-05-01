/**
 * @file test_threading.c
 * @brief 线程池单元测试
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n  %s\n", message, #condition); \
            return 0; \
        } \
    } while (0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return 1; \
    } while (0)

static void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#else
    usleep((useconds_t)milliseconds * 1000);
#endif
}

typedef struct {
    cf_atomic_int_t* counter;
    int value;
} counter_task_arg_t;

static void increment_task(void* arg) {
    counter_task_arg_t* task_arg = (counter_task_arg_t*)arg;
    cf_atomic_add(task_arg->counter, task_arg->value);
}

static void delayed_increment_task(void* arg) {
    counter_task_arg_t* task_arg = (counter_task_arg_t*)arg;
    sleep_ms(5);
    cf_atomic_add(task_arg->counter, task_arg->value);
}

static int test_thread_pool_create_destroy(void) {
    printf("\n测试: 线程池创建和销毁\n");

    cf_thread_config_t config = cf_thread_config_default();
    config.num_threads = 2;
    config.task_queue_size = 32;

    cf_thread_pool_t* pool = NULL;
    cf_result_t result = cf_thread_pool_create(&config, &pool);

    TEST_ASSERT(result == CF_SUCCESS, "线程池创建返回成功");
    TEST_ASSERT(pool != NULL, "线程池不为空");

    int active = -1;
    int pending = -1;
    cf_thread_pool_get_stats(pool, &active, &pending);
    TEST_ASSERT(active == 0, "新线程池无活跃线程");
    TEST_ASSERT(pending == 0, "新线程池无待处理任务");

    cf_thread_pool_destroy(pool);

    config.enable_threading = false;
    pool = (cf_thread_pool_t*)0x1;
    result = cf_thread_pool_create(&config, &pool);
    TEST_ASSERT(result == CF_SUCCESS, "禁用多线程时创建返回成功");
    TEST_ASSERT(pool == NULL, "禁用多线程时返回空线程池");

    TEST_PASS("线程池创建和销毁");
}

static int test_task_submit_and_execute(void) {
    printf("\n测试: 任务提交和执行\n");

    cf_thread_config_t config = cf_thread_config_default();
    config.num_threads = 2;
    config.task_queue_size = 128;

    cf_thread_pool_t* pool = NULL;
    TEST_ASSERT(cf_thread_pool_create(&config, &pool) == CF_SUCCESS, "线程池创建成功");

    cf_atomic_int_t counter = {0};
    counter_task_arg_t args[64];

    for (int i = 0; i < 64; ++i) {
        args[i].counter = &counter;
        args[i].value = 1;
        TEST_ASSERT(cf_thread_pool_submit(pool, increment_task, &args[i]) == CF_SUCCESS, "任务提交成功");
    }

    cf_thread_pool_wait(pool);
    TEST_ASSERT(counter.value == 64, "全部任务已执行");

    int active = -1;
    int pending = -1;
    cf_thread_pool_get_stats(pool, &active, &pending);
    TEST_ASSERT(active == 0, "等待后无活跃线程");
    TEST_ASSERT(pending == 0, "等待后无待处理任务");

    cf_thread_pool_destroy(pool);

    TEST_PASS("任务提交和执行");
}

static int test_concurrency_safety(void) {
    printf("\n测试: 并发安全性\n");

    cf_thread_config_t config = cf_thread_config_default();
    config.num_threads = 4;
    config.task_queue_size = 512;

    cf_thread_pool_t* pool = NULL;
    TEST_ASSERT(cf_thread_pool_create(&config, &pool) == CF_SUCCESS, "线程池创建成功");

    cf_atomic_int_t counter = {0};
    counter_task_arg_t* args = (counter_task_arg_t*)calloc(256, sizeof(counter_task_arg_t));
    TEST_ASSERT(args != NULL, "任务参数分配成功");

    for (int i = 0; i < 256; ++i) {
        args[i].counter = &counter;
        args[i].value = 1;
        TEST_ASSERT(cf_thread_pool_submit(pool, delayed_increment_task, &args[i]) == CF_SUCCESS, "并发任务提交成功");
    }

    cf_thread_pool_wait(pool);
    TEST_ASSERT(counter.value == 256, "原子计数器结果正确");

    int previous = cf_atomic_add(&counter, 10);
    TEST_ASSERT(previous == 256, "原子加法返回旧值");
    TEST_ASSERT(counter.value == 266, "原子加法更新成功");
    TEST_ASSERT(cf_atomic_compare_exchange(&counter, 266, 512), "原子CAS成功");
    TEST_ASSERT(counter.value == 512, "原子CAS更新成功");
    TEST_ASSERT(!cf_atomic_compare_exchange(&counter, 266, 1), "原子CAS失败路径正确");

    free(args);
    cf_thread_pool_destroy(pool);

    TEST_PASS("并发安全性");
}

static int test_thread_pool_performance(void) {
    printf("\n测试: 线程池性能\n");

    cf_thread_config_t config = cf_thread_config_default();
    config.num_threads = 4;
    config.task_queue_size = 2048;

    cf_thread_pool_t* pool = NULL;
    TEST_ASSERT(cf_thread_pool_create(&config, &pool) == CF_SUCCESS, "线程池创建成功");

    cf_atomic_int_t counter = {0};
    const int task_count = 1000;
    counter_task_arg_t* args = (counter_task_arg_t*)calloc((size_t)task_count, sizeof(counter_task_arg_t));
    TEST_ASSERT(args != NULL, "性能测试参数分配成功");

    clock_t start = clock();
    for (int i = 0; i < task_count; ++i) {
        args[i].counter = &counter;
        args[i].value = 1;
        TEST_ASSERT(cf_thread_pool_submit(pool, increment_task, &args[i]) == CF_SUCCESS, "性能测试任务提交成功");
    }
    cf_thread_pool_wait(pool);
    clock_t end = clock();

    double elapsed_ms = ((double)(end - start) * 1000.0) / (double)CLOCKS_PER_SEC;
    printf("  执行%d个轻量任务耗时: %.2f ms\n", task_count, elapsed_ms);

    TEST_ASSERT(counter.value == task_count, "性能测试全部任务完成");
    TEST_ASSERT(elapsed_ms < 5000.0, "性能测试在合理时间内完成");

    free(args);
    cf_thread_pool_destroy(pool);

    TEST_PASS("线程池性能");
}

int main(void) {
    printf("========================================\n");
    printf("Contourforge 线程池测试\n");
    printf("版本: %s\n", cf_get_version());
    printf("========================================\n");

    int passed = 0;
    int total = 0;

#define RUN_TEST(test_func) \
    do { \
        total++; \
        if (test_func()) passed++; \
    } while (0)

    RUN_TEST(test_thread_pool_create_destroy);
    RUN_TEST(test_task_submit_and_execute);
    RUN_TEST(test_concurrency_safety);
    RUN_TEST(test_thread_pool_performance);

    printf("\n========================================\n");
    printf("测试结果: %d/%d 通过\n", passed, total);
    printf("========================================\n");

    return (passed == total) ? 0 : 1;
}
