/**
 * @file control.h
 * @brief Contourforge控制模块API
 * @version 1.0.0
 */

#ifndef CF_CONTROL_H
#define CF_CONTROL_H

#include "types.h"
#include "core.h"
#include "rendering.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 输入处理 ========== */

/**
 * @brief 鼠标按钮
 */
typedef enum {
    CF_MOUSE_BUTTON_LEFT = 0,
    CF_MOUSE_BUTTON_RIGHT = 1,
    CF_MOUSE_BUTTON_MIDDLE = 2
} cf_mouse_button_t;

/**
 * @brief 键盘按键
 */
typedef enum {
    CF_KEY_W = 87,
    CF_KEY_A = 65,
    CF_KEY_S = 83,
    CF_KEY_D = 68,
    CF_KEY_Z = 90,
    CF_KEY_Y = 89,
    CF_KEY_SPACE = 32,
    CF_KEY_SHIFT = 340,
    CF_KEY_CTRL = 341,
    CF_KEY_ESC = 256,
    CF_KEY_DELETE = 127
} cf_key_t;

/**
 * @brief 输入状态
 */
typedef struct {
    double mouse_x;             /**< 鼠标X坐标 */
    double mouse_y;             /**< 鼠标Y坐标 */
    double mouse_delta_x;       /**< 鼠标X偏移 */
    double mouse_delta_y;       /**< 鼠标Y偏移 */
    bool mouse_buttons[8];      /**< 鼠标按钮状态 */
    bool keys[512];             /**< 键盘按键状态 */
} cf_input_state_t;

/**
 * @brief 更新输入状态
 * @param renderer 渲染器
 * @param input 输入状态
 */
void cf_input_update(cf_renderer_t* renderer, cf_input_state_t* input);

/* ========== 节点选择 ========== */

/**
 * @brief 选择器句柄
 */
typedef struct cf_selector cf_selector_t;

/**
 * @brief 创建选择器
 * @param model 模型
 * @param renderer 渲染器
 * @param selector 输出选择器指针
 * @return 返回码
 */
cf_result_t cf_selector_create(
    cf_model_t* model,
    cf_renderer_t* renderer,
    cf_selector_t** selector
);

/**
 * @brief 选择点（射线投射）
 * @param selector 选择器
 * @param screen_x 屏幕X坐标
 * @param screen_y 屏幕Y坐标
 * @param radius 选择半径（世界空间）
 * @param out_index 输出点索引
 * @return 返回码（CF_ERROR_NOT_FOUND表示未选中）
 */
cf_result_t cf_selector_pick_point(
    cf_selector_t* selector,
    double screen_x,
    double screen_y,
    float radius,
    cf_index_t* out_index
);

/**
 * @brief 选择线段
 * @param selector 选择器
 * @param screen_x 屏幕X坐标
 * @param screen_y 屏幕Y坐标
 * @param radius 选择半径（世界空间）
 * @param out_index 输出线段索引
 * @return 返回码（CF_ERROR_NOT_FOUND表示未选中）
 */
cf_result_t cf_selector_pick_line(
    cf_selector_t* selector,
    double screen_x,
    double screen_y,
    float radius,
    cf_index_t* out_index
);

/**
 * @brief 框选点
 * @param selector 选择器
 * @param x1 起始X坐标
 * @param y1 起始Y坐标
 * @param x2 结束X坐标
 * @param y2 结束Y坐标
 * @param indices 输出点索引数组
 * @param count 输出点数量
 * @return 返回码
 */
cf_result_t cf_selector_box_select(
    cf_selector_t* selector,
    double x1,
    double y1,
    double x2,
    double y2,
    cf_index_t** indices,
    size_t* count
);

/**
 * @brief 销毁选择器
 * @param selector 选择器
 */
void cf_selector_destroy(cf_selector_t* selector);

/* ========== 编辑器 ========== */

/**
 * @brief 编辑器句柄
 */
typedef struct cf_editor cf_editor_t;

/**
 * @brief 编辑操作类型
 */
typedef enum {
    CF_EDIT_MOVE_POINT,         /**< 移动点 */
    CF_EDIT_DELETE_POINT,       /**< 删除点 */
    CF_EDIT_ADD_POINT,          /**< 添加点 */
    CF_EDIT_SPLIT_LINE,         /**< 分割线段 */
    CF_EDIT_MERGE_LINES         /**< 合并线段 */
} cf_edit_type_t;

/**
 * @brief 创建编辑器
 * @param model 模型
 * @param max_undo_levels 最大撤销层数
 * @param editor 输出编辑器指针
 * @return 返回码
 */
cf_result_t cf_editor_create(
    cf_model_t* model,
    size_t max_undo_levels,
    cf_editor_t** editor
);

/**
 * @brief 移动点
 * @param editor 编辑器
 * @param point_index 点索引
 * @param new_position 新位置
 * @return 返回码
 */
cf_result_t cf_editor_move_point(
    cf_editor_t* editor,
    cf_index_t point_index,
    cf_point3_t new_position
);

/**
 * @brief 删除点
 * @param editor 编辑器
 * @param point_index 点索引
 * @return 返回码
 */
cf_result_t cf_editor_delete_point(
    cf_editor_t* editor,
    cf_index_t point_index
);

/**
 * @brief 添加点
 * @param editor 编辑器
 * @param position 位置
 * @param out_index 输出点索引
 * @return 返回码
 */
cf_result_t cf_editor_add_point(
    cf_editor_t* editor,
    cf_point3_t position,
    cf_index_t* out_index
);

/**
 * @brief 撤销
 * @param editor 编辑器
 * @return 返回码
 */
cf_result_t cf_editor_undo(cf_editor_t* editor);

/**
 * @brief 重做
 * @param editor 编辑器
 * @return 返回码
 */
cf_result_t cf_editor_redo(cf_editor_t* editor);

/**
 * @brief 检查是否可以撤销
 * @param editor 编辑器
 * @return true=可以撤销
 */
bool cf_editor_can_undo(const cf_editor_t* editor);

/**
 * @brief 检查是否可以重做
 * @param editor 编辑器
 * @return true=可以重做
 */
bool cf_editor_can_redo(const cf_editor_t* editor);

/**
 * @brief 销毁编辑器
 * @param editor 编辑器
 */
void cf_editor_destroy(cf_editor_t* editor);

#ifdef __cplusplus
}
#endif

#endif /* CF_CONTROL_H */
