/**
 * @file interactive_editor.c
 * @brief 交互式等高线编辑器 - Phase 3测试程序
 * 
 * 功能：
 * 1. 加载高度图生成模型
 * 2. 支持鼠标选择节点
 * 3. 支持拖动节点移动
 * 4. 支持在线上添加新节点
 * 5. 支持撤销/重做
 * 6. 实时渲染更新
 * 7. 验证所有库的集成功能
 * 
 * 控制：
 * - 鼠标左键：选择/拖动节点
 * - 鼠标右键：旋转视角
 * - 鼠标滚轮：缩放
 * - Ctrl+Z：撤销
 * - Ctrl+Y：重做
 * - Delete：删除选中节点
 * - F1：显示/隐藏帮助
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

// 编辑器状态
typedef struct {
    cf_renderer_t* renderer;
    cf_model_t* model;
    cf_camera_t* camera;
    cf_selector_t* selector;
    cf_editor_t* editor;
    
    // 相机控制
    double last_mouse_x;
    double last_mouse_y;
    bool right_mouse_dragging;
    float camera_distance;
    float camera_angle_h;
    float camera_angle_v;
    
    // 编辑状态
    cf_index_t selected_point;
    bool is_dragging;
    bool show_help;
    
    // 键盘状态
    bool ctrl_pressed;
    bool key_z_pressed;
    bool key_y_pressed;
    bool delete_pressed;
    bool f1_pressed;
    
    // 统计信息
    double last_time;
    int frame_count;
    float fps;
    int edit_count;
} editor_state_t;

// 前向声明
void print_help(void);

/**
 * @brief 更新相机位置
 */
void update_camera(editor_state_t* state) {
    float x = state->camera_distance * cosf(state->camera_angle_v) * sinf(state->camera_angle_h);
    float y = state->camera_distance * sinf(state->camera_angle_v);
    float z = state->camera_distance * cosf(state->camera_angle_v) * cosf(state->camera_angle_h);
    
    cf_point3_t position = {x, y, z};
    cf_point3_t target = {0.0f, 0.0f, 0.0f};
    
    cf_camera_set_position(state->camera, position);
    cf_camera_set_target(state->camera, target);
}

/**
 * @brief 屏幕坐标转世界坐标（简化版，投影到XZ平面）
 */
cf_point3_t screen_to_world(editor_state_t* state, double screen_x, double screen_y) {
    // 这是一个简化的实现，实际应该使用射线投射
    // 这里假设我们在XZ平面上编辑（Y=0）
    
    int width, height;
    cf_renderer_get_size(state->renderer, &width, &height);
    
    // 归一化屏幕坐标到[-1, 1]
    float ndc_x = (float)(screen_x / width) * 2.0f - 1.0f;
    float ndc_y = 1.0f - (float)(screen_y / height) * 2.0f;
    
    // 简化：直接映射到世界坐标
    // 实际应该使用逆投影矩阵
    cf_bounds_t bounds = state->model->bounds;
    float size_x = bounds.max.x - bounds.min.x;
    float size_z = bounds.max.z - bounds.min.z;
    
    cf_point3_t world_pos;
    world_pos.x = ndc_x * size_x * 0.5f;
    world_pos.y = 0.0f;  // 投影到XZ平面
    world_pos.z = ndc_y * size_z * 0.5f;
    
    return world_pos;
}

/**
 * @brief 处理鼠标输入
 */
void handle_mouse_input(editor_state_t* state) {
    void* window = cf_renderer_get_window(state->renderer);
    if (!window) return;
    
    GLFWwindow* glfw_window = (GLFWwindow*)window;
    
    // 获取鼠标位置
    double mouse_x, mouse_y;
    glfwGetCursorPos(glfw_window, &mouse_x, &mouse_y);
    
    // 获取按钮状态
    int left_button = glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT);
    int right_button = glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT);
    
    // 右键拖动 - 旋转视角
    if (right_button == GLFW_PRESS) {
        if (!state->right_mouse_dragging) {
            state->right_mouse_dragging = true;
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
        state->right_mouse_dragging = false;
    }
    
    // 左键 - 选择和拖动
    if (left_button == GLFW_PRESS) {
        if (!state->is_dragging) {
            // 尝试选择点
            cf_index_t index;
            cf_result_t result = cf_selector_pick_point(
                state->selector,
                mouse_x,
                mouse_y,
                10.0f,  // 选择半径（像素）
                &index
            );
            
            if (result == CF_SUCCESS) {
                state->selected_point = index;
                state->is_dragging = true;
                printf("选中节点 #%u\n", index);
            } else {
                state->selected_point = (cf_index_t)-1;
            }
        } else if (state->selected_point != (cf_index_t)-1) {
            // 拖动选中的点
            cf_point3_t new_pos = screen_to_world(state, mouse_x, mouse_y);
            
            // 保持原始高度
            const cf_point3_t* old_pos = cf_point_set_get(state->model->points, state->selected_point);
            if (old_pos) {
                new_pos.y = old_pos->y;
                
                cf_result_t result = cf_editor_move_point(state->editor, state->selected_point, new_pos);
                if (result == CF_SUCCESS) {
                    state->edit_count++;
                    // 标记模型需要更新
                    state->model->points->dirty = true;
                }
            }
        }
    } else {
        state->is_dragging = false;
    }
}

/**
 * @brief 处理键盘输入
 */
void handle_keyboard_input(editor_state_t* state) {
    void* window = cf_renderer_get_window(state->renderer);
    if (!window) return;
    
    GLFWwindow* glfw_window = (GLFWwindow*)window;
    
    // 检查Ctrl键
    bool ctrl_now = (glfwGetKey(glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                     glfwGetKey(glfw_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
    state->ctrl_pressed = ctrl_now;
    
    // Ctrl+Z - 撤销
    bool z_now = (glfwGetKey(glfw_window, GLFW_KEY_Z) == GLFW_PRESS);
    if (z_now && !state->key_z_pressed && state->ctrl_pressed) {
        if (cf_editor_can_undo(state->editor)) {
            cf_editor_undo(state->editor);
            printf("撤销操作 (剩余: %d次编辑)\n", --state->edit_count);
            state->model->points->dirty = true;
        } else {
            printf("无法撤销\n");
        }
    }
    state->key_z_pressed = z_now;
    
    // Ctrl+Y - 重做
    bool y_now = (glfwGetKey(glfw_window, GLFW_KEY_Y) == GLFW_PRESS);
    if (y_now && !state->key_y_pressed && state->ctrl_pressed) {
        if (cf_editor_can_redo(state->editor)) {
            cf_editor_redo(state->editor);
            printf("重做操作 (编辑次数: %d)\n", ++state->edit_count);
            state->model->points->dirty = true;
        } else {
            printf("无法重做\n");
        }
    }
    state->key_y_pressed = y_now;
    
    // Delete - 删除选中的点
    bool delete_now = (glfwGetKey(glfw_window, GLFW_KEY_DELETE) == GLFW_PRESS);
    if (delete_now && !state->delete_pressed && state->selected_point != (cf_index_t)-1) {
        cf_result_t result = cf_editor_delete_point(state->editor, state->selected_point);
        if (result == CF_SUCCESS) {
            printf("删除节点 #%u\n", state->selected_point);
            state->selected_point = (cf_index_t)-1;
            state->edit_count++;
            state->model->points->dirty = true;
        }
    }
    state->delete_pressed = delete_now;
    
    // F1 - 切换帮助显示
    bool f1_now = (glfwGetKey(glfw_window, GLFW_KEY_F1) == GLFW_PRESS);
    if (f1_now && !state->f1_pressed) {
        state->show_help = !state->show_help;
        if (state->show_help) {
            print_help();
        }
    }
    state->f1_pressed = f1_now;
}

/**
 * @brief 滚轮回调
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    editor_state_t* state = (editor_state_t*)glfwGetWindowUserPointer(window);
    if (!state) return;
    
    state->camera_distance *= (1.0f - (float)yoffset * 0.1f);
    
    if (state->camera_distance < 5.0f) state->camera_distance = 5.0f;
    if (state->camera_distance > 200.0f) state->camera_distance = 200.0f;
    
    update_camera(state);
}

/**
 * @brief 更新FPS和窗口标题
 */
void update_fps(editor_state_t* state) {
    double current_time = glfwGetTime();
    state->frame_count++;
    
    if (current_time - state->last_time >= 1.0) {
        state->fps = (float)state->frame_count / (float)(current_time - state->last_time);
        state->frame_count = 0;
        state->last_time = current_time;
        
        char title[256];
        const char* selected_str = (state->selected_point != (cf_index_t)-1) ? "已选中" : "未选中";
        snprintf(title, sizeof(title), 
                 "Contourforge Interactive Editor - FPS: %.1f | Points: %zu | Edits: %d | %s",
                 state->fps, state->model->points->count, state->edit_count, selected_str);
        
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
    printf("  交互式编辑器控制\n");
    printf("========================================\n");
    printf("\n");
    printf("鼠标控制：\n");
    printf("  左键点击      - 选择节点\n");
    printf("  左键拖动      - 移动选中的节点\n");
    printf("  右键拖动      - 旋转视角\n");
    printf("  滚轮          - 缩放\n");
    printf("\n");
    printf("键盘控制：\n");
    printf("  Ctrl+Z        - 撤销\n");
    printf("  Ctrl+Y        - 重做\n");
    printf("  Delete        - 删除选中的节点\n");
    printf("  F1            - 显示/隐藏此帮助\n");
    printf("  ESC           - 退出\n");
    printf("\n");
    printf("提示：\n");
    printf("  - 节点会在鼠标附近高亮显示\n");
    printf("  - 拖动节点时会保持原始高度\n");
    printf("  - 所有编辑操作都支持撤销/重做\n");
    printf("========================================\n");
    printf("\n");
}

int main(int argc, char** argv) {
    cf_result_t result;
    editor_state_t state = {0};
    
    printf("Contourforge v%s\n", cf_get_version());
    printf("\n");
    printf("========================================\n");
    printf("  Contourforge Interactive Editor\n");
    printf("  Phase 3 - 完整集成测试\n");
    printf("========================================\n");
    printf("\n");
    
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
    }
    
    // 初始化状态
    state.selected_point = (cf_index_t)-1;
    state.show_help = false;
    state.edit_count = 0;
    
    // 1. 加载高度图
    printf("步骤 1/5: 加载高度图...\n");
    printf("  文件: %s\n", heightmap_path);
    
    cf_heightmap_t* heightmap = NULL;
    result = cf_heightmap_load(heightmap_path, &heightmap);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 加载高度图失败 (code: %d)\n", result);
        return 1;
    }
    
    printf("  尺寸: %dx%d\n", heightmap->width, heightmap->height);
    printf("  高度范围: %.2f - %.2f\n", heightmap->min_height, heightmap->max_height);
    
    // 2. 生成等高线
    printf("\n步骤 2/5: 生成等高线...\n");
    printf("  等高线间隔: %.1f\n", interval);
    
    cf_contour_config_t config = {
        .interval = interval,
        .min_height = heightmap->min_height,
        .max_height = heightmap->max_height,
        .simplify_tolerance = 1.0f,
        .build_topology = true
    };
    
    clock_t start_time = clock();
    result = cf_contour_generate(heightmap, &config, &state.model);
    clock_t end_time = clock();
    
    cf_heightmap_destroy(heightmap);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 生成等高线失败 (code: %d)\n", result);
        return 1;
    }
    
    double gen_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("  顶点数: %zu\n", state.model->points->count);
    printf("  线段数: %zu\n", state.model->lines->count);
    printf("  生成时间: %.2f ms\n", gen_time * 1000.0);
    
    // 3. 初始化渲染器
    printf("\n步骤 3/5: 初始化渲染器...\n");
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge Interactive Editor",
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
    
    result = cf_renderer_set_model(state.renderer, state.model);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 设置模型失败 (code: %d)\n", result);
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        return 1;
    }
    
    // 4. 创建选择器和编辑器
    printf("\n步骤 4/5: 初始化编辑器...\n");
    
    result = cf_selector_create(state.model, state.renderer, &state.selector);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 创建选择器失败 (code: %d)\n", result);
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        return 1;
    }
    printf("  选择器已创建\n");
    
    result = cf_editor_create(state.model, 100, &state.editor);
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 创建编辑器失败 (code: %d)\n", result);
        cf_selector_destroy(state.selector);
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        return 1;
    }
    printf("  编辑器已创建 (撤销层数: 100)\n");
    
    // 5. 设置相机
    printf("\n步骤 5/5: 设置相机...\n");
    state.camera = cf_renderer_get_camera(state.renderer);
    if (!state.camera) {
        fprintf(stderr, "错误: 获取相机失败\n");
        cf_editor_destroy(state.editor);
        cf_selector_destroy(state.selector);
        cf_renderer_destroy(state.renderer);
        cf_model_destroy(state.model);
        return 1;
    }
    
    // 自动调整相机
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
    
    printf("\n编辑器已就绪！\n");
    printf("按 F1 显示帮助信息\n");
    printf("----------------------------------------\n");
    
    // 6. 主循环
    while (!cf_renderer_should_close(state.renderer)) {
        handle_mouse_input(&state);
        handle_keyboard_input(&state);
        update_fps(&state);
        
        cf_renderer_begin_frame(state.renderer);
        cf_renderer_render(state.renderer);
        cf_renderer_end_frame(state.renderer);
    }
    
    // 7. 清理
    printf("\n清理资源...\n");
    cf_editor_destroy(state.editor);
    cf_selector_destroy(state.selector);
    cf_renderer_destroy(state.renderer);
    cf_model_destroy(state.model);
    
    printf("程序正常退出。总编辑次数: %d\n", state.edit_count);
    return 0;
}
