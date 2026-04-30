/**
 * @file test_memory.c
 * @brief 内存管理测试
 */

#include <contourforge/core.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_memory_pool_create(void)
{
    printf("Testing memory pool creation...\n");
    
    cf_memory_pool_t* pool = NULL;
    cf_result_t result = cf_memory_pool_create(64, 100, &pool);
    
    assert(result == CF_SUCCESS);
    assert(pool != NULL);
    
    cf_memory_pool_destroy(pool);
    
    printf("  Memory pool creation test passed!\n");
}

void test_memory_pool_alloc_free(void)
{
    printf("Testing memory pool allocation and deallocation...\n");
    
    cf_memory_pool_t* pool = NULL;
    cf_memory_pool_create(64, 10, &pool);
    
    // 分配内存
    void* ptr1 = cf_memory_pool_alloc(pool);
    assert(ptr1 != NULL);
    
    void* ptr2 = cf_memory_pool_alloc(pool);
    assert(ptr2 != NULL);
    assert(ptr1 != ptr2);
    
    // 写入数据
    memset(ptr1, 0xAA, 64);
    memset(ptr2, 0xBB, 64);
    
    // 释放内存
    cf_memory_pool_free(pool, ptr1);
    cf_memory_pool_free(pool, ptr2);
    
    // 重新分配（应该复用之前的内存）
    void* ptr3 = cf_memory_pool_alloc(pool);
    assert(ptr3 != NULL);
    
    cf_memory_pool_destroy(pool);
    
    printf("  Memory pool alloc/free test passed!\n");
}

void test_memory_pool_exhaustion(void)
{
    printf("Testing memory pool exhaustion...\n");
    
    cf_memory_pool_t* pool = NULL;
    cf_memory_pool_create(64, 3, &pool);
    
    void* ptr1 = cf_memory_pool_alloc(pool);
    void* ptr2 = cf_memory_pool_alloc(pool);
    void* ptr3 = cf_memory_pool_alloc(pool);
    
    assert(ptr1 != NULL);
    assert(ptr2 != NULL);
    assert(ptr3 != NULL);
    
    // 第4次分配应该失败
    void* ptr4 = cf_memory_pool_alloc(pool);
    assert(ptr4 == NULL);
    
    // 释放一个后应该可以再次分配
    cf_memory_pool_free(pool, ptr2);
    void* ptr5 = cf_memory_pool_alloc(pool);
    assert(ptr5 != NULL);
    
    cf_memory_pool_destroy(pool);
    
    printf("  Memory pool exhaustion test passed!\n");
}

int main(void)
{
    printf("Running memory management tests...\n\n");
    
    test_memory_pool_create();
    test_memory_pool_alloc_free();
    test_memory_pool_exhaustion();
    
    printf("\nAll memory tests passed!\n");
    return 0;
}
