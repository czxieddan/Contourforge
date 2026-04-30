/**
 * @file editor.c
 * @brief 交互编辑模块实现
 */

#include "../../include/contourforge/control.h"
#include "../../include/contourforge/core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief 编辑操作记录
 */
typedef struct cf_edit_record {
    cf_edit_type_t type;
    cf_index_t point_index;
    cf_point3_t old_position;
    cf_point3_t new_position;
    struct cf_edit_record* next;
} cf_edit_record_t;

/**
 * @brief 编辑器结构
 */
struct cf_editor {
    cf_model_t* model;
    size_t max_undo_levels;
    cf_edit_record_t* undo_stack;
    cf_edit_record_t* redo_stack;
    size_t undo_count;
    size_t redo_count;
};

/**
 * @brief 创建编辑记录
 */
static cf_edit_record_t* create_edit_record(
    cf_edit_type_t type,
    cf_index_t point_index,
    cf_point3_t old_pos,
    cf_point3_t new_pos
) {
    cf_edit_record_t* record = (cf_edit_record_t*)malloc(sizeof(cf_edit_record_t));
    if (!record) {
        return NULL;
    }
    
    record->type = type;
    record->point_index = point_index;
    record->old_position = old_pos;
    record->new_position = new_pos;
    record->next = NULL;
    
    return record;
}

/**
 * @brief 清空编辑记录栈
 */
static void clear_edit_stack(cf_edit_record_t** stack) {
    cf_edit_record_t* current = *stack;
    while (current) {
        cf_edit_record_t* next = current->next;
        free(current);
        current = next;
    }
    *stack = NULL;
}

/**
 * @brief 推入编辑记录
 */
static void push_edit_record(
    cf_editor_t* editor,
    cf_edit_record_t* record
) {
    record->next = editor->undo_stack;
    editor->undo_stack = record;
    editor->undo_count++;
    
    // 限制栈大小
    if (editor->undo_count > editor->max_undo_levels) {
        // 移除最旧的记录
        cf_edit_record_t* current = editor->undo_stack;
        cf_edit_record_t* prev = NULL;
        
        while (current->next) {
            prev = current;
            current = current->next;
        }
        
        if (prev) {
            prev->next = NULL;
            free(current);
            editor->undo_count--;
        }
    }
    
    // 清空重做栈
    clear_edit_stack(&editor->redo_stack);
    editor->redo_count = 0;
}

/**
 * @brief 创建编辑器
 */
cf_result_t cf_editor_create(
    cf_model_t* model,
    size_t max_undo_levels,
    cf_editor_t** editor
) {
    if (!model || !editor) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_editor_t* ed = (cf_editor_t*)malloc(sizeof(cf_editor_t));
    if (!ed) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    ed->model = model;
    ed->max_undo_levels = max_undo_levels > 0 ? max_undo_levels : 100;
    ed->undo_stack = NULL;
    ed->redo_stack = NULL;
    ed->undo_count = 0;
    ed->redo_count = 0;
    
    *editor = ed;
    return CF_SUCCESS;
}

/**
 * @brief 移动点
 */
cf_result_t cf_editor_move_point(
    cf_editor_t* editor,
    cf_index_t point_index,
    cf_point3_t new_position
) {
    if (!editor || !editor->model || !editor->model->points) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_point_set_t* ps = editor->model->points;
    
    if (point_index >= ps->count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 保存旧位置
    cf_point3_t old_position = ps->points[point_index];
    
    // 创建编辑记录
    cf_edit_record_t* record = create_edit_record(
        CF_EDIT_MOVE_POINT,
        point_index,
        old_position,
        new_position
    );
    
    if (!record) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 更新点位置
    ps->points[point_index] = new_position;
    ps->dirty = true;
    
    // 更新边界盒
    cf_model_update_bounds(editor->model);
    
    // 推入撤销栈
    push_edit_record(editor, record);
    
    return CF_SUCCESS;
}

/**
 * @brief 删除点
 */
cf_result_t cf_editor_delete_point(
    cf_editor_t* editor,
    cf_index_t point_index
) {
    if (!editor || !editor->model || !editor->model->points) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_point_set_t* ps = editor->model->points;
    
    if (point_index >= ps->count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 保存旧位置
    cf_point3_t old_position = ps->points[point_index];
    
    // 创建编辑记录
    cf_edit_record_t* record = create_edit_record(
        CF_EDIT_DELETE_POINT,
        point_index,
        old_position,
        (cf_point3_t){0, 0, 0}
    );
    
    if (!record) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 删除点（简化实现：标记为无效）
    // 实际应该重新组织数组并更新所有线段的索引
    ps->points[point_index].x = INFINITY;
    ps->points[point_index].y = INFINITY;
    ps->points[point_index].z = INFINITY;
    ps->dirty = true;
    
    // 推入撤销栈
    push_edit_record(editor, record);
    
    return CF_SUCCESS;
}

/**
 * @brief 添加点
 */
cf_result_t cf_editor_add_point(
    cf_editor_t* editor,
    cf_point3_t position,
    cf_index_t* out_index
) {
    if (!editor || !editor->model || !editor->model->points || !out_index) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 添加点
    cf_index_t index = cf_point_set_add(editor->model->points, position);
    
    // 创建编辑记录
    cf_edit_record_t* record = create_edit_record(
        CF_EDIT_ADD_POINT,
        index,
        (cf_point3_t){0, 0, 0},
        position
    );
    
    if (!record) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 推入撤销栈
    push_edit_record(editor, record);
    
    *out_index = index;
    return CF_SUCCESS;
}

/**
 * @brief 撤销
 */
cf_result_t cf_editor_undo(cf_editor_t* editor) {
    if (!editor || !editor->undo_stack) {
        return CF_ERROR_NOT_FOUND;
    }
    
    // 弹出撤销栈
    cf_edit_record_t* record = editor->undo_stack;
    editor->undo_stack = record->next;
    editor->undo_count--;
    
    // 执行撤销操作
    cf_point_set_t* ps = editor->model->points;
    
    switch (record->type) {
        case CF_EDIT_MOVE_POINT:
            if (record->point_index < ps->count) {
                ps->points[record->point_index] = record->old_position;
                ps->dirty = true;
            }
            break;
            
        case CF_EDIT_DELETE_POINT:
            if (record->point_index < ps->count) {
                ps->points[record->point_index] = record->old_position;
                ps->dirty = true;
            }
            break;
            
        case CF_EDIT_ADD_POINT:
            // 删除添加的点
            if (record->point_index < ps->count) {
                ps->points[record->point_index].x = INFINITY;
                ps->points[record->point_index].y = INFINITY;
                ps->points[record->point_index].z = INFINITY;
                ps->dirty = true;
            }
            break;
            
        default:
            break;
    }
    
    // 推入重做栈
    record->next = editor->redo_stack;
    editor->redo_stack = record;
    editor->redo_count++;
    
    // 更新边界盒
    cf_model_update_bounds(editor->model);
    
    return CF_SUCCESS;
}

/**
 * @brief 重做
 */
cf_result_t cf_editor_redo(cf_editor_t* editor) {
    if (!editor || !editor->redo_stack) {
        return CF_ERROR_NOT_FOUND;
    }
    
    // 弹出重做栈
    cf_edit_record_t* record = editor->redo_stack;
    editor->redo_stack = record->next;
    editor->redo_count--;
    
    // 执行重做操作
    cf_point_set_t* ps = editor->model->points;
    
    switch (record->type) {
        case CF_EDIT_MOVE_POINT:
            if (record->point_index < ps->count) {
                ps->points[record->point_index] = record->new_position;
                ps->dirty = true;
            }
            break;
            
        case CF_EDIT_DELETE_POINT:
            if (record->point_index < ps->count) {
                ps->points[record->point_index].x = INFINITY;
                ps->points[record->point_index].y = INFINITY;
                ps->points[record->point_index].z = INFINITY;
                ps->dirty = true;
            }
            break;
            
        case CF_EDIT_ADD_POINT:
            if (record->point_index < ps->count) {
                ps->points[record->point_index] = record->new_position;
                ps->dirty = true;
            }
            break;
            
        default:
            break;
    }
    
    // 推入撤销栈
    record->next = editor->undo_stack;
    editor->undo_stack = record;
    editor->undo_count++;
    
    // 更新边界盒
    cf_model_update_bounds(editor->model);
    
    return CF_SUCCESS;
}

/**
 * @brief 检查是否可以撤销
 */
bool cf_editor_can_undo(const cf_editor_t* editor) {
    return editor && editor->undo_stack != NULL;
}

/**
 * @brief 检查是否可以重做
 */
bool cf_editor_can_redo(const cf_editor_t* editor) {
    return editor && editor->redo_stack != NULL;
}

/**
 * @brief 销毁编辑器
 */
void cf_editor_destroy(cf_editor_t* editor) {
    if (editor) {
        clear_edit_stack(&editor->undo_stack);
        clear_edit_stack(&editor->redo_stack);
        free(editor);
    }
}
