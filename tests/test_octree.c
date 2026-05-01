/**
 * @file test_octree.c
 * @brief 八叉树测试
 */

#include <contourforge/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void test_octree_create(void)
{
    printf("Testing octree creation...\n");
    
    cf_bounds_t bounds = {
        .min = {0.0f, 0.0f, 0.0f},
        .max = {100.0f, 100.0f, 100.0f}
    };
    
    cf_octree_t* octree = NULL;
    cf_result_t result = cf_octree_create(bounds, 8, 10, &octree);
    
    assert(result == CF_SUCCESS);
    assert(octree != NULL);
    
    cf_octree_destroy(octree);
    
    printf("  Octree creation test passed!\n");
}

void test_octree_insert(void)
{
    printf("Testing octree insertion...\n");
    
    cf_bounds_t bounds = {
        .min = {0.0f, 0.0f, 0.0f},
        .max = {100.0f, 100.0f, 100.0f}
    };
    
    cf_octree_t* octree = NULL;
    cf_octree_create(bounds, 8, 10, &octree);
    
    // 插入点
    cf_point3_t p1 = {10.0f, 10.0f, 10.0f};
    cf_result_t result = cf_octree_insert(octree, p1, 0);
    assert(result == CF_SUCCESS);
    
    cf_point3_t p2 = {50.0f, 50.0f, 50.0f};
    result = cf_octree_insert(octree, p2, 1);
    assert(result == CF_SUCCESS);
    
    cf_point3_t p3 = {90.0f, 90.0f, 90.0f};
    result = cf_octree_insert(octree, p3, 2);
    assert(result == CF_SUCCESS);
    
    cf_octree_destroy(octree);
    
    printf("  Octree insertion test passed!\n");
}

void test_octree_query(void)
{
    printf("Testing octree query...\n");
    
    cf_bounds_t bounds = {
        .min = {0.0f, 0.0f, 0.0f},
        .max = {100.0f, 100.0f, 100.0f}
    };
    
    cf_octree_t* octree = NULL;
    cf_octree_create(bounds, 8, 10, &octree);
    
    // 插入多个点
    for (int i = 0; i < 20; i++) {
        cf_point3_t p = {
            (float)(i * 5),
            (float)(i * 5),
            (float)(i * 5)
        };
        cf_octree_insert(octree, p, i);
    }
    
    // 查询特定区域
    cf_bounds_t query_bounds = {
        .min = {0.0f, 0.0f, 0.0f},
        .max = {50.0f, 50.0f, 50.0f}
    };
    
    cf_index_t* indices = NULL;
    size_t count = 0;
    cf_result_t result = cf_octree_query(octree, query_bounds, &indices, &count);
    
    assert(result == CF_SUCCESS);
    assert(count > 0);
    assert(indices != NULL);
    
    printf("    Found %zu points in query region\n", count);
    
    // 清理
    free(indices);
    cf_octree_destroy(octree);
    
    printf("  Octree query test passed!\n");
}

int main(void)
{
    printf("Running octree tests...\n\n");
    
    test_octree_create();
    test_octree_insert();
    test_octree_query();
    
    printf("\nAll octree tests passed!\n");
    return 0;
}
