/**
 * @file memory.c
 * @brief Contourforge内存管理实现
 */

#include "contourforge/core.h"
#include "contourforge/types.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========== 内存池实现 ========== */

/**
 * @brief 内存块节点
 */
typedef struct cf_memory_block {
    struct cf_memory_block* next;  /**< 下一个空闲块 */
} cf_memory_block_t;

/**
 * @brief 内存池结构
 */
struct cf_memory_pool {
    void* memory;                   /**< 内存区域 */
    cf_memory_block_t* free_list;   /**< 空闲链表 */
    size_t block_size;              /**< 块大小 */
    size_t block_count;             /**< 块数量 */
    size_t allocated_count;         /**< 已分配数量 */
    
#ifdef CF_DEBUG_MEMORY
    size_t peak_usage;              /**< 峰值使用量 */
    size_t total_allocs;            /**< 总分配次数 */
    size_t total_frees;             /**< 总释放次数 */
#endif
};

/**
 * @brief 创建内存池
 */
cf_result_t cf_memory_pool_create(
    size_t block_size,
    size_t block_count,
    cf_memory_pool_t** pool
) {
    if (block_size == 0 || block_count == 0 || pool == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 确保块大小至少能容纳指针 */
    if (block_size < sizeof(cf_memory_block_t)) {
        block_size = sizeof(cf_memory_block_t);
    }
    
    /* 分配内存池结构 */
    cf_memory_pool_t* p = (cf_memory_pool_t*)malloc(sizeof(cf_memory_pool_t));
    if (p == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    /* 分配内存区域 */
    p->memory = malloc(block_size * block_count);
    if (p->memory == NULL) {
        free(p);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    /* 初始化内存池 */
    p->block_size = block_size;
    p->block_count = block_count;
    p->allocated_count = 0;
    p->free_list = NULL;
    
#ifdef CF_DEBUG_MEMORY
    p->peak_usage = 0;
    p->total_allocs = 0;
    p->total_frees = 0;
#endif
    
    /* 构建空闲链表 */
    char* ptr = (char*)p->memory;
    for (size_t i = 0; i < block_count; i++) {
        cf_memory_block_t* block = (cf_memory_block_t*)(ptr + i * block_size);
        block->next = p->free_list;
        p->free_list = block;
    }
    
    *pool = p;
    return CF_SUCCESS;
}

/**
 * @brief 从内存池分配内存
 */
void* cf_memory_pool_alloc(cf_memory_pool_t* pool) {
    if (pool == NULL || pool->free_list == NULL) {
        return NULL;
    }
    
    /* 从空闲链表取出一个块 */
    cf_memory_block_t* block = pool->free_list;
    pool->free_list = block->next;
    pool->allocated_count++;
    
#ifdef CF_DEBUG_MEMORY
    pool->total_allocs++;
    if (pool->allocated_count > pool->peak_usage) {
        pool->peak_usage = pool->allocated_count;
    }
#endif
    
    /* 清零内存 */
    memset(block, 0, pool->block_size);
    
    return block;
}

/**
 * @brief 释放内存到内存池
 */
void cf_memory_pool_free(cf_memory_pool_t* pool, void* ptr) {
    if (pool == NULL || ptr == NULL) {
        return;
    }
    
    /* 检查指针是否在内存池范围内 */
    char* mem_start = (char*)pool->memory;
    char* mem_end = mem_start + (pool->block_size * pool->block_count);
    char* p = (char*)ptr;
    
    if (p < mem_start || p >= mem_end) {
        /* 指针不在内存池范围内 */
        assert(0 && "Invalid pointer passed to cf_memory_pool_free");
        return;
    }
    
    /* 检查指针是否对齐 */
    size_t offset = p - mem_start;
    if (offset % pool->block_size != 0) {
        /* 指针未对齐 */
        assert(0 && "Unaligned pointer passed to cf_memory_pool_free");
        return;
    }
    
    /* 将块加入空闲链表 */
    cf_memory_block_t* block = (cf_memory_block_t*)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
    pool->allocated_count--;
    
#ifdef CF_DEBUG_MEMORY
    pool->total_frees++;
#endif
}

/**
 * @brief 获取内存池统计信息
 */
void cf_memory_pool_get_stats(
    const cf_memory_pool_t* pool,
    size_t* allocated,
    size_t* capacity
) {
    if (pool == NULL) {
        return;
    }
    
    if (allocated != NULL) {
        *allocated = pool->allocated_count;
    }
    
    if (capacity != NULL) {
        *capacity = pool->block_count;
    }
}

/**
 * @brief 销毁内存池
 */
void cf_memory_pool_destroy(cf_memory_pool_t* pool) {
    if (pool == NULL) {
        return;
    }
    
#ifdef CF_DEBUG_MEMORY
    /* 检查内存泄漏 */
    if (pool->allocated_count > 0) {
        fprintf(stderr, "Memory pool leak detected: %zu blocks still allocated\n",
                pool->allocated_count);
    }
    
    /* 打印统计信息 */
    fprintf(stderr, "Memory pool stats:\n");
    fprintf(stderr, "  Total allocations: %zu\n", pool->total_allocs);
    fprintf(stderr, "  Total frees: %zu\n", pool->total_frees);
    fprintf(stderr, "  Peak usage: %zu/%zu blocks\n", pool->peak_usage, pool->block_count);
#endif
    
    free(pool->memory);
    free(pool);
}

/* ========== 对象池实现 ========== */

/**
 * @brief 对象池（基于内存池）
 */
typedef struct cf_object_pool {
    cf_memory_pool_t* memory_pool;  /**< 底层内存池 */
    size_t object_size;              /**< 对象大小 */
} cf_object_pool_t;

/**
 * @brief 创建对象池
 */
cf_result_t cf_object_pool_create(
    size_t object_size,
    size_t initial_capacity,
    cf_object_pool_t** pool
) {
    if (object_size == 0 || initial_capacity == 0 || pool == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_object_pool_t* p = (cf_object_pool_t*)malloc(sizeof(cf_object_pool_t));
    if (p == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    /* 创建底层内存池 */
    cf_result_t result = cf_memory_pool_create(
        object_size,
        initial_capacity,
        &p->memory_pool
    );
    
    if (CF_FAILED(result)) {
        free(p);
        return result;
    }
    
    p->object_size = object_size;
    *pool = p;
    
    return CF_SUCCESS;
}

/**
 * @brief 从对象池分配对象
 */
void* cf_object_pool_alloc(cf_object_pool_t* pool) {
    if (pool == NULL) {
        return NULL;
    }
    
    return cf_memory_pool_alloc(pool->memory_pool);
}

/**
 * @brief 释放对象到对象池
 */
void cf_object_pool_free(cf_object_pool_t* pool, void* object) {
    if (pool == NULL || object == NULL) {
        return;
    }
    
    cf_memory_pool_free(pool->memory_pool, object);
}

/**
 * @brief 销毁对象池
 */
void cf_object_pool_destroy(cf_object_pool_t* pool) {
    if (pool == NULL) {
        return;
    }
    
    cf_memory_pool_destroy(pool->memory_pool);
    free(pool);
}

/* ========== 通用内存分配器 ========== */

/**
 * @brief 分配内存（带对齐）
 */
void* cf_malloc_aligned(size_t size, size_t alignment) {
    if (size == 0) {
        return NULL;
    }
    
    /* Windows使用_aligned_malloc，其他平台使用aligned_alloc */
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    return aligned_alloc(alignment, size);
#endif
}

/**
 * @brief 释放对齐内存
 */
void cf_free_aligned(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

/**
 * @brief 分配并清零内存
 */
void* cf_calloc(size_t count, size_t size) {
    return calloc(count, size);
}

/**
 * @brief 重新分配内存
 */
void* cf_realloc(void* ptr, size_t new_size) {
    return realloc(ptr, new_size);
}

/**
 * @brief 释放内存
 */
void cf_free(void* ptr) {
    free(ptr);
}
