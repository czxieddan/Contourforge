/**
 * @file simple_viewer.c
 * @brief 简单的等高线查看器示例
 * 
 * 这个示例展示如何：
 * 1. 加载高度图
 * 2. 生成等高线
 * 3. 初始化渲染器
 * 4. 渲染等高线模型
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    printf("Contourforge Simple Viewer v%s\n", cf_get_version());
    printf("========================================\n\n");
    
    // 检查命令行参数
    if (argc < 2) {
        printf("Usage: %s <heightmap.png>\n", argv[0]);
        printf("Example: %s ../data/heightmaps/terrain.png\n", argv[0]);
        return 1;
    }
    
    const char* heightmap_path = argv[1];
    
    // 1. 加载高度图
    printf("Loading heightmap: %s\n", heightmap_path);
    cf_heightmap_t* heightmap = NULL;
    cf_result_t result = cf_heightmap_load(heightmap_path, &heightmap);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to load heightmap (code: %d)\n", result);
        return 1;
    }
    
    printf("  Size: %dx%d\n", heightmap->width, heightmap->height);
    printf("  Height range: %.2f - %.2f\n", heightmap->min_height, heightmap->max_height);
    
    // 2. 生成等高线
    printf("\nGenerating contours...\n");
    cf_contour_config_t config = {
        .interval = 10.0f,
        .min_height = heightmap->min_height,
        .max_height = heightmap->max_height,
        .simplify_tolerance = 1.0f,
        .build_topology = true
    };
    
    cf_model_t* model = NULL;
    result = cf_contour_generate(heightmap, &config, &model);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to generate contours (code: %d)\n", result);
        cf_heightmap_destroy(heightmap);
        return 1;
    }
    
    printf("  Points: %zu\n", model->points->count);
    printf("  Lines: %zu\n", model->lines->count);
    
    // 3. 初始化渲染器
    printf("\nInitializing renderer...\n");
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge Simple Viewer",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.15f, 1.0f}
    };
    
    cf_renderer_t* renderer = NULL;
    result = cf_renderer_init(&renderer_config, &renderer);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to initialize renderer (code: %d)\n", result);
        cf_model_destroy(model);
        cf_heightmap_destroy(heightmap);
        return 1;
    }
    
    // 设置模型
    result = cf_renderer_set_model(renderer, model);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to set model (code: %d)\n", result);
        cf_renderer_destroy(renderer);
        cf_model_destroy(model);
        cf_heightmap_destroy(heightmap);
        return 1;
    }
    
    printf("Renderer initialized successfully.\n");
    printf("\nControls:\n");
    printf("  Mouse: Rotate view\n");
    printf("  Scroll: Zoom in/out\n");
    printf("  ESC: Exit\n\n");
    
    // 4. 渲染循环
    printf("Starting render loop...\n");
    while (!cf_renderer_should_close(renderer)) {
        cf_renderer_begin_frame(renderer);
        cf_renderer_render(renderer);
        cf_renderer_end_frame(renderer);
    }
    
    // 5. 清理
    printf("\nCleaning up...\n");
    cf_renderer_destroy(renderer);
    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
    
    printf("Done.\n");
    return 0;
}
