/**
 * @file lod_generator.c
 * @brief LOD生成器实现
 */

#include "contourforge/core.h"
#include "contourforge/types.h"
#include "contourforge/datagen.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* ========== 辅助函数 ========== */

/**
 * @brief 计算两点之间的距离
 */
static float point_distance(cf_point3_t p1, cf_point3_t p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

/**
 * @brief 计算点的曲率（使用相邻点）
 */
static float calculate_curvature(
    const cf_point3_t* points,
    size_t index,
    size_t count
) {
    if (index == 0 || index >= count - 1) {
        return FLT_MAX;  // 边界点保持高优先级
    }
    
    cf_point3_t prev = points[index - 1];
    cf_point3_t curr = points[index];
    cf_point3_t next = points[index + 1];
    
    // 计算两个向量
    float v1x = curr.x - prev.x;
    float v1y = curr.y - prev.y;
    float v1z = curr.z - prev.z;
    
    float v2x = next.x - curr.x;
    float v2y = next.y - curr.y;
    float v2z = next.z - curr.z;
    
    // 归一化
    float len1 = sqrtf(v1x * v1x + v1y * v1y + v1z * v1z);
    float len2 = sqrtf(v2x * v2x + v2y * v2y + v2z * v2z);
    
    if (len1 < 1e-6f || len2 < 1e-6f) {
        return 0.0f;
    }
    
    v1x /= len1; v1y /= len1; v1z /= len1;
    v2x /= len2; v2y /= len2; v2z /= len2;
    
    // 计算角度变化（曲率的近似）
    float dot = v1x * v2x + v1y * v2y + v1z * v2z;
    dot = CF_CLAMP(dot, -1.0f, 1.0f);
    
    return acosf(dot);  // 返回角度作为曲率度量
}

/**
 * @brief 均匀采样点
 */
static cf_result_t uniform_sampling(
    const cf_point_set_t* source_points,
    float ratio,
    cf_index_t** out_indices,
    size_t* out_count
) {
    if (ratio <= 0.0f || ratio > 1.0f) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    size_t target_count = (size_t)(source_points->count * ratio);
    if (target_count < 2) target_count = 2;
    
    cf_index_t* indices = (cf_index_t*)malloc(sizeof(cf_index_t) * target_count);
    if (!indices) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 计算采样步长
    float step = (float)(source_points->count - 1) / (float)(target_count - 1);
    
    for (size_t i = 0; i < target_count; i++) {
        size_t idx = (size_t)(i * step + 0.5f);
        if (idx >= source_points->count) {
            idx = source_points->count - 1;
        }
        indices[i] = (cf_index_t)idx;
    }
    
    *out_indices = indices;
    *out_count = target_count;
    
    return CF_SUCCESS;
}

/**
 * @brief 基于重要性的采样
 */
static cf_result_t importance_sampling(
    const cf_point_set_t* source_points,
    float ratio,
    bool preserve_boundaries,
    cf_index_t** out_indices,
    size_t* out_count
) {
    if (ratio <= 0.0f || ratio > 1.0f) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    size_t target_count = (size_t)(source_points->count * ratio);
    if (target_count < 2) target_count = 2;
    
    // 计算每个点的重要性（曲率）
    float* importance = (float*)malloc(sizeof(float) * source_points->count);
    if (!importance) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < source_points->count; i++) {
        importance[i] = calculate_curvature(
            source_points->points,
            i,
            source_points->count
        );
    }
    
    // 创建索引-重要性对
    typedef struct {
        cf_index_t index;
        float importance;
        bool selected;
    } point_importance_t;
    
    point_importance_t* pi = (point_importance_t*)malloc(
        sizeof(point_importance_t) * source_points->count
    );
    if (!pi) {
        free(importance);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < source_points->count; i++) {
        pi[i].index = (cf_index_t)i;
        pi[i].importance = importance[i];
        pi[i].selected = false;
    }
    
    free(importance);
    
    // 始终选择首尾点
    if (preserve_boundaries) {
        pi[0].selected = true;
        pi[source_points->count - 1].selected = true;
    }
    
    // 贪心选择重要性最高的点
    size_t selected_count = preserve_boundaries ? 2 : 0;
    
    while (selected_count < target_count) {
        float max_importance = -1.0f;
        size_t max_idx = 0;
        
        for (size_t i = 0; i < source_points->count; i++) {
            if (!pi[i].selected && pi[i].importance > max_importance) {
                max_importance = pi[i].importance;
                max_idx = i;
            }
        }
        
        pi[max_idx].selected = true;
        selected_count++;
    }
    
    // 收集选中的索引（按原始顺序）
    cf_index_t* indices = (cf_index_t*)malloc(sizeof(cf_index_t) * target_count);
    if (!indices) {
        free(pi);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    size_t out_idx = 0;
    for (size_t i = 0; i < source_points->count && out_idx < target_count; i++) {
        if (pi[i].selected) {
            indices[out_idx++] = pi[i].index;
        }
    }
    
    free(pi);
    
    *out_indices = indices;
    *out_count = out_idx;
    
    return CF_SUCCESS;
}

/**
 * @brief 简化线集
 */
static cf_result_t simplify_lines(
    const cf_line_set_t* source_lines,
    const cf_index_t* point_indices,
    size_t point_count,
    cf_index_t** out_line_indices,
    size_t* out_line_count
) {
    // 创建点索引映射（原始索引 -> 新索引）
    cf_index_t* index_map = (cf_index_t*)malloc(
        sizeof(cf_index_t) * source_lines->point_set->count
    );
    if (!index_map) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 初始化为无效值
    for (size_t i = 0; i < source_lines->point_set->count; i++) {
        index_map[i] = (cf_index_t)-1;
    }
    
    // 建立映射
    for (size_t i = 0; i < point_count; i++) {
        index_map[point_indices[i]] = (cf_index_t)i;
    }
    
    // 统计有效线段数量
    size_t valid_count = 0;
    for (size_t i = 0; i < source_lines->count; i++) {
        cf_index_t p1 = source_lines->lines[i].p1;
        cf_index_t p2 = source_lines->lines[i].p2;
        
        if (index_map[p1] != (cf_index_t)-1 && 
            index_map[p2] != (cf_index_t)-1) {
            valid_count++;
        }
    }
    
    // 分配输出数组（每条线2个索引）
    cf_index_t* line_indices = (cf_index_t*)malloc(
        sizeof(cf_index_t) * valid_count * 2
    );
    if (!line_indices) {
        free(index_map);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 填充有效线段
    size_t out_idx = 0;
    for (size_t i = 0; i < source_lines->count; i++) {
        cf_index_t p1 = source_lines->lines[i].p1;
        cf_index_t p2 = source_lines->lines[i].p2;
        
        if (index_map[p1] != (cf_index_t)-1 && 
            index_map[p2] != (cf_index_t)-1) {
            line_indices[out_idx * 2 + 0] = index_map[p1];
            line_indices[out_idx * 2 + 1] = index_map[p2];
            out_idx++;
        }
    }
    
    free(index_map);
    
    *out_line_indices = line_indices;
    *out_line_count = valid_count;
    
    return CF_SUCCESS;
}

/**
 * @brief 生成单个LOD层级
 */
static cf_result_t generate_lod_level(
    cf_model_t* base_model,
    float ratio,
    bool use_importance,
    bool preserve_boundaries,
    cf_lod_level_t* level
) {
    cf_result_t result;
    
    // 采样点
    if (use_importance) {
        result = importance_sampling(
            base_model->points,
            ratio,
            preserve_boundaries,
            &level->point_indices,
            &level->point_count
        );
    } else {
        result = uniform_sampling(
            base_model->points,
            ratio,
            &level->point_indices,
            &level->point_count
        );
    }
    
    if (CF_FAILED(result)) {
        return result;
    }
    
    // 简化线段
    if (base_model->lines != NULL && base_model->lines->count > 0) {
        result = simplify_lines(
            base_model->lines,
            level->point_indices,
            level->point_count,
            &level->line_indices,
            &level->line_count
        );
        
        if (CF_FAILED(result)) {
            free(level->point_indices);
            return result;
        }
    } else {
        level->line_indices = NULL;
        level->line_count = 0;
    }
    
    return CF_SUCCESS;
}

/* ========== LOD API实现 ========== */

/**
 * @brief 创建LOD模型
 */
cf_result_t cf_lod_create(
    cf_model_t* base_model,
    const cf_lod_config_t* config,
    cf_lod_model_t** lod_model
) {
    if (!base_model || !config || !lod_model) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (config->level_count == 0 || 
        !config->distance_thresholds || 
        !config->simplification_ratios) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 分配LOD模型
    cf_lod_model_t* lod = (cf_lod_model_t*)malloc(sizeof(cf_lod_model_t));
    if (!lod) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    lod->base_model = base_model;
    lod->level_count = config->level_count;
    lod->current_level = 0;
    
    // 分配LOD层级数组
    lod->levels = (cf_lod_level_t*)calloc(
        config->level_count,
        sizeof(cf_lod_level_t)
    );
    if (!lod->levels) {
        free(lod);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 生成各个LOD层级
    for (size_t i = 0; i < config->level_count; i++) {
        lod->levels[i].distance_threshold = config->distance_thresholds[i];
        lod->levels[i].simplification_ratio = config->simplification_ratios[i];
        
        cf_result_t result = generate_lod_level(
            base_model,
            config->simplification_ratios[i],
            config->use_importance_sampling,
            config->preserve_boundaries,
            &lod->levels[i]
        );
        
        if (CF_FAILED(result)) {
            // 清理已生成的层级
            for (size_t j = 0; j < i; j++) {
                free(lod->levels[j].point_indices);
                free(lod->levels[j].line_indices);
            }
            free(lod->levels);
            free(lod);
            return result;
        }
    }
    
    *lod_model = lod;
    return CF_SUCCESS;
}

/**
 * @brief 根据距离选择LOD层级
 */
int cf_lod_select_level(
    const cf_lod_model_t* lod_model,
    float distance
) {
    if (!lod_model || lod_model->level_count == 0) {
        return 0;
    }
    
    // 从最近到最远查找合适的层级
    for (size_t i = 0; i < lod_model->level_count; i++) {
        if (distance < lod_model->levels[i].distance_threshold) {
            return (int)i;
        }
    }
    
    // 超过所有阈值，使用最低细节层级
    return (int)(lod_model->level_count - 1);
}

/**
 * @brief 设置当前LOD层级
 */
cf_result_t cf_lod_set_level(
    cf_lod_model_t* lod_model,
    int level
) {
    if (!lod_model) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (level < 0 || level >= (int)lod_model->level_count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    lod_model->current_level = level;
    return CF_SUCCESS;
}

/**
 * @brief 获取当前LOD层级
 */
int cf_lod_get_current_level(const cf_lod_model_t* lod_model) {
    if (!lod_model) {
        return 0;
    }
    
    return lod_model->current_level;
}

/**
 * @brief 获取LOD统计信息
 */
cf_result_t cf_lod_get_stats(
    const cf_lod_model_t* lod_model,
    cf_lod_stats_t* stats
) {
    if (!lod_model || !stats) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    memset(stats, 0, sizeof(cf_lod_stats_t));
    
    // 原始数据统计
    stats->original_point_count = lod_model->base_model->points->count;
    stats->original_line_count = lod_model->base_model->lines ? 
        lod_model->base_model->lines->count : 0;
    
    // 分配层级统计数组
    stats->level_point_counts = (size_t*)malloc(
        sizeof(size_t) * lod_model->level_count
    );
    stats->level_line_counts = (size_t*)malloc(
        sizeof(size_t) * lod_model->level_count
    );
    stats->reduction_ratios = (float*)malloc(
        sizeof(float) * lod_model->level_count
    );
    
    if (!stats->level_point_counts || 
        !stats->level_line_counts || 
        !stats->reduction_ratios) {
        cf_lod_stats_destroy(stats);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 统计各层级
    size_t total_memory = sizeof(cf_lod_model_t);
    total_memory += sizeof(cf_lod_level_t) * lod_model->level_count;
    
    for (size_t i = 0; i < lod_model->level_count; i++) {
        const cf_lod_level_t* level = &lod_model->levels[i];
        
        stats->level_point_counts[i] = level->point_count;
        stats->level_line_counts[i] = level->line_count;
        stats->reduction_ratios[i] = 
            (float)level->point_count / (float)stats->original_point_count;
        
        total_memory += sizeof(cf_index_t) * level->point_count;
        total_memory += sizeof(cf_index_t) * level->line_count * 2;
    }
    
    stats->total_memory_bytes = total_memory;
    
    return CF_SUCCESS;
}

/**
 * @brief 销毁LOD统计信息
 */
void cf_lod_stats_destroy(cf_lod_stats_t* stats) {
    if (!stats) {
        return;
    }
    
    free(stats->level_point_counts);
    free(stats->level_line_counts);
    free(stats->reduction_ratios);
    
    memset(stats, 0, sizeof(cf_lod_stats_t));
}

/**
 * @brief 销毁LOD模型
 */
void cf_lod_destroy(cf_lod_model_t* lod_model) {
    if (!lod_model) {
        return;
    }
    
    if (lod_model->levels) {
        for (size_t i = 0; i < lod_model->level_count; i++) {
            free(lod_model->levels[i].point_indices);
            free(lod_model->levels[i].line_indices);
        }
        free(lod_model->levels);
    }
    
    free(lod_model);
}
