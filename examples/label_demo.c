/**
 * @file label_demo.c
 * @brief 等高线标注演示程序
 * 
 * 演示如何使用标注系统在3D场景中显示等高线高度值
 */

#include <contourforge/contourforge.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ========== 全局变量 ========== */

static cf_renderer_t* g_renderer = NULL;
static cf_camera_t* g_camera = NULL;
static cf_model_t* g_model = NULL;
static cf_shader_t* g_line_shader = NULL;
static cf_shader_t* g_text_shader = NULL;
static cf_font_t* g_font = NULL;
static cf_text_renderer_t* g_text_renderer = NULL;
static cf_label_manager_t* g_label_manager = NULL;

static bool g_show_labels = true;
static bool g_mouse_pressed = false;
static double g_last_mouse_x = 0.0;
static double g_last_mouse_y = 0.0;

/* ========== 回调函数 ========== */

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
            case GLFW_KEY_L:
                g_show_labels = !g_show_labels;
                printf("Labels: %s\n", g_show_labels ? "ON" : "OFF");
                break;
            case GLFW_KEY_R:
                /* 重新生成标注 */
                if (g_label_manager && g_model && g_camera) {
                    cf_label_manager_generate_labels(g_label_manager, g_model, g_camera);
                    printf("Labels regenerated\n");
                }
                break;
        }
    }
}

/**
 * @brief 鼠标按钮回调
 */
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_mouse_pressed = (action == GLFW_PRESS);
    }
}

/**
 * @brief 鼠标移动回调
 */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    
    if (g_mouse_pressed && g_camera) {
        double dx = xpos - g_last_mouse_x;
        double dy = ypos - g_last_mouse_y;
        
        cf_camera_orbit(g_camera, (float)dx * 0.01f, (float)dy * 0.01f);
        
        /* 更新标注可见性 */
        if (g_label_manager) {
            cf_label_manager_update(g_label_manager, g_camera);
        }
    }
    
    g_last_mouse_x = xpos;
    g_last_mouse_y = ypos;
}

/**
 * @brief 鼠标滚轮回调
 */
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    
    if (g_camera) {
        cf_camera_zoom(g_camera, (float)yoffset * -10.0f);
        
        /* 更新标注（LOD） */
        if (g_label_manager && g_model) {
            cf_label_manager_generate_labels(g_label_manager, g_model, g_camera);
        }
    }
}

/* ========== 主程序 ========== */

int main(int argc, char** argv) {
    printf("=== Contourforge Label Demo ===\n");
    printf("Controls:\n");
    printf("  L - Toggle labels on/off\n");
    printf("  R - Regenerate labels\n");
    printf("  Mouse drag - Rotate camera\n");
    printf("  Mouse wheel - Zoom\n");
    printf("  ESC - Exit\n\n");
    
    /* 解析命令行参数 */
    const char* heightmap_path = "data/heightmaps/terrain_peaks.png";
    if (argc > 1) {
        heightmap_path = argv[1];
    }
    
    /* 初始化渲染器 */
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge - Label Demo",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.15f, 1.0f}
    };
    
    cf_result_t result = cf_renderer_init(&renderer_config, &g_renderer);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }
    
    /* 设置回调 */
    GLFWwindow* window = (GLFWwindow*)cf_renderer_get_window(g_renderer);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    /* 加载着色器 */
    result = cf_shader_load("shaders/line.vert", "shaders/line.frag", &g_line_shader);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to load line shader\n");
        cf_renderer_destroy(g_renderer);
        return 1;
    }
    
    result = cf_shader_load("shaders/text.vert", "shaders/text.frag", &g_text_shader);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to load text shader\n");
        cf_shader_destroy(g_line_shader);
        cf_renderer_destroy(g_renderer);
        return 1;
    }
    
    cf_renderer_set_shader(g_renderer, g_line_shader);
    
    /* 加载字体 */
    result = cf_font_load("data/fonts/default.ttf", 24.0f, &g_font);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to load font\n");
        cf_shader_destroy(g_text_shader);
        cf_shader_destroy(g_line_shader);
        cf_renderer_destroy(g_renderer);
        return 1;
    }
    
    /* 创建文字渲染器 */
    result = cf_text_renderer_create(g_font, g_text_shader, &g_text_renderer);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to create text renderer\n");
        cf_font_destroy(g_font);
        cf_shader_destroy(g_text_shader);
        cf_shader_destroy(g_line_shader);
        cf_renderer_destroy(g_renderer);
        return 1;
    }
    
    /* 加载高度图 */
    printf("Loading heightmap: %s\n", heightmap_path);
    cf_heightmap_t* heightmap;
    result = cf_heightmap_load(heightmap_path, &heightmap);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to load heightmap\n");
        cf_text_renderer_destroy(g_text_renderer);
        cf_font_destroy(g_font);
        cf_shader_destroy(g_text_shader);
        cf_shader_destroy(g_line_shader);
        cf_renderer_destroy(g_renderer);
        return 1;
    }
    
    /* 生成等高线 */
    printf("Generating contours...\n");
    cf_contour_config_t contour_config = {
        .interval = 10.0f,
        .min_height = 0.0f,
        .max_height = 255.0f
    };
    
    result = cf_contour_generate(heightmap, &contour_config, &g_model);
    cf_heightmap_destroy(heightmap);
    
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to generate contours\n");
        cf_text_renderer_destroy(g_text_renderer);
        cf_font_destroy(g_font);
        cf_shader_destroy(g_text_shader);
        cf_shader_destroy(g_line_shader);
        cf_renderer_destroy(g_renderer);
        return 1;
    }
    
    printf("Generated %zu lines\n", g_model->lines->count);
    
    cf_renderer_set_model(g_renderer, g_model);
    
    /* 设置相机 */
    g_camera = cf_renderer_get_camera(g_renderer);
    cf_point3_t center = cf_model_get_center(g_model);
    cf_vec3_t size = cf_model_get_size(g_model);
    float max_dim = fmaxf(fmaxf(size.x, size.y), size.z);
    
    cf_point3_t camera_pos = {
        center.x,
        center.y - max_dim * 1.5f,
        center.z + max_dim * 1.0f
    };
    
    cf_camera_set_position(g_camera, camera_pos);
    cf_camera_set_target(g_camera, center);
    
    /* 创建标注管理器 */
    cf_label_config_t label_config = {
        .spacing = 50.0f,
        .min_distance = 10.0f,
        .max_distance = 1000.0f,
        .lod_levels = 4,
        .unit = "m",
        .decimal_places = 1,
        .color = {1.0f, 1.0f, 0.0f, 1.0f},  /* 黄色 */
        .size = 24.0f,
        .show_index = false
    };
    
    result = cf_label_manager_create(g_text_renderer, &label_config, &g_label_manager);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to create label manager\n");
        cf_model_destroy(g_model);
        cf_text_renderer_destroy(g_text_renderer);
        cf_font_destroy(g_font);
        cf_shader_destroy(g_text_shader);
        cf_shader_destroy(g_line_shader);
        cf_renderer_destroy(g_renderer);
        return 1;
    }
    
    /* 生成标注 */
    printf("Generating labels...\n");
    result = cf_label_manager_generate_labels(g_label_manager, g_model, g_camera);
    if (CF_FAILED(result)) {
        fprintf(stderr, "Failed to generate labels\n");
    }
    
    /* 主循环 */
    printf("Entering main loop...\n");
    while (!cf_renderer_should_close(g_renderer)) {
        cf_renderer_begin_frame(g_renderer);
        
        /* 渲染等高线 */
        cf_renderer_render(g_renderer);
        
        /* 渲染标注 */
        if (g_show_labels) {
            const float* view = cf_camera_get_view_matrix(g_camera);
            const float* proj = cf_camera_get_projection_matrix(g_camera);
            cf_label_manager_render(g_label_manager, view, proj);
        }
        
        cf_renderer_end_frame(g_renderer);
    }
    
    /* 清理 */
    printf("Cleaning up...\n");
    cf_label_manager_destroy(g_label_manager);
    cf_model_destroy(g_model);
    cf_text_renderer_destroy(g_text_renderer);
    cf_font_destroy(g_font);
    cf_shader_destroy(g_text_shader);
    cf_shader_destroy(g_line_shader);
    cf_renderer_destroy(g_renderer);
    
    printf("Done!\n");
    return 0;
}
