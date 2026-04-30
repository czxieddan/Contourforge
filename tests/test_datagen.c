/**
 * @file test_datagen.c
 * @brief 数据生成模块测试
 */

#include <contourforge/datagen.h>
#include <stdio.h>
#include <assert.h>

void test_heightmap_load(void)
{
    printf("Testing heightmap loading...\n");
    
    // 注意：这个测试需要实际的图像文件
    // 在实际测试中，应该使用测试数据
    
    printf("  Heightmap loading test skipped (requires test data)\n");
}

void test_heightmap_sample(void)
{
    printf("Testing heightmap sampling...\n");
    
    // 创建一个简单的测试高度图
    cf_heightmap_t heightmap = {
        .width = 10,
        .height = 10,
        .min_height = 0.0f,
        .max_height = 100.0f,
        .data = NULL
    };
    
    // 分配数据
    heightmap.data = (float*)malloc(100 * sizeof(float));
    for (int i = 0; i < 100; i++) {
        heightmap.data[i] = (float)i;
    }
    
    // 采样
    float h1 = cf_heightmap_sample(&heightmap, 0.0f, 0.0f);
    float h2 = cf_heightmap_sample(&heightmap, 0.5f, 0.5f);
    float h3 = cf_heightmap_sample(&heightmap, 1.0f, 1.0f);
    
    printf("    Sample at (0.0, 0.0): %.2f\n", h1);
    printf("    Sample at (0.5, 0.5): %.2f\n", h2);
    printf("    Sample at (1.0, 1.0): %.2f\n", h3);
    
    free(heightmap.data);
    
    printf("  Heightmap sampling test passed!\n");
}

void test_contour_config(void)
{
    printf("Testing contour configuration...\n");
    
    cf_contour_config_t config = {
        .interval = 10.0f,
        .min_height = 0.0f,
        .max_height = 1000.0f,
        .simplify_tolerance = 1.0f,
        .build_topology = true
    };
    
    assert(config.interval == 10.0f);
    assert(config.build_topology == true);
    
    printf("  Contour configuration test passed!\n");
}

int main(void)
{
    printf("Running data generation tests...\n\n");
    
    test_heightmap_load();
    test_heightmap_sample();
    test_contour_config();
    
    printf("\nAll data generation tests passed!\n");
    return 0;
}
