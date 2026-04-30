/**
 * @file interactive_editor.c
 * @brief 交互式等高线编辑器示例
 * 
 * 这个示例展示如何：
 * 1. 加载和渲染等高线
 * 2. 处理鼠标和键盘输入
 * 3. 选择和编辑节点
 * 4. 撤销/重做操作
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    cf_renderer_t* renderer;
    cf_model_t* model;
    cf_selector_t* selector;
    cf_editor_t* editor;
    cf_input_state_t input;
    cf_index_t selected_point;
    bool is_dragging;
} editor_state_t;

void print_help(void)
{
    printf("\nControls:\n");
    printf("  Left Mouse: Select point\n");
    printf("  Left Mouse + Drag: Move point\n");
    printf("  Right Mouse: Rotate view\n");
    printf("  Scroll: Zoom in/out\n");
    printf("  Ctrl+Z: Undo\n");
    printf("  Ctrl+Y: Redo\n");
    printf("  Delete: Delete selected point\n");
    printf("  ESC: Exit\n\n");
}

void handle_input(editor_state_t* state)
{
    cf_input_update(state->renderer, &state->input);
    
    // 左键点击 - 选择点
    if (state->input.mouse_buttons[CF_MOUSE_BUTTON_LEFT] && !state->is_dragging) {
        cf_index_t index;
        cf_result_t result = cf_selector_pick_point(
            state->selector,
            state->input.mouse_x,
            state->input.mouse_y,
            5.0f,  // 选择半径
            &index
        );
        
        if (result == CF_SUCCESS) {
            state->selected_point = index;
            state->is_dragging = true;
            printf("Selected point %u\n", index);
        }
    }
    
    // 拖拽点
    if (state->is_dragging && state->input.mouse_buttons[CF_MOUSE_BUTTON_LEFT]) {
        // TODO: 将屏幕坐标转换为世界坐标
        // 这里需要实现射线投射到平面的逻辑
        // cf_point3_t new_pos = screen_to_world(...);
        // cf_editor_move_point(state->editor, state->selected_point, new_pos);
    }
    
    // 释放鼠标
    if (!state->input.mouse_buttons[CF_MOUSE_BUTTON_LEFT]) {
        state->is_dragging = false;
    }
    
    // 撤销 (Ctrl+Z)
    if (state->input.keys[CF_KEY_CTRL] && state->input.keys[CF_KEY_Z]) {
        if (cf_editor_can_undo(state->editor)) {
            cf_editor_undo(state->editor);
            printf("Undo\n");
        }
    }
    
    // 重做 (Ctrl+Y)
    if (state->input.keys[CF_KEY_CTRL] && state->input.keys['Y']) {
        if (cf_editor_can_redo(state->editor)) {
            cf_editor_redo(state->editor);
            printf("Redo\n");
        }
    }
    
    // 删除选中的点
    if (state->input.keys[127] && state->selected_point != (cf_index_t)-1) { // Delete key
        cf_editor_delete_point(state->editor, state->selected_point);
        printf("Deleted point %u\n", state->selected_point);
        state->selected_point = (cf_index_t)-1;
    }
}

int main(int argc, char** argv)
{
    printf("Contourforge Interactive Editor v%s\n", cf_get_version());
    printf("========================================\n\n");
    
    // 检查命令行参数
    if (argc < 2) {
        printf("Usage: %s <heightmap.png>\n", argv[0]);
        printf("Example: %s ../data/heightmaps/terrain.png\n", argv[0]);
        return 1;
    }
    
    const char* heightmap_path = argv[1];
    
    // 加载高度图并生成等高线
    printf("Loading heightmap: %s\n", heightmap_path);
    cf_heightmap_t* heightmap = NULL;
    cf_result_t result = cf_heightmap_load(heightmap_path, &heightmap);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to load heightmap (code: %d)\n", result);
        return 1;
    }
    
    printf("Generating contours...\n");
    cf_contour_config_t config = {
        .interval = 10.0f,
        .min_height = heightmap->min_height,
        .max_height = heightmap->max_height,
        .simplify_tolerance = 1.0f,
        .build_topology = true
    };
    
    cf_model_t* model = NULL;
    result = cf_contour_generate(heightmap, &config, &model);
    cf_heightmap_destroy(heightmap);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to generate contours (code: %d)\n", result);
        return 1;
    }
    
    printf("  Points: %zu\n", model->points->count);
    printf("  Lines: %zu\n", model->lines->count);
    
    // 初始化渲染器
    printf("\nInitializing renderer...\n");
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge Interactive Editor",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.15f, 1.0f}
    };
    
    cf_renderer_t* renderer = NULL;
    result = cf_renderer_init(&renderer_config, &renderer);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "Error: Failed to initialize renderer (code: %d)\n", result);
        cf_model_destroy(model);
        return 1;
    }
    
    cf_renderer_set_model(renderer, model);
    
    // 创建选择器和编辑器
    cf_selector_t* selector = NULL;
    cf_selector_create(model, renderer, &selector);
    
    cf_editor_t* editor = NULL;
    cf_editor_create(model, 100, &editor);  // 100层撤销
    
    // 初始化编辑器状态
    editor_state_t state = {
        .renderer = renderer,
        .model = model,
        .selector = selector,
        .editor = editor,
        .selected_point = (cf_index_t)-1,
        .is_dragging = false
    };
    
    print_help();
    
    // 渲染循环
    printf("Starting editor...\n");
    while (!cf_renderer_should_close(renderer)) {
        handle_input(&state);
        
        cf_renderer_begin_frame(renderer);
        cf_renderer_render(renderer);
        cf_renderer_end_frame(renderer);
    }
    
    // 清理
    printf("\nCleaning up...\n");
    cf_editor_destroy(editor);
    cf_selector_destroy(selector);
    cf_renderer_destroy(renderer);
    cf_model_destroy(model);
    
    printf("Done.\n");
    return 0;
}
