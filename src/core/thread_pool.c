/**
 * @file thread_pool.c
 * @brief 线程池实现（跨平台）
 */

#include "contourforge/threading.h"
#include <stdlib.h>
#include <string.h>

/* ========== 平台相关头文件 ========== */

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
    #include <unistd.h>
#endif

/* ========== 跨平台线程抽象 ========== */

#ifdef _WIN32
    typedef HANDLE cf_thread_handle_t;
    typedef CRITICAL_SECTION cf_mutex_impl_t;
    typedef CONDITION_VARIABLE cf_cond_impl_t;
    
    #define CF_THREAD_FUNC_RETURN unsigned int
    #define CF_THREAD_FUNC_CALL __stdcall
#else
    typedef pthread_t cf_thread_handle_t;
    typedef pthread_mutex_t cf_mutex_impl_t;
    typedef pthread_cond_t cf_cond_impl_t;
    
    #define CF_THREAD_FUNC_RETURN void*
    #define CF_THREAD_FUNC_CALL
#endif

/* ========== 互斥锁实现 ========== */

struct cf_mutex {
    cf_mutex_impl_t impl;
};

cf_result_t cf_mutex_create(cf_mutex_t** mutex) {
    if (!mutex) return CF_ERROR_INVALID_PARAM;
    
    cf_mutex_t* m = (cf_mutex_t*)malloc(sizeof(cf_mutex_t));
    if (!m) return CF_ERROR_OUT_OF_MEMORY;
    
#ifdef _WIN32
    InitializeCriticalSection(&m->impl);
#else
    pthread_mutex_init(&m->impl, NULL);
#endif
    
    *mutex = m;
    return CF_SUCCESS;
}

void cf_mutex_lock(cf_mutex_t* mutex) {
    if (!mutex) return;
    
#ifdef _WIN32
    EnterCriticalSection(&mutex->impl);
#else
    pthread_mutex_lock(&mutex->impl);
#endif
}

void cf_mutex_unlock(cf_mutex_t* mutex) {
    if (!mutex) return;
    
#ifdef _WIN32
    LeaveCriticalSection(&mutex->impl);
#else
    pthread_mutex_unlock(&mutex->impl);
#endif
}

void cf_mutex_destroy(cf_mutex_t* mutex) {
    if (!mutex) return;
    
#ifdef _WIN32
    DeleteCriticalSection(&mutex->impl);
#else
    pthread_mutex_destroy(&mutex->impl);
#endif
    
    free(mutex);
}

/* ========== 条件变量 ========== */

typedef struct {
    cf_cond_impl_t impl;
} cf_cond_t;

static cf_result_t cf_cond_create(cf_cond_t** cond) {
    if (!cond) return CF_ERROR_INVALID_PARAM;
    
    cf_cond_t* c = (cf_cond_t*)malloc(sizeof(cf_cond_t));
    if (!c) return CF_ERROR_OUT_OF_MEMORY;
    
#ifdef _WIN32
    InitializeConditionVariable(&c->impl);
#else
    pthread_cond_init(&c->impl, NULL);
#endif
    
    *cond = c;
    return CF_SUCCESS;
}

static void cf_cond_wait(cf_cond_t* cond, cf_mutex_t* mutex) {
    if (!cond || !mutex) return;
    
#ifdef _WIN32
    SleepConditionVariableCS(&cond->impl, &mutex->impl, INFINITE);
#else
    pthread_cond_wait(&cond->impl, &mutex->impl);
#endif
}

static void cf_cond_signal(cf_cond_t* cond) {
    if (!cond) return;
    
#ifdef _WIN32
    WakeConditionVariable(&cond->impl);
#else
    pthread_cond_signal(&cond->impl);
#endif
}

static void cf_cond_broadcast(cf_cond_t* cond) {
    if (!cond) return;
    
#ifdef _WIN32
    WakeAllConditionVariable(&cond->impl);
#else
    pthread_cond_broadcast(&cond->impl);
#endif
}

static void cf_cond_destroy(cf_cond_t* cond) {
    if (!cond) return;
    
#ifdef _WIN32
    // Windows条件变量不需要显式销毁
#else
    pthread_cond_destroy(&cond->impl);
#endif
    
    free(cond);
}

/* ========== 原子操作 ========== */

int cf_atomic_add(cf_atomic_int_t* atomic, int value) {
    if (!atomic) return 0;
    
#ifdef _WIN32
    return InterlockedExchangeAdd((volatile LONG*)&atomic->value, value);
#else
    return __sync_fetch_and_add(&atomic->value, value);
#endif
}

bool cf_atomic_compare_exchange(cf_atomic_int_t* atomic, int expected, int desired) {
    if (!atomic) return false;
    
#ifdef _WIN32
    return InterlockedCompareExchange((volatile LONG*)&atomic->value, desired, expected) == expected;
#else
    return __sync_bool_compare_and_swap(&atomic->value, expected, desired);
#endif
}

/* ========== 任务队列 ========== */

typedef struct cf_task {
    cf_task_func_t func;
    void* arg;
    struct cf_task* next;
} cf_task_t;

typedef struct {
    cf_task_t* head;
    cf_task_t* tail;
    int count;
    int max_size;
} cf_task_queue_t;

static cf_result_t task_queue_create(int max_size, cf_task_queue_t** queue) {
    if (!queue) return CF_ERROR_INVALID_PARAM;
    
    cf_task_queue_t* q = (cf_task_queue_t*)malloc(sizeof(cf_task_queue_t));
    if (!q) return CF_ERROR_OUT_OF_MEMORY;
    
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->max_size = max_size;
    
    *queue = q;
    return CF_SUCCESS;
}

static bool task_queue_push(cf_task_queue_t* queue, cf_task_func_t func, void* arg) {
    if (!queue || queue->count >= queue->max_size) return false;
    
    cf_task_t* task = (cf_task_t*)malloc(sizeof(cf_task_t));
    if (!task) return false;
    
    task->func = func;
    task->arg = arg;
    task->next = NULL;
    
    if (queue->tail) {
        queue->tail->next = task;
    } else {
        queue->head = task;
    }
    queue->tail = task;
    queue->count++;
    
    return true;
}

static bool task_queue_pop(cf_task_queue_t* queue, cf_task_t* out_task) {
    if (!queue || !out_task || !queue->head) return false;
    
    cf_task_t* task = queue->head;
    queue->head = task->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->count--;
    
    out_task->func = task->func;
    out_task->arg = task->arg;
    free(task);
    
    return true;
}

static void task_queue_destroy(cf_task_queue_t* queue) {
    if (!queue) return;
    
    while (queue->head) {
        cf_task_t* task = queue->head;
        queue->head = task->next;
        free(task);
    }
    
    free(queue);
}

/* ========== 线程池实现 ========== */

struct cf_thread_pool {
    cf_thread_handle_t* threads;
    int num_threads;
    
    cf_task_queue_t* queue;
    cf_mutex_t* queue_mutex;
    cf_cond_t* queue_cond;
    
    cf_atomic_int_t active_threads;
    cf_atomic_int_t shutdown;
};

typedef struct {
    cf_thread_pool_t* pool;
    int thread_id;
} thread_worker_arg_t;

static CF_THREAD_FUNC_RETURN CF_THREAD_FUNC_CALL thread_worker(void* arg) {
    thread_worker_arg_t* worker_arg = (thread_worker_arg_t*)arg;
    cf_thread_pool_t* pool = worker_arg->pool;
    free(worker_arg);
    
    while (1) {
        cf_mutex_lock(pool->queue_mutex);
        
        // 等待任务或关闭信号
        while (pool->queue->count == 0 && pool->shutdown.value == 0) {
            cf_cond_wait(pool->queue_cond, pool->queue_mutex);
        }
        
        // 检查是否需要关闭
        if (pool->shutdown.value != 0 && pool->queue->count == 0) {
            cf_mutex_unlock(pool->queue_mutex);
            break;
        }
        
        // 获取任务
        cf_task_t task;
        if (task_queue_pop(pool->queue, &task)) {
            cf_atomic_add(&pool->active_threads, 1);
            cf_mutex_unlock(pool->queue_mutex);
            
            // 执行任务
            task.func(task.arg);
            
            cf_atomic_add(&pool->active_threads, -1);
        } else {
            cf_mutex_unlock(pool->queue_mutex);
        }
    }
    
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

/* ========== 获取CPU核心数 ========== */

static int get_cpu_count(void) {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (int)sysinfo.dwNumberOfProcessors;
#else
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    return (nprocs > 0) ? (int)nprocs : 1;
#endif
}

/* ========== 公共API ========== */

cf_thread_config_t cf_thread_config_default(void) {
    cf_thread_config_t config;
    config.num_threads = 0;  // 自动检测
    config.enable_threading = true;
    config.task_queue_size = 1024;
    return config;
}

cf_result_t cf_thread_pool_create(
    const cf_thread_config_t* config,
    cf_thread_pool_t** pool
) {
    if (!config || !pool) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 如果禁用多线程，返回NULL（调用者需要检查）
    if (!config->enable_threading) {
        *pool = NULL;
        return CF_SUCCESS;
    }
    
    cf_thread_pool_t* p = (cf_thread_pool_t*)malloc(sizeof(cf_thread_pool_t));
    if (!p) return CF_ERROR_OUT_OF_MEMORY;
    
    memset(p, 0, sizeof(cf_thread_pool_t));
    
    // 确定线程数
    p->num_threads = config->num_threads;
    if (p->num_threads <= 0) {
        p->num_threads = get_cpu_count();
    }
    if (p->num_threads < 1) p->num_threads = 1;
    if (p->num_threads > 64) p->num_threads = 64;  // 限制最大线程数
    
    // 创建任务队列
    cf_result_t result = task_queue_create(config->task_queue_size, &p->queue);
    if (result != CF_SUCCESS) {
        free(p);
        return result;
    }
    
    // 创建互斥锁和条件变量
    result = cf_mutex_create(&p->queue_mutex);
    if (result != CF_SUCCESS) {
        task_queue_destroy(p->queue);
        free(p);
        return result;
    }
    
    result = cf_cond_create(&p->queue_cond);
    if (result != CF_SUCCESS) {
        cf_mutex_destroy(p->queue_mutex);
        task_queue_destroy(p->queue);
        free(p);
        return result;
    }
    
    // 初始化原子变量
    p->active_threads.value = 0;
    p->shutdown.value = 0;
    
    // 创建工作线程
    p->threads = (cf_thread_handle_t*)malloc(sizeof(cf_thread_handle_t) * p->num_threads);
    if (!p->threads) {
        cf_cond_destroy(p->queue_cond);
        cf_mutex_destroy(p->queue_mutex);
        task_queue_destroy(p->queue);
        free(p);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    for (int i = 0; i < p->num_threads; i++) {
        thread_worker_arg_t* arg = (thread_worker_arg_t*)malloc(sizeof(thread_worker_arg_t));
        if (!arg) {
            // 清理已创建的线程
            p->shutdown.value = 1;
            cf_cond_broadcast(p->queue_cond);
            for (int j = 0; j < i; j++) {
#ifdef _WIN32
                WaitForSingleObject(p->threads[j], INFINITE);
                CloseHandle(p->threads[j]);
#else
                pthread_join(p->threads[j], NULL);
#endif
            }
            free(p->threads);
            cf_cond_destroy(p->queue_cond);
            cf_mutex_destroy(p->queue_mutex);
            task_queue_destroy(p->queue);
            free(p);
            return CF_ERROR_OUT_OF_MEMORY;
        }
        
        arg->pool = p;
        arg->thread_id = i;
        
#ifdef _WIN32
        p->threads[i] = (HANDLE)_beginthreadex(NULL, 0, thread_worker, arg, 0, NULL);
        if (p->threads[i] == 0) {
            free(arg);
            // 清理
            p->shutdown.value = 1;
            cf_cond_broadcast(p->queue_cond);
            for (int j = 0; j < i; j++) {
                WaitForSingleObject(p->threads[j], INFINITE);
                CloseHandle(p->threads[j]);
            }
            free(p->threads);
            cf_cond_destroy(p->queue_cond);
            cf_mutex_destroy(p->queue_mutex);
            task_queue_destroy(p->queue);
            free(p);
            return CF_ERROR_UNKNOWN;
        }
#else
        if (pthread_create(&p->threads[i], NULL, thread_worker, arg) != 0) {
            free(arg);
            // 清理
            p->shutdown.value = 1;
            cf_cond_broadcast(p->queue_cond);
            for (int j = 0; j < i; j++) {
                pthread_join(p->threads[j], NULL);
            }
            free(p->threads);
            cf_cond_destroy(p->queue_cond);
            cf_mutex_destroy(p->queue_mutex);
            task_queue_destroy(p->queue);
            free(p);
            return CF_ERROR_UNKNOWN;
        }
#endif
    }
    
    *pool = p;
    return CF_SUCCESS;
}

cf_result_t cf_thread_pool_submit(
    cf_thread_pool_t* pool,
    cf_task_func_t func,
    void* arg
) {
    if (!pool || !func) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_mutex_lock(pool->queue_mutex);
    
    if (pool->shutdown.value != 0) {
        cf_mutex_unlock(pool->queue_mutex);
        return CF_ERROR_NOT_INITIALIZED;
    }
    
    if (!task_queue_push(pool->queue, func, arg)) {
        cf_mutex_unlock(pool->queue_mutex);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    cf_cond_signal(pool->queue_cond);
    cf_mutex_unlock(pool->queue_mutex);
    
    return CF_SUCCESS;
}

void cf_thread_pool_wait(cf_thread_pool_t* pool) {
    if (!pool) return;
    
    // 等待所有任务完成
    while (1) {
        cf_mutex_lock(pool->queue_mutex);
        int pending = pool->queue->count;
        int active = pool->active_threads.value;
        cf_mutex_unlock(pool->queue_mutex);
        
        if (pending == 0 && active == 0) {
            break;
        }
        
        // 短暂休眠
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
    }
}

void cf_thread_pool_get_stats(
    const cf_thread_pool_t* pool,
    int* active_threads,
    int* pending_tasks
) {
    if (!pool) return;
    
    if (active_threads) {
        *active_threads = pool->active_threads.value;
    }
    
    if (pending_tasks) {
        cf_mutex_lock(pool->queue_mutex);
        *pending_tasks = pool->queue->count;
        cf_mutex_unlock(pool->queue_mutex);
    }
}

void cf_thread_pool_destroy(cf_thread_pool_t* pool) {
    if (!pool) return;
    
    // 设置关闭标志
    pool->shutdown.value = 1;
    
    // 唤醒所有线程
    cf_cond_broadcast(pool->queue_cond);
    
    // 等待所有线程结束
    for (int i = 0; i < pool->num_threads; i++) {
#ifdef _WIN32
        WaitForSingleObject(pool->threads[i], INFINITE);
        CloseHandle(pool->threads[i]);
#else
        pthread_join(pool->threads[i], NULL);
#endif
    }
    
    // 清理资源
    free(pool->threads);
    cf_cond_destroy(pool->queue_cond);
    cf_mutex_destroy(pool->queue_mutex);
    task_queue_destroy(pool->queue);
    free(pool);
}
