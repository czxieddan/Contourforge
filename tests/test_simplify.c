/**
 * @file test_simplify.c
 * @brief 线段简化算法测试
 */

#include <contourforge/datagen.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void test_douglas_peucker(void)
{
    printf("Testing Douglas-Peucker simplification...\n");
    
    // 创建测试点集（一条锯齿线）
    cf_point3_t points[] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {2.0f, 0.5f, 0.0f},
        {3.0f, 1.5f, 0.0f},
        {4.0f, 1.0f, 0.0f},
        {5.0f, 0.0f, 0.0f}
    };
    size_t count = 6;
    
    cf_point3_t* out_points = NULL;
    size_t out_count = 0;
    
    cf_result_t result = cf_simplify_douglas_peucker(
        points,
        count,
        0.5f,  // 容差
        &out_points,
        &out_count
    );
    
    assert(result == CF_SUCCESS);
    assert(out_points != NULL);
    assert(out_count < count);  // 简化后点数应该减少
    
    printf("    Original points: %zu\n", count);
    printf("    Simplified points: %zu\n", out_count);
    printf("    Reduction: %.1f%%\n", (1.0f - (float)out_count / count) * 100.0f);
    
    free(out_points);
    
    printf("  Douglas-Peucker test passed!\n");
}

void test_visvalingam(void)
{
    printf("Testing Visvalingam-Whyatt simplification...\n");
    
    // 创建测试点集
    cf_point3_t points[] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {2.0f, 0.5f, 0.0f},
        {3.0f, 1.5f, 0.0f},
        {4.0f, 1.0f, 0.0f},
        {5.0f, 0.0f, 0.0f}
    };
    size_t count = 6;
    size_t target_count = 4;
    
    cf_point3_t* out_points = NULL;
    size_t out_count = 0;
    
    cf_result_t result = cf_simplify_visvalingam(
        points,
        count,
        target_count,
        &out_points,
        &out_count
    );
    
    assert(result == CF_SUCCESS);
    assert(out_points != NULL);
    assert(out_count == target_count);
    
    printf("    Original points: %zu\n", count);
    printf("    Target points: %zu\n", target_count);
    printf("    Result points: %zu\n", out_count);
    
    free(out_points);
    
    printf("  Visvalingam-Whyatt test passed!\n");
}

int main(void)
{
    printf("Running line simplification tests...\n\n");
    
    test_douglas_peucker();
    test_visvalingam();
    
    printf("\nAll simplification tests passed!\n");
    return 0;
}
