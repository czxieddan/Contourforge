/**
 * @file simple_viewer.c
 * @brief 简单的3D点云查看器 - Phase 1测试程序
 * 
 * 功能：
 * 1. 手动创建测试点（立方体的8个顶点）
 * 2. 使用渲染库显示这些点
 * 3. 支持相机旋转、缩放
 * 4. 验证核心库和渲染库的基本功能
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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 全局状态
typedef struct {
    cf_renderer_t* renderer;
    cf_model_t* model;
    cf_camera_t* camera;
    
    // 相机控制
    double last_mouse_x;
    double last_mouse_y;
    bool mouse_dragging;
    float camera_distance;
    float camera_angle_h;  // 水平角度
    float camera_angle_v;  // 垂直角度
    
    // 统计信息
    double last_time;
    int frame_count;
    float fps;
} viewer_state_t;

/**
 * @brief 创建立方体测试数据
 */
cf_result_t create_cube_model(cf_model_t** out_model) {
    cf_result_t result;
    
    // 创建模型
    result = cf_model_create("Test Cube", out_model);
    if (result != CF_SUCCESS) {
        return result;
    }
    
    cf_model_t* model = *out_model;
    
    // 立方体的8个顶点（边长为2，中心在原点）
    cf_point3_t vertices[8] = {
        {-1.0f, -1.0f, -1.0f},  // 0: 左下后
        { 1.0f, -1.0f, -1.0f},  // 1: 右下后
        { 1.0f,  1.0f, -1.0f},  // 2: 右上后
        {-1.0f,  1.0f, -1.0f},  // 3: 左上后
        {-1.0f, -1.0f,  1.0f},  // 4: 左下前
        { 1.0f, -1.0f,  1.0f},  // 5: 右下前
        { 1.0f,  1.0f,  1.0f},  // 6: 右上前
        {-1.0f,  1.0f,  1.0f}   // 7: 左上前
    };
    
    // 添加顶点到点集
    for (int i = 0; i < 8; i++) {
        cf_point_set_add(model->points, vertices[i]);
    }
    
    // 立方体的12条边
    int edges[12][2] = {
        // 后面4条边
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        // 前面4条边
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        // 连接前后的4条边
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    
    // 添加边到线集
    for (int i = 0; i < 12; i++) {
        cf_line_set_add(model->lines, edges[i][0], edges[i][1]);
    }
    
    // 更新边界盒
    cf_model_update_bounds(model);
    
    printf("创建立方体模型:\n");
    printf("  顶点数: %zu\n", model->points->count);
    printf("  边数: %zu\n", model->lines->count);
    printf("  边界: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f)\n",
           model->bounds.min.x, model->bounds.min.y, model->bounds.min.z,
           model->bounds.max.x, model->bounds.max.y, model->bounds.max.z);
    
    return CF_SUCCESS;
}

/**
 * @brief 更新相机位置
 */
void update_camera(viewer_state_t* state) {
    // 球面坐标转换为笛卡尔坐标
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
void handle_mouse_input(viewer_state_t* state) {
    void* window = cf_renderer_get_window(state->renderer);
    if (!window) return;
    
    // 获取鼠标位置
    double mouse_x, mouse_y;
    glfwGetCursorPos((GLFWwindow*)window, &mouse_x, &mouse_y);
    
    // 检查鼠标按钮状态
    int left_button = glfwGetMouseButton((GLFWwindow*)window, GLFW_MOUSE_BUTTON_LEFT);
    
    if (left_button == GLFW_PRESS) {
        if (!state->mouse_dragging) {
            // 开始拖动
            state->mouse_dragging = true;
            state->last_mouse_x = mouse_x;
            state->last_mouse_y = mouse_y;
        } else {
            // 计算鼠标移动量
            double delta_x = mouse_x - state->last_mouse_x;
            double delta_y = mouse_y - state->last_mouse_y;
            
            // 更新相机角度
            state->camera_angle_h += (float)delta_x * 0.005f;
            state->camera_angle_v -= (float)delta_y * 0.005f;
            
            // 限制垂直角度
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
 * @brief 处理滚轮输入（缩放）
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    viewer_state_t* state = (viewer_state_t*)glfwGetWindowUserPointer(window);
    if (!state) return;
    
    // 缩放相机距离
    state->camera_distance *= (1.0f - (float)yoffset * 0.1f);
    
    // 限制距离范围
    if (state->camera_distance < 2.0f) state->camera_distance = 2.0f;
    if (state->camera_distance > 50.0f) state->camera_distance = 50.0f;
    
    update_camera(state);
}

/**
 * @brief 更新FPS统计
 */
void update_fps(viewer_state_t* state) {
    double current_time = glfwGetTime();
    state->frame_count++;
    
    if (current_time - state->last_time >= 1.0) {
        state->fps = (float)state->frame_count / (float)(current_time - state->last_time);
        state->frame_count = 0;
        state->last_time = current_time;
        
        // 更新窗口标题显示FPS
        char title[256];
        snprintf(title, sizeof(title), "Contourforge Simple Viewer - FPS: %.1f", state->fps);
        void* window = cf_renderer_get_window(state->renderer);
        if (window) {
            glfwSetWindowTitle((GLFWwindow*)window, title);
        }
    }
}

/**
 * @brief 打印帮助信息
 */
void print_help(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Contourforge Simple Viewer\n");
    printf("  Phase 1 - 基础渲染测试\n");
    printf("========================================\n");
    printf("\n");
    printf("功能：\n");
    printf("  - 显示立方体线框模型（8个顶点，12条边）\n");
    printf("  - 验证核心库和渲染库的基本功能\n");
    printf("\n");
    printf("控制：\n");
    printf("  鼠标左键拖动  - 旋转视角\n");
    printf("  鼠标滚轮      - 缩放\n");
    printf("  ESC           - 退出\n");
    printf("\n");
}

int main(void) {
    cf_result_t result;
    viewer_state_t state = {0};
    
    // 打印版本信息
    printf("Contourforge v%s\n", cf_get_version());
    print_help();
    
    // 1. 创建测试模型（立方体）
    printf("步骤 1/3: 创建测试模型...\n");
    result = create_cube_model(&state.model);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 创建模型失败 (code: %d)\n", result);
        return 1;
    }
    
    // 2. 初始化渲染器
    printf("\n步骤 2/3: 初始化渲染器...\n");
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge Simple Viewer",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.15f, 1.0f}
    };
    
    result = cf_renderer_init(&renderer_config, &state.renderer);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 初始化渲染器失败 (code: %d)\n", result);
        cf_model_destroy(state.model);
        return 1;
    }
    printf("  窗口大小: %dx%d\n", renderer_config.width, renderer_config.height);
    printf("  MSAA: %dx\n", renderer_config.msaa_samples);
    
    // 设置模型
    result = cf_renderer_set_model(state.renderer, state.model);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 设置模型失败 (code: %d)\n", result);
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        return 1;
    }
    
    // 3. 设置相机
    printf("\n步骤 3/3: 设置相机...\n");
    state.camera = cf_renderer_get_camera(state.renderer);
    if (!state.camera) {
        fprintf(stderr, "错误: 获取相机失败\n");
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        return 1;
    }
    
    // 初始化相机参数
    state.camera_distance = 5.0f;
    state.camera_angle_h = (float)M_PI * 0.25f;  // 45度
    state.camera_angle_v = (float)M_PI * 0.2f;   // 36度
    update_camera(&state);
    
    // 设置透视投影
    int width, height;
    cf_renderer_get_size(state.renderer, &width, &height);
    float aspect = (float)width / (float)height;
    cf_camera_set_perspective(state.camera, 45.0f, aspect, 0.1f, 100.0f);
    
    printf("  相机距离: %.1f\n", state.camera_distance);
    printf("  视野角度: 45°\n");
    
    // 设置滚轮回调
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
    
    // 4. 渲染循环
    while (!cf_renderer_should_close(state.renderer)) {
        // 处理输入
        handle_mouse_input(&state);
        
        // 更新FPS
        update_fps(&state);
        
        // 渲染
        cf_renderer_begin_frame(state.renderer);
        cf_renderer_render(state.renderer);
        cf_renderer_end_frame(state.renderer);
    }
    
    // 5. 清理
    printf("\n清理资源...\n");
    cf_renderer_destroy(state.renderer);
    cf_model_destroy(state.model);
    
    printf("程序正常退出。\n");
    return 0;
}
