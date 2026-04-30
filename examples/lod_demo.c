/**
 * @file lod_demo.c
 * @brief LOD系统演示程序
 */

#include <contourforge/contourforge.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 全局状态 */
static struct {
    cf_renderer_t* renderer;
    cf_lod_model_t* lod_model;
    cf_shader_t* shader;
    
    bool auto_lod;
    int manual_lod_level;
    bool show_stats;
    
    double last_mouse_x;
    double last_mouse_y;
    bool first_mouse;
    bool mouse_dragging;
} g_state = {
    .auto_lod = true,
    .manual_lod_level = 0,
    .show_stats = true,
    .first_mouse = true,
    .mouse_dragging = false
};

/**
 * @brief 鼠标按钮回调
 */
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    (void)window;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_state.mouse_dragging = (action == GLFW_PRESS);
        g_state.first_mouse = true;
    }
}

/**
 * @brief 鼠标移动回调
 */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    
    if (g_state.first_mouse) {
        g_state.last_mouse_x = xpos;
        g_state.last_mouse_y = ypos;
        g_state.first_mouse = false;
        return;
    }
    
    if (g_state.mouse_dragging) {
        double dx = xpos - g_state.last_mouse_x;
        double dy = ypos - g_state.last_mouse_y;
        
        cf_camera_t* camera = cf_renderer_get_camera(g_state.renderer);
        cf_camera_orbit(camera, (float)dx * 0.005f, (float)dy * 0.005f);
    }
    
    g_state.last_mouse_x = xpos;
    g_state.last_mouse_y = ypos;
}

/**
 * @brief 鼠标滚轮回调
 */
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    
    cf_camera_t* camera = cf_renderer_get_camera(g_state.renderer);
    cf_camera_zoom(camera, (float)yoffset * -0.1f);
}

/**
 * @brief 键盘回调
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
                
            case GLFW_KEY_A:
                /* 切换自动LOD */
                g_state.auto_lod = !g_state.auto_lod;
                cf_renderer_set_auto_lod(g_state.renderer, g_state.auto_lod);
                printf("自动LOD: %s\n", g_state.auto_lod ? "开启" : "关闭");
                break;
                
            case GLFW_KEY_1:
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
            case GLFW_KEY_5:
                /* 手动选择LOD层级 */
                if (!g_state.auto_lod) {
                    int level = key - GLFW_KEY_1;
                    if (level < (int)g_state.lod_model->level_count) {
                        g_state.manual_lod_level = level;
                        cf_lod_set_level(g_state.lod_model, level);
                        printf("手动LOD层级: %d\n", level);
                    }
                }
                break;
                
            case GLFW_KEY_S:
                /* 切换统计信息显示 */
                g_state.show_stats = !g_state.show_stats;
                break;
                
            case GLFW_KEY_D:
                /* 切换LOD调试模式 */
                {
                    static bool debug = false;
                    debug = !debug;
                    cf_renderer_set_lod_debug(g_state.renderer, debug);
                    printf("LOD调试模式: %s\n", debug ? "开启" : "关闭");
                }
                break;
        }
    }
}

/**
 * @brief 打印统计信息
 */
static void print_stats(void) {
    if (!g_state.show_stats) {
        return;
    }
    
    cf_lod_stats_t stats;
    if (cf_lod_get_stats(g_state.lod_model, &stats) == CF_SUCCESS) {
        int current_level = cf_lod_get_current_level(g_state.lod_model);
        
        printf("\n========== LOD统计信息 ==========\n");
        printf("原始数据: %zu 点, %zu 线\n", 
               stats.original_point_count, 
               stats.original_line_count);
        printf("当前LOD层级: %d\n", current_level);
        printf("总内存占用: %.2f KB\n", stats.total_memory_bytes / 1024.0f);
        
        printf("\n各层级详情:\n");
        for (size_t i = 0; i < g_state.lod_model->level_count; i++) {
            printf("  LOD %zu: %zu 点 (%.1f%%), %zu 线, 距离阈值: %.1f\n",
                   i,
                   stats.level_point_counts[i],
                   stats.reduction_ratios[i] * 100.0f,
                   stats.level_line_counts[i],
                   g_state.lod_model->levels[i].distance_threshold);
        }
        printf("================================\n\n");
        
        cf_lod_stats_destroy(&stats);
    }
}

/**
 * @brief 创建测试地形
 */
static cf_model_t* create_test_terrain(void) {
    cf_model_t* model = NULL;
    if (cf_model_create("LOD Test Terrain", &model) != CF_SUCCESS) {
        return NULL;
    }
    
    /* 创建一个复杂的地形网格 */
    const int grid_size = 100;
    const float spacing = 0.2f;
    const float amplitude = 5.0f;
    
    /* 生成点 */
    for (int z = 0; z < grid_size; z++) {
        for (int x = 0; x < grid_size; x++) {
            float fx = (x - grid_size / 2) * spacing;
            float fz = (z - grid_size / 2) * spacing;
            
            /* 使用多个正弦波创建复杂地形 */
            float y = amplitude * (
                sinf(fx * 0.5f) * cosf(fz * 0.5f) +
                0.5f * sinf(fx * 1.0f + fz * 1.0f) +
                0.25f * sinf(fx * 2.0f) * sinf(fz * 2.0f)
            );
            
            cf_point3_t point = {fx, y, fz};
            cf_point_set_add(model->points, point);
        }
    }
    
    /* 生成线段（网格） */
    for (int z = 0; z < grid_size; z++) {
        for (int x = 0; x < grid_size; x++) {
            cf_index_t idx = z * grid_size + x;
            
            /* 水平线 */
            if (x < grid_size - 1) {
                cf_line_set_add(model->lines, idx, idx + 1);
            }
            
            /* 垂直线 */
            if (z < grid_size - 1) {
                cf_line_set_add(model->lines, idx, idx + grid_size);
            }
        }
    }
    
    cf_model_update_bounds(model);
    
    printf("创建测试地形: %zu 点, %zu 线\n", 
           model->points->count, 
           model->lines->count);
    
    return model;
}

/**
 * @brief 主函数
 */
int main(void) {
    printf("=== Contourforge LOD系统演示 ===\n\n");
    printf("控制说明:\n");
    printf("  鼠标左键拖拽: 旋转视角\n");
    printf("  鼠标滚轮: 缩放\n");
    printf("  A: 切换自动/手动LOD\n");
    printf("  1-5: 手动选择LOD层级（需先关闭自动LOD）\n");
    printf("  S: 切换统计信息显示\n");
    printf("  D: 切换LOD调试模式\n");
    printf("  ESC: 退出\n\n");
    
    /* 初始化渲染器 */
    cf_renderer_config_t config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge LOD Demo",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.15f, 1.0f}
    };
    
    if (cf_renderer_init(&config, &g_state.renderer) != CF_SUCCESS) {
        fprintf(stderr, "无法初始化渲染器\n");
        return 1;
    }
    
    /* 加载着色器 */
    if (cf_shader_load("shaders/line.vert", "shaders/line.frag", &g_state.shader) != CF_SUCCESS) {
        fprintf(stderr, "无法加载着色器\n");
        cf_renderer_destroy(g_state.renderer);
        return 1;
    }
    
    cf_renderer_set_shader(g_state.renderer, g_state.shader);
    
    /* 创建测试地形 */
    cf_model_t* base_model = create_test_terrain();
    if (!base_model) {
        fprintf(stderr, "无法创建测试地形\n");
        cf_shader_destroy(g_state.shader);
        cf_renderer_destroy(g_state.renderer);
        return 1;
    }
    
    /* 配置LOD */
    float distances[] = {20.0f, 40.0f, 60.0f, 80.0f, 100.0f};
    float ratios[] = {1.0f, 0.5f, 0.25f, 0.125f, 0.0625f};
    
    cf_lod_config_t lod_config = {
        .level_count = 5,
        .distance_thresholds = distances,
        .simplification_ratios = ratios,
        .preserve_boundaries = true,
        .use_importance_sampling = true
    };
    
    /* 创建LOD模型 */
    printf("生成LOD层级...\n");
    if (cf_lod_create(base_model, &lod_config, &g_state.lod_model) != CF_SUCCESS) {
        fprintf(stderr, "无法创建LOD模型\n");
        cf_model_destroy(base_model);
        cf_shader_destroy(g_state.shader);
        cf_renderer_destroy(g_state.renderer);
        return 1;
    }
    
    /* 设置LOD模型到渲染器 */
    cf_renderer_set_lod_model(g_state.renderer, g_state.lod_model);
    cf_renderer_set_auto_lod(g_state.renderer, true);
    
    /* 打印初始统计信息 */
    print_stats();
    
    /* 设置相机 */
    cf_camera_t* camera = cf_renderer_get_camera(g_state.renderer);
    cf_point3_t cam_pos = {0.0f, 15.0f, 30.0f};
    cf_point3_t cam_target = {0.0f, 0.0f, 0.0f};
    cf_camera_set_position(camera, cam_pos);
    cf_camera_set_target(camera, cam_target);
    
    /* 设置回调 */
    GLFWwindow* window = (GLFWwindow*)cf_renderer_get_window(g_state.renderer);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    /* 主循环 */
    double last_time = glfwGetTime();
    int frame_count = 0;
    
    while (!cf_renderer_should_close(g_state.renderer)) {
        /* 计算FPS */
        double current_time = glfwGetTime();
        frame_count++;
        
        if (current_time - last_time >= 1.0) {
            char title[256];
            int current_level = cf_lod_get_current_level(g_state.lod_model);
            snprintf(title, sizeof(title), 
                     "Contourforge LOD Demo - FPS: %d, LOD Level: %d, Auto: %s",
                     frame_count, current_level, g_state.auto_lod ? "ON" : "OFF");
            glfwSetWindowTitle(window, title);
            
            frame_count = 0;
            last_time = current_time;
        }
        
        /* 渲染 */
        cf_renderer_begin_frame(g_state.renderer);
        cf_renderer_render(g_state.renderer);
        cf_renderer_end_frame(g_state.renderer);
    }
    
    /* 清理 */
    cf_lod_destroy(g_state.lod_model);
    cf_model_destroy(base_model);
    cf_shader_destroy(g_state.shader);
    cf_renderer_destroy(g_state.renderer);
    
    printf("\n程序正常退出\n");
    return 0;
}
