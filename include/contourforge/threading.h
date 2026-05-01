/**
 * @file threading.h
 * @brief Contourforge多线程支持API
 * @version 0.2.0
 */

#ifndef CF_THREADING_H
#define CF_THREADING_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 线程配置 ========== */

/**
 * @brief 线程配置
 */
typedef struct {
    int num_threads;        /**< 线程数量（0=自动检测CPU核心数） */
    bool enable_threading;  /**< 是否启用多线程 */
    int task_queue_size;    /**< 任务队列大小 */
} cf_thread_config_t;

/**
 * @brief 获取默认线程配置
 * @return 默认配置
 */
cf_thread_config_t cf_thread_config_default(void);

/* ========== 线程池 ========== */

/**
 * @brief 线程池句柄
 */
typedef struct cf_thread_pool cf_thread_pool_t;

/**
 * @brief 任务函数类型
 * @param arg 任务参数
 */
typedef void (*cf_task_func_t)(void* arg);

/**
 * @brief 创建线程池
 * @param config 线程配置
 * @param pool 输出线程池指针
 * @return 返回码
 */
cf_result_t cf_thread_pool_create(
    const cf_thread_config_t* config,
    cf_thread_pool_t** pool
);

/**
 * @brief 提交任务到线程池
 * @param pool 线程池
 * @param func 任务函数
 * @param arg 任务参数
 * @return 返回码
 */
cf_result_t cf_thread_pool_submit(
    cf_thread_pool_t* pool,
    cf_task_func_t func,
    void* arg
);

/**
 * @brief 等待所有任务完成
 * @param pool 线程池
 */
void cf_thread_pool_wait(cf_thread_pool_t* pool);

/**
 * @brief 获取线程池统计信息
 * @param pool 线程池
 * @param active_threads 输出活跃线程数
 * @param pending_tasks 输出待处理任务数
 */
void cf_thread_pool_get_stats(
    const cf_thread_pool_t* pool,
    int* active_threads,
    int* pending_tasks
);

/**
 * @brief 销毁线程池
 * @param pool 线程池
 */
void cf_thread_pool_destroy(cf_thread_pool_t* pool);

/* ========== 互斥锁 ========== */

/**
 * @brief 互斥锁句柄
 */
typedef struct cf_mutex cf_mutex_t;

/**
 * @brief 创建互斥锁
 * @param mutex 输出互斥锁指针
 * @return 返回码
 */
cf_result_t cf_mutex_create(cf_mutex_t** mutex);

/**
 * @brief 锁定互斥锁
 * @param mutex 互斥锁
 */
void cf_mutex_lock(cf_mutex_t* mutex);

/**
 * @brief 解锁互斥锁
 * @param mutex 互斥锁
 */
void cf_mutex_unlock(cf_mutex_t* mutex);

/**
 * @brief 销毁互斥锁
 * @param mutex 互斥锁
 */
void cf_mutex_destroy(cf_mutex_t* mutex);

/* ========== 原子操作 ========== */

/**
 * @brief 原子整数类型
 */
typedef struct {
    volatile int value;
} cf_atomic_int_t;

/**
 * @brief 原子加法
 * @param atomic 原子整数
 * @param value 增加值
 * @return 操作前的值
 */
int cf_atomic_add(cf_atomic_int_t* atomic, int value);

/**
 * @brief 原子比较并交换
 * @param atomic 原子整数
 * @param expected 期望值
 * @param desired 目标值
 * @return 是否成功
 */
bool cf_atomic_compare_exchange(
    cf_atomic_int_t* atomic,
    int expected,
    int desired
);

#ifdef __cplusplus
}
#endif

#endif /* CF_THREADING_H */
