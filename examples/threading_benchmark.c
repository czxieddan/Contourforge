/**
 * @file threading_benchmark.c
 * @brief 多线程性能测试
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// 获取当前时间（毫秒）
static double get_time_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)(counter.QuadPart * 1000.0 / frequency.QuadPart);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
#endif
}

// 测试等高线提取性能
static void benchmark_contour_extraction(const char* heightmap_path) {
    printf("\n=== 等高线提取性能测试 ===\n");
    
    // 加载高度图
    cf_heightmap_t* heightmap;
    cf_result_t result = cf_heightmap_load(heightmap_path, &heightmap);
    if (result != CF_SUCCESS) {
        printf("错误：无法加载高度图\n");
        return;
    }
    
    printf("高度图尺寸: %dx%d\n", heightmap->width, heightmap->height);
    
    // 创建线程池
    cf_thread_config_t thread_config = cf_thread_config_default();
    cf_thread_pool_t* thread_pool;
    
    // 测试不同线程数
    int thread_counts[] = {1, 2, 4, 8};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    for (int i = 0; i < num_tests; i++) {
        thread_config.num_threads = thread_counts[i];
        
        result = cf_thread_pool_create(&thread_config, &thread_pool);
        if (result != CF_SUCCESS) {
            printf("错误：无法创建线程池\n");
            continue;
        }
        
        // 创建点集和线集
        cf_point_set_t* point_set;
        cf_line_set_t* line_set;
        cf_point_set_create(10000, &point_set);
        cf_line_set_create(point_set, 10000, &line_set);
        
        // 测试提取
        double start_time = get_time_ms();
        
        result = cf_contour_extract_parallel(
            heightmap,
            (heightmap->min_height + heightmap->max_height) / 2.0f,
            thread_pool,
            point_set,
            line_set
        );
        
        double end_time = get_time_ms();
        double elapsed = end_time - start_time;
        
        if (result == CF_SUCCESS) {
            printf("线程数: %d, 时间: %.2f ms, 点数: %zu, 线段数: %zu\n",
                   thread_counts[i], elapsed, point_set->count, line_set->count);
        } else {
            printf("线程数: %d, 提取失败\n", thread_counts[i]);
        }
        
        // 清理
        cf_line_set_destroy(line_set);
        cf_point_set_destroy(point_set);
        cf_thread_pool_destroy(thread_pool);
    }
    
    cf_heightmap_destroy(heightmap);
}

// 测试线段简化性能
static void benchmark_simplification(void) {
    printf("\n=== 线段简化性能测试 ===\n");
    
    // 生成测试数据
    size_t point_count = 10000;
    cf_point3_t* points = (cf_point3_t*)malloc(sizeof(cf_point3_t) * point_count);
    
    for (size_t i = 0; i < point_count; i++) {
        float t = (float)i / (float)point_count;
        points[i].x = t * 100.0f;
        points[i].y = sinf(t * 20.0f) * 10.0f;
        points[i].z = cosf(t * 15.0f) * 5.0f;
    }
    
    printf("输入点数: %zu\n", point_count);
    
    // 创建线程池
    cf_thread_config_t thread_config = cf_thread_config_default();
    cf_thread_pool_t* thread_pool;
    
    int thread_counts[] = {1, 2, 4, 8};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    for (int i = 0; i < num_tests; i++) {
        thread_config.num_threads = thread_counts[i];
        
        cf_result_t result = cf_thread_pool_create(&thread_config, &thread_pool);
        if (result != CF_SUCCESS) {
            printf("错误：无法创建线程池\n");
            continue;
        }
        
        cf_point3_t* out_points = NULL;
        size_t out_count = 0;
        
        double start_time = get_time_ms();
        
        result = cf_simplify_douglas_peucker_parallel(
            points,
            point_count,
            0.5f,
            thread_pool,
            &out_points,
            &out_count
        );
        
        double end_time = get_time_ms();
        double elapsed = end_time - start_time;
        
        if (result == CF_SUCCESS) {
            float reduction = (1.0f - (float)out_count / (float)point_count) * 100.0f;
            printf("线程数: %d, 时间: %.2f ms, 输出点数: %zu, 简化率: %.1f%%\n",
                   thread_counts[i], elapsed, out_count, reduction);
            free(out_points);
        } else {
            printf("线程数: %d, 简化失败\n", thread_counts[i]);
        }
        
        cf_thread_pool_destroy(thread_pool);
    }
    
    free(points);
}

// 测试LOD生成性能
static void benchmark_lod_generation(const char* heightmap_path) {
    printf("\n=== LOD生成性能测试 ===\n");
    
    // 加载高度图并生成等高线
    cf_heightmap_t* heightmap;
    cf_result_t result = cf_heightmap_load(heightmap_path, &heightmap);
    if (result != CF_SUCCESS) {
        printf("错误：无法加载高度图\n");
        return;
    }
    
    cf_contour_config_t contour_config = {
        .interval = 10.0f,
        .min_height = heightmap->min_height,
        .max_height = heightmap->max_height,
        .simplify_tolerance = 0.0f,
        .build_topology = false
    };
    
    cf_model_t* model;
    result = cf_contour_generate(heightmap, &contour_config, &model);
    if (result != CF_SUCCESS) {
        printf("错误：无法生成等高线\n");
        cf_heightmap_destroy(heightmap);
        return;
    }
    
    printf("基础模型点数: %zu\n", model->points->count);
    
    // 配置LOD
    cf_lod_config_t lod_config;
    lod_config.level_count = 4;
    
    float distances[] = {0.0f, 50.0f, 100.0f, 200.0f};
    float ratios[] = {1.0f, 0.5f, 0.25f, 0.1f};
    
    lod_config.distance_thresholds = distances;
    lod_config.simplification_ratios = ratios;
    lod_config.preserve_boundaries = true;
    lod_config.use_importance_sampling = true;
    
    // 测试不同线程数
    int thread_counts[] = {1, 2, 4};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    for (int i = 0; i < num_tests; i++) {
        cf_thread_config_t thread_config = cf_thread_config_default();
        thread_config.num_threads = thread_counts[i];
        
        cf_thread_pool_t* thread_pool;
        result = cf_thread_pool_create(&thread_config, &thread_pool);
        if (result != CF_SUCCESS) {
            printf("错误：无法创建线程池\n");
            continue;
        }
        
        cf_lod_model_t* lod_model;
        
        double start_time = get_time_ms();
        
        result = cf_lod_create_parallel(model, &lod_config, thread_pool, &lod_model);
        
        double end_time = get_time_ms();
        double elapsed = end_time - start_time;
        
        if (result == CF_SUCCESS) {
            printf("线程数: %d, 时间: %.2f ms, LOD层级: %zu\n",
                   thread_counts[i], elapsed, lod_model->level_count);
            
            // 显示各层级信息
            for (size_t j = 0; j < lod_model->level_count; j++) {
                printf("  层级 %zu: 点数=%zu, 简化率=%.1f%%\n",
                       j,
                       lod_model->levels[j].point_count,
                       lod_model->levels[j].simplification_ratio * 100.0f);
            }
            
            cf_lod_destroy(lod_model);
        } else {
            printf("线程数: %d, LOD生成失败\n", thread_counts[i]);
        }
        
        cf_thread_pool_destroy(thread_pool);
    }
    
    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
}

int main(int argc, char** argv) {
    printf("Contourforge 多线程性能测试\n");
    printf("================================\n");
    
    const char* heightmap_path = "data/heightmaps/terrain_large.png";
    
    if (argc > 1) {
        heightmap_path = argv[1];
    }
    
    printf("使用高度图: %s\n", heightmap_path);
    
    // 运行测试
    benchmark_contour_extraction(heightmap_path);
    benchmark_simplification();
    benchmark_lod_generation(heightmap_path);
    
    printf("\n测试完成！\n");
    
    return 0;
}
