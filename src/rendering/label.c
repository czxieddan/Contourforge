/**
 * @file label.c
 * @brief 标注系统实现
 */

#include "contourforge/rendering.h"
#include "contourforge/core.h"
#include "contourforge/types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ========== 标注结构 ========== */

/**
 * @brief 标注
 */
struct cf_label {
    cf_point3_t position;      /**< 3D位置 */
    char text[64];             /**< 标注文字 */
    cf_color_t color;          /**< 颜色 */
    float size;                /**< 字体大小 */
    bool visible;              /**< 是否可见 */
    float height;              /**< 等高线高度 */
};

/**
 * @brief 标注管理器
 */
struct cf_label_manager {
    cf_text_renderer_t* text_renderer;  /**< 文字渲染器 */
    cf_label_config_t config;           /**< 配置 */
    cf_label_t* labels;                 /**< 标注数组 */
    size_t label_count;                 /**< 标注数量 */
    size_t label_capacity;              /**< 标注容量 */
    bool initialized;                   /**< 是否已初始化 */
};

/* ========== 辅助函数 ========== */

/**
 * @brief 计算两点之间的距离
 */
static float distance_3d(cf_point3_t a, cf_point3_t b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

/**
 * @brief 格式化高度文字
 */
static void format_height_text(float height, const cf_label_config_t* config, char* buffer, size_t buffer_size) {
    char format[32];
    snprintf(format, sizeof(format), "%%.%df%%s", config->decimal_places);
    snprintf(buffer, buffer_size, format, height, config->unit);
}

/**
 * @brief 检查标注是否与已有标注重叠
 */
static bool check_overlap(cf_point3_t pos, cf_label_t* labels, size_t count, float min_distance) {
    for (size_t i = 0; i < count; i++) {
        if (distance_3d(pos, labels[i].position) < min_distance) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 沿线段放置标注
 */
static void place_labels_on_line(
    cf_point3_t p1,
    cf_point3_t p2,
    float height,
    const cf_label_config_t* config,
    cf_label_t* labels,
    size_t* label_count,
    size_t max_labels,
    float* accumulated_length
) {
    float segment_length = distance_3d(p1, p2);
    
    /* 如果累积长度超过间距，放置标注 */
    while (*accumulated_length >= config->spacing && *label_count < max_labels) {
        /* 计算标注在线段上的位置 */
        float t = (config->spacing - (*accumulated_length - segment_length)) / segment_length;
        
        if (t >= 0.0f && t <= 1.0f) {
            cf_point3_t label_pos;
            label_pos.x = p1.x + t * (p2.x - p1.x);
            label_pos.y = p1.y + t * (p2.y - p1.y);
            label_pos.z = p1.z + t * (p2.z - p1.z);
            
            /* 检查是否与已有标注重叠 */
            if (!check_overlap(label_pos, labels, *label_count, config->spacing * 0.5f)) {
                cf_label_t* label = &labels[*label_count];
                label->position = label_pos;
                label->color = config->color;
                label->size = config->size;
                label->visible = true;
                label->height = height;
                
                /* 格式化文字 */
                format_height_text(height, config, label->text, sizeof(label->text));
                
                (*label_count)++;
            }
        }
        
        *accumulated_length -= config->spacing;
    }
    
    *accumulated_length += segment_length;
}

/* ========== 标注管理器API ========== */

/**
 * @brief 创建标注管理器
 */
cf_result_t cf_label_manager_create(
    cf_text_renderer_t* text_renderer,
    const cf_label_config_t* config,
    cf_label_manager_t** manager
) {
    if (text_renderer == NULL || config == NULL || manager == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_label_manager_t* mgr = (cf_label_manager_t*)malloc(sizeof(cf_label_manager_t));
    if (mgr == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    memset(mgr, 0, sizeof(cf_label_manager_t));
    
    mgr->text_renderer = text_renderer;
    mgr->config = *config;
    mgr->label_capacity = 1000;
    mgr->labels = (cf_label_t*)malloc(sizeof(cf_label_t) * mgr->label_capacity);
    
    if (mgr->labels == NULL) {
        free(mgr);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    mgr->initialized = true;
    *manager = mgr;
    return CF_SUCCESS;
}

/**
 * @brief 为等高线生成标注
 */
cf_result_t cf_label_manager_generate_labels(
    cf_label_manager_t* manager,
    cf_model_t* model,
    cf_camera_t* camera
) {
    if (manager == NULL || model == NULL || !manager->initialized) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 清除现有标注 */
    manager->label_count = 0;
    
    if (model->lines == NULL || model->lines->count == 0) {
        return CF_SUCCESS;
    }
    
    /* 获取相机位置用于LOD计算 */
    cf_point3_t camera_pos = {0, 0, 0};
    if (camera != NULL) {
        camera_pos = cf_camera_get_position(camera);
    }
    
    cf_point3_t model_center = cf_model_get_center(model);
    float camera_distance = distance_3d(camera_pos, model_center);
    
    /* 根据距离调整间距（LOD） */
    float spacing = manager->config.spacing;
    if (camera_distance < 100.0f) {
        spacing = manager->config.spacing;
    } else if (camera_distance < 300.0f) {
        spacing = manager->config.spacing * 2.0f;
    } else if (camera_distance < 600.0f) {
        spacing = manager->config.spacing * 4.0f;
    } else {
        spacing = manager->config.spacing * 8.0f;
    }
    
    /* 临时更新配置 */
    cf_label_config_t temp_config = manager->config;
    temp_config.spacing = spacing;
    
    /* 遍历所有线段，放置标注 */
    float accumulated_length = 0.0f;
    
    for (size_t i = 0; i < model->lines->count && manager->label_count < manager->label_capacity; i++) {
        const cf_line_t* line = &model->lines->lines[i];
        
        /* 获取线段的两个端点 */
        const cf_point3_t* p1 = cf_point_set_get(model->points, line->p1);
        const cf_point3_t* p2 = cf_point_set_get(model->points, line->p2);
        
        if (p1 == NULL || p2 == NULL) {
            continue;
        }
        
        /* 使用Z坐标作为高度 */
        float height = (p1->z + p2->z) * 0.5f;
        
        /* 沿线段放置标注 */
        place_labels_on_line(
            *p1, *p2, height,
            &temp_config,
            manager->labels,
            &manager->label_count,
            manager->label_capacity,
            &accumulated_length
        );
    }
    
    return CF_SUCCESS;
}

/**
 * @brief 更新标注（相机变化时调用）
 */
cf_result_t cf_label_manager_update(
    cf_label_manager_t* manager,
    cf_camera_t* camera
) {
    if (manager == NULL || !manager->initialized) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (camera == NULL) {
        return CF_SUCCESS;
    }
    
    /* 获取相机位置 */
    cf_point3_t camera_pos = cf_camera_get_position(camera);
    
    /* 更新标注可见性（基于距离） */
    for (size_t i = 0; i < manager->label_count; i++) {
        float dist = distance_3d(manager->labels[i].position, camera_pos);
        manager->labels[i].visible = (dist >= manager->config.min_distance && 
                                      dist <= manager->config.max_distance);
    }
    
    return CF_SUCCESS;
}

/**
 * @brief 渲染标注
 */
cf_result_t cf_label_manager_render(
    cf_label_manager_t* manager,
    const float* view_matrix,
    const float* projection_matrix
) {
    if (manager == NULL || !manager->initialized) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (view_matrix == NULL || projection_matrix == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 渲染所有可见标注 */
    for (size_t i = 0; i < manager->label_count; i++) {
        if (manager->labels[i].visible) {
            cf_result_t result = cf_text_renderer_render_3d(
                manager->text_renderer,
                manager->labels[i].text,
                manager->labels[i].position,
                manager->labels[i].color,
                view_matrix,
                projection_matrix
            );
            
            if (CF_FAILED(result)) {
                fprintf(stderr, "Failed to render label %zu\n", i);
            }
        }
    }
    
    return CF_SUCCESS;
}

/**
 * @brief 清除所有标注
 */
void cf_label_manager_clear(cf_label_manager_t* manager) {
    if (manager == NULL) {
        return;
    }
    
    manager->label_count = 0;
}

/**
 * @brief 销毁标注管理器
 */
void cf_label_manager_destroy(cf_label_manager_t* manager) {
    if (manager == NULL) {
        return;
    }
    
    free(manager->labels);
    free(manager);
}
