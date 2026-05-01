/**
 * @file heightmap_loader.c
 * @brief 高度图加载和等高线生成测试 - Phase 2测试程序
 * 
 * 功能：
 * 1. 加载灰度图并生成3D模型
 * 2. 提取等高线并简化线段
 * 3. 渲染生成的等高线模型
 * 4. 显示统计信息（点数、线数、简化率等）
 * 5. 验证数据生成库的完整流程
 * 
 * 控制：
 * - 鼠标拖动：旋转视角
 * - 鼠标滚轮：缩放
 * - ESC：退出
 */

#include <contourforge/contourforge.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 全局状态
typedef struct {
    cf_renderer_t* renderer;
    cf_model_t* model;
    cf_camera_t* camera;
    cf_heightmap_t* heightmap;
    
    // 相机控制
    double last_mouse_x;
    double last_mouse_y;
    bool mouse_dragging;
    float camera_distance;
    float camera_angle_h;
    float camera_angle_v;
    
    // 统计信息
    double last_time;
    int frame_count;
    float fps;
    size_t original_point_count;
    size_t simplified_point_count;
    double generation_time;
} loader_state_t;

/**
 * @brief 更新相机位置
 */
void update_camera(loader_state_t* state) {
    float x = state->camera_distance * cosf(state->camera_angle_v) * sinf(state->camera_angle_h);
    float y = state->camera_distance * sinf(state->camera_angle_v);
    float z = state->camera_distance * cosf(state->camera_angle_v) * cosf(state->camera_angle_h);
    
    cf_point3_t position = {x, y, z};
    cf_point3_t target = {0.0f, 0.0f, 0.0f};
    
    cf_camera_set_position(state->camera, position);
    cf_camera_set_target(state->camera, target);
}

/**
 * @brief 处理鼠标输入
 */
void handle_mouse_input(loader_state_t* state) {
    void* window = cf_renderer_get_window(state->renderer);
    if (!window) return;
    
    double mouse_x, mouse_y;
    glfwGetCursorPos((GLFWwindow*)window, &mouse_x, &mouse_y);
    
    int left_button = glfwGetMouseButton((GLFWwindow*)window, GLFW_MOUSE_BUTTON_LEFT);
    
    if (left_button == GLFW_PRESS) {
        if (!state->mouse_dragging) {
            state->mouse_dragging = true;
            state->last_mouse_x = mouse_x;
            state->last_mouse_y = mouse_y;
        } else {
            double delta_x = mouse_x - state->last_mouse_x;
            double delta_y = mouse_y - state->last_mouse_y;
            
            state->camera_angle_h += (float)delta_x * 0.005f;
            state->camera_angle_v -= (float)delta_y * 0.005f;
            
            const float max_angle = (float)M_PI * 0.49f;
            if (state->camera_angle_v > max_angle) state->camera_angle_v = max_angle;
            if (state->camera_angle_v < -max_angle) state->camera_angle_v = -max_angle;
            
            state->last_mouse_x = mouse_x;
            state->last_mouse_y = mouse_y;
            
            update_camera(state);
        }
    } else {
        state->mouse_dragging = false;
    }
}

/**
 * @brief 滚轮回调（缩放）
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    loader_state_t* state = (loader_state_t*)glfwGetWindowUserPointer(window);
    if (!state) return;
    
    state->camera_distance *= (1.0f - (float)yoffset * 0.1f);
    
    if (state->camera_distance < 5.0f) state->camera_distance = 5.0f;
    if (state->camera_distance > 200.0f) state->camera_distance = 200.0f;
    
    update_camera(state);
}

/**
 * @brief 更新FPS统计
 */
void update_fps(loader_state_t* state) {
    double current_time = glfwGetTime();
    state->frame_count++;
    
    if (current_time - state->last_time >= 1.0) {
        state->fps = (float)state->frame_count / (float)(current_time - state->last_time);
        state->frame_count = 0;
        state->last_time = current_time;
        
        char title[256];
        snprintf(title, sizeof(title), 
                 "Contourforge Heightmap Loader - FPS: %.1f | Points: %zu | Lines: %zu",
                 state->fps, state->model->points->count, state->model->lines->count);
        void* window = cf_renderer_get_window(state->renderer);
        if (window) {
            glfwSetWindowTitle((GLFWwindow*)window, title);
        }
    }
}

/**
 * @brief 打印高度图统计信息
 */
void print_heightmap_stats(const cf_heightmap_t* heightmap) {
    printf("\n高度图信息:\n");
    printf("  尺寸: %dx%d\n", heightmap->width, heightmap->height);
    printf("  总像素: %d\n", heightmap->width * heightmap->height);
    printf("  高度范围: %.2f - %.2f\n", heightmap->min_height, heightmap->max_height);
    printf("  高度差: %.2f\n", heightmap->max_height - heightmap->min_height);
    
    // 计算平均高度
    float sum = 0.0f;
    int count = heightmap->width * heightmap->height;
    for (int i = 0; i < count; i++) {
        sum += heightmap->data[i];
    }
    float avg = sum / count;
    printf("  平均高度: %.2f\n", avg);
}

/**
 * @brief 打印模型统计信息
 */
void print_model_stats(const cf_model_t* model, size_t original_points, double gen_time) {
    printf("\n等高线模型:\n");
    printf("  顶点数: %zu\n", model->points->count);
    printf("  线段数: %zu\n", model->lines->count);
    
    if (original_points > 0) {
        float reduction = 100.0f * (1.0f - (float)model->points->count / (float)original_points);
        printf("  简化率: %.1f%% (原始: %zu)\n", reduction, original_points);
    }
    
    printf("  生成时间: %.2f ms\n", gen_time * 1000.0);
    printf("  边界盒:\n");
    printf("    Min: (%.2f, %.2f, %.2f)\n", 
           model->bounds.min.x, model->bounds.min.y, model->bounds.min.z);
    printf("    Max: (%.2f, %.2f, %.2f)\n",
           model->bounds.max.x, model->bounds.max.y, model->bounds.max.z);
}

/**
 * @brief 打印帮助信息
 */
void print_help(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Contourforge Heightmap Loader\n");
    printf("  Phase 2 - 数据生成测试\n");
    printf("========================================\n");
    printf("\n");
    printf("功能：\n");
    printf("  - 加载灰度图并生成3D高度场\n");
    printf("  - 提取等高线\n");
    printf("  - 简化线段（Douglas-Peucker算法）\n");
    printf("  - 渲染等高线模型\n");
    printf("  - 显示详细统计信息\n");
    printf("\n");
    printf("控制：\n");
    printf("  鼠标左键拖动  - 旋转视角\n");
    printf("  鼠标滚轮      - 缩放\n");
    printf("  ESC           - 退出\n");
    printf("\n");
}

int main(int argc, char** argv) {
    cf_result_t result;
    loader_state_t state = {0};
    
    printf("Contourforge v%s\n", cf_get_version());
    print_help();
    
    // 检查命令行参数
    if (argc < 2) {
        printf("用法: %s <heightmap.png> [interval]\n", argv[0]);
        printf("示例: %s ../data/heightmaps/terrain_peaks.png 10.0\n", argv[0]);
        printf("\n可用的测试图像:\n");
        printf("  - gradient_simple.png   (简单渐变)\n");
        printf("  - gradient_radial.png   (径向渐变)\n");
        printf("  - terrain_peaks.png     (山峰地形)\n");
        printf("  - wave_pattern.png      (波浪图案)\n");
        printf("  - terrain_large.png     (大型地形 512x512)\n");
        return 1;
    }
    
    const char* heightmap_path = argv[1];
    float interval = 10.0f;
    if (argc >= 3) {
        interval = (float)atof(argv[2]);
        if (interval <= 0.0f) {
            fprintf(stderr, "错误: 等高线间隔必须大于0\n");
            return 1;
        }
    }
    
    // 1. 加载高度图
    printf("步骤 1/4: 加载高度图...\n");
    printf("  文件: %s\n", heightmap_path);
    
    // 检测格式
    cf_heightmap_format_t format = cf_heightmap_detect_format(heightmap_path);
    printf("  检测格式: %s\n", cf_heightmap_format_name(format));
    
    clock_t start_time = clock();
    result = cf_heightmap_load(heightmap_path, &state.heightmap);
    clock_t end_time = clock();
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 加载高度图失败 (code: %d)\n", result);
        fprintf(stderr, "请确保文件路径正确且文件为支持的格式。\n");
        fprintf(stderr, "支持的格式: PNG, JPEG, BMP, TIFF, GeoTIFF\n");
        return 1;
    }
    
    double load_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("  加载时间: %.2f ms\n", load_time * 1000.0);
    print_heightmap_stats(state.heightmap);
    
    // 2. 生成等高线
    printf("\n步骤 2/4: 生成等高线...\n");
    printf("  等高线间隔: %.1f\n", interval);
    printf("  简化容差: 1.0\n");
    
    cf_contour_config_t config = {
        .interval = interval,
        .min_height = state.heightmap->min_height,
        .max_height = state.heightmap->max_height,
        .simplify_tolerance = 1.0f,
        .build_topology = true
    };
    
    int expected_levels = (int)((config.max_height - config.min_height) / interval) + 1;
    printf("  预计等高线层数: %d\n", expected_levels);
    
    start_time = clock();
    result = cf_contour_generate(state.heightmap, &config, &state.model);
    end_time = clock();
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 生成等高线失败 (code: %d)\n", result);
        cf_heightmap_destroy(state.heightmap);
        return 1;
    }
    
    state.generation_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    print_model_stats(state.model, 0, state.generation_time);
    
    // 3. 初始化渲染器
    printf("\n步骤 3/4: 初始化渲染器...\n");
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge Heightmap Loader",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.15f, 1.0f}
    };
    
    result = cf_renderer_init(&renderer_config, &state.renderer);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 初始化渲染器失败 (code: %d)\n", result);
        cf_model_destroy(state.model);
        cf_heightmap_destroy(state.heightmap);
        return 1;
    }
    
    result = cf_renderer_set_model(state.renderer, state.model);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 设置模型失败 (code: %d)\n", result);
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        cf_heightmap_destroy(state.heightmap);
        return 1;
    }
    
    printf("  窗口大小: %dx%d\n", renderer_config.width, renderer_config.height);
    
    // 4. 设置相机
    printf("\n步骤 4/4: 设置相机...\n");
    state.camera = cf_renderer_get_camera(state.renderer);
    if (!state.camera) {
        fprintf(stderr, "错误: 获取相机失败\n");
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        cf_heightmap_destroy(state.heightmap);
        return 1;
    }
    
    // 根据模型大小自动调整相机距离
    cf_bounds_t bounds = state.model->bounds;
    float size_x = bounds.max.x - bounds.min.x;
    float size_y = bounds.max.y - bounds.min.y;
    float size_z = bounds.max.z - bounds.min.z;
    float max_size = fmaxf(fmaxf(size_x, size_y), size_z);
    
    state.camera_distance = max_size * 2.0f;
    state.camera_angle_h = (float)M_PI * 0.25f;
    state.camera_angle_v = (float)M_PI * 0.3f;
    update_camera(&state);
    
    int width, height;
    cf_renderer_get_size(state.renderer, &width, &height);
    float aspect = (float)width / (float)height;
    cf_camera_set_perspective(state.camera, 45.0f, aspect, 0.1f, max_size * 10.0f);
    
    printf("  相机距离: %.1f\n", state.camera_distance);
    printf("  模型尺寸: %.1f x %.1f x %.1f\n", size_x, size_y, size_z);
    
    // 设置回调
    void* window = cf_renderer_get_window(state.renderer);
    if (window) {
        glfwSetWindowUserPointer((GLFWwindow*)window, &state);
        glfwSetScrollCallback((GLFWwindow*)window, scroll_callback);
    }
    
    // 初始化FPS统计
    state.last_time = glfwGetTime();
    state.frame_count = 0;
    state.fps = 0.0f;
    
    printf("\n渲染循环开始...\n");
    printf("----------------------------------------\n");
    
    // 5. 渲染循环
    while (!cf_renderer_should_close(state.renderer)) {
        handle_mouse_input(&state);
        update_fps(&state);
        
        cf_renderer_begin_frame(state.renderer);
        cf_renderer_render(state.renderer);
        cf_renderer_end_frame(state.renderer);
    }
    
    // 6. 清理
    printf("\n清理资源...\n");
    cf_renderer_destroy(state.renderer);
    cf_model_destroy(state.model);
    cf_heightmap_destroy(state.heightmap);
    
    printf("程序正常退出。\n");
    return 0;
}
