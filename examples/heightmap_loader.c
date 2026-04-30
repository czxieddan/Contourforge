/**
 * @file heightmap_loader.c
 * @brief 高度图加载和分析示例
 * 
 * 这个示例展示如何：
 * 1. 加载高度图
 * 2. 分析高度图统计信息
 * 3. 采样高度值
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>

void print_heightmap_stats(const cf_heightmap_t* heightmap)
{
    printf("\nHeightmap Statistics:\n");
    printf("  Dimensions: %dx%d\n", heightmap->width, heightmap->height);
    printf("  Total pixels: %d\n", heightmap->width * heightmap->height);
    printf("  Min height: %.2f\n", heightmap->min_height);
    printf("  Max height: %.2f\n", heightmap->max_height);
    printf("  Height range: %.2f\n", heightmap->max_height - heightmap->min_height);
    
    // 计算平均高度
    float sum = 0.0f;
    int count = heightmap->width * heightmap->height;
    for (int i = 0; i < count; i++) {
        sum += heightmap->data[i];
    }
    float avg = sum / count;
    printf("  Average height: %.2f\n", avg);
}

void sample_heightmap(const cf_heightmap_t* heightmap)
{
    printf("\nSampling heightmap at various positions:\n");
    
    float positions[][2] = {
        {0.0f, 0.0f},   // 左下角
        {0.5f, 0.5f},   // 中心
        {1.0f, 1.0f},   // 右上角
        {0.25f, 0.75f}, // 左上四分之一
        {0.75f, 0.25f}  // 右下四分之一
    };
    
    for (int i = 0; i < 5; i++) {
        float x = positions[i][0];
        float y = positions[i][1];
        float height = cf_heightmap_sample(heightmap, x, y);
        printf("  Position (%.2f, %.2f): height = %.2f\n", x, y, height);
    }
}

int main(int argc, char** argv)
{
    printf("Contourforge Heightmap Loader v%s\n", cf_get_version());
    printf("========================================\n\n");
    
    // 检查命令行参数
    if (argc < 2) {
        printf("Usage: %s <heightmap.png>\n", argv[0]);
        printf("Example: %s ../data/heightmaps/terrain.png\n", argv[0]);
        return 1;
    }
    
    const char* heightmap_path = argv[1];
    
    // 加载高度图
    printf("Loading heightmap: %s\n", heightmap_path);
    cf_heightmap_t* heightmap = NULL;
    cf_result_t result = cf_heightmap_load(heightmap_path, &heightmap);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to load heightmap (code: %d)\n", result);
        return 1;
    }
    
    printf("Heightmap loaded successfully!\n");
    
    // 打印统计信息
    print_heightmap_stats(heightmap);
    
    // 采样高度图
    sample_heightmap(heightmap);
    
    // 清理
    printf("\nCleaning up...\n");
    cf_heightmap_destroy(heightmap);
    
    printf("Done.\n");
    return 0;
}
