/**
 * @file lod_parallel.c
 * @brief 并行LOD生成实现
 */

#include "contourforge/core.h"
#include "contourforge/types.h"
#include "contourforge/datagen.h"
#include "contourforge/threading.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* ========== 并行采样任务 ========== */

typedef struct {
    const cf_point_set_t* source_points;
    float ratio;
    bool preserve_boundaries;
    
    // 输出
    cf_index_t* indices;
    size_t count;
    cf_result_t result;
} lod_sampling_task_t;

/**
 * @brief 计算点的曲率
 */
static float calculate_curvature(
    const cf_point3_t* points,
    size_t index,
    size_t count
) {
    if (index == 0 || index >= count - 1) {
        return FLT_MAX;
    }
    
    cf_point3_t prev = points[index - 1];
    cf_point3_t curr = points[index];
    cf_point3_t next = points[index + 1];
    
    float v1x = curr.x - prev.x;
    float v1y = curr.y - prev.y;
    float v1z = curr.z - prev.z;
    
    float v2x = next.x - curr.x;
    float v2y = next.y - curr.y;
    float v2z = next.z - curr.z;
    
    float len1 = sqrtf(v1x * v1x + v1y * v1y + v1z * v1z);
    float len2 = sqrtf(v2x * v2x + v2y * v2y + v2z * v2z);
    
    if (len1 < 1e-6f || len2 < 1e-6f) {
        return 0.0f;
    }
    
    v1x /= len1; v1y /= len1; v1z /= len1;
    v2x /= len2; v2y /= len2; v2z /= len2;
    
    float dot = v1x * v2x + v1y * v2y + v1z * v2z;
    dot = (dot < -1.0f) ? -1.0f : ((dot > 1.0f) ? 1.0f : dot);
    
    return acosf(dot);
}

/**
 * @brief 并行重要性采样工作函数
 */
static void importance_sampling_worker(void* arg) {
    lod_sampling_task_t* task = (lod_sampling_task_t*)arg;
    
    const cf_point_set_t* source = task->source_points;
    size_t target_count = (size_t)(source->count * task->ratio);
    if (target_count < 2) target_count = 2;
    
    // 计算每个点的重要性
    float* importance = (float*)malloc(sizeof(float) * source->count);
    if (!importance) {
        task->result = CF_ERROR_OUT_OF_MEMORY;
        return;
    }
    
    for (size_t i = 0; i < source->count; i++) {
        importance[i] = calculate_curvature(source->points, i, source->count);
    }
    
    // 创建索引-重要性对
    typedef struct {
        cf_index_t index;
        float importance;
        bool selected;
    } point_importance_t;
    
    point_importance_t* pi = (point_importance_t*)malloc(
        sizeof(point_importance_t) * source->count
    );
    if (!pi) {
        free(importance);
        task->result = CF_ERROR_OUT_OF_MEMORY;
        return;
    }
    
    for (size_t i = 0; i < source->count; i++) {
        pi[i].index = (cf_index_t)i;
        pi[i].importance = importance[i];
        pi[i].selected = false;
    }
    
    free(importance);
    
    // 始终选择首尾点
    if (task->preserve_boundaries) {
        pi[0].selected = true;
        pi[source->count - 1].selected = true;
    }
    
    // 贪心选择重要性最高的点
    size_t selected_count = task->preserve_boundaries ? 2 : 0;
    
    while (selected_count < target_count) {
        float max_importance = -1.0f;
        size_t max_idx = 0;
        
        for (size_t i = 0; i < source->count; i++) {
            if (!pi[i].selected && pi[i].importance > max_importance) {
                max_importance = pi[i].importance;
                max_idx = i;
            }
        }
        
        pi[max_idx].selected = true;
        selected_count++;
    }
    
    // 收集选中的索引
    task->indices = (cf_index_t*)malloc(sizeof(cf_index_t) * target_count);
    if (!task->indices) {
        free(pi);
        task->result = CF_ERROR_OUT_OF_MEMORY;
        return;
    }
    
    size_t out_idx = 0;
    for (size_t i = 0; i < source->count && out_idx < target_count; i++) {
        if (pi[i].selected) {
            task->indices[out_idx++] = pi[i].index;
        }
    }
    
    task->count = target_count;
    task->result = CF_SUCCESS;
    
    free(pi);
}

/**
 * @brief 并行创建LOD模型
 */
cf_result_t cf_lod_create_parallel(
    cf_model_t* base_model,
    const cf_lod_config_t* config,
    cf_thread_pool_t* thread_pool,
    cf_lod_model_t** lod_model
) {
    if (!base_model || !config || !lod_model) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 如果没有线程池，回退到单线程
    if (!thread_pool) {
        return CF_ERROR_INVALID_PARAM;  // 需要实现单线程版本
    }
    
    // 创建LOD模型结构
    cf_lod_model_t* lod = (cf_lod_model_t*)malloc(sizeof(cf_lod_model_t));
    if (!lod) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    lod->base_model = base_model;
    lod->level_count = config->level_count;
    lod->current_level = 0;
    lod->levels = (cf_lod_level_t*)malloc(sizeof(cf_lod_level_t) * lod->level_count);
    if (!lod->levels) {
        free(lod);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 第0级是原始模型（全部点和线）
    lod->levels[0].point_count = base_model->points->count;
    lod->levels[0].point_indices = (cf_index_t*)malloc(sizeof(cf_index_t) * lod->levels[0].point_count);
    for (size_t i = 0; i < lod->levels[0].point_count; i++) {
        lod->levels[0].point_indices[i] = (cf_index_t)i;
    }
    lod->levels[0].line_count = base_model->lines->count;
    lod->levels[0].line_indices = NULL;  // 简化：不复制线索引
    lod->levels[0].distance_threshold = 0.0f;
    lod->levels[0].simplification_ratio = 1.0f;
    
    // 并行生成其他LOD级别
    lod_sampling_task_t* tasks = (lod_sampling_task_t*)malloc(
        sizeof(lod_sampling_task_t) * (lod->level_count - 1)
    );
    if (!tasks) {
        free(lod->levels[0].point_indices);
        free(lod->levels);
        free(lod);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 初始化任务
    for (size_t i = 1; i < lod->level_count; i++) {
        float ratio = config->simplification_ratios[i];
        
        tasks[i - 1].source_points = base_model->points;
        tasks[i - 1].ratio = ratio;
        tasks[i - 1].preserve_boundaries = config->preserve_boundaries;
        tasks[i - 1].indices = NULL;
        tasks[i - 1].count = 0;
        tasks[i - 1].result = CF_SUCCESS;
    }
    
    // 提交任务
    for (size_t i = 0; i < lod->level_count - 1; i++) {
        cf_result_t result = cf_thread_pool_submit(
            thread_pool,
            importance_sampling_worker,
            &tasks[i]
        );
        if (result != CF_SUCCESS) {
            cf_thread_pool_wait(thread_pool);
            for (size_t j = 0; j < i; j++) {
                free(tasks[j].indices);
            }
            free(tasks);
            free(lod->levels[0].point_indices);
            free(lod->levels);
            free(lod);
            return result;
        }
    }
    
    // 等待完成
    cf_thread_pool_wait(thread_pool);
    
    // 检查错误并构建LOD层级
    cf_result_t final_result = CF_SUCCESS;
    for (size_t i = 1; i < lod->level_count; i++) {
        if (tasks[i - 1].result != CF_SUCCESS) {
            final_result = tasks[i - 1].result;
            break;
        }
        
        // 设置LOD层级
        lod->levels[i].point_count = tasks[i - 1].count;
        lod->levels[i].point_indices = tasks[i - 1].indices;
        lod->levels[i].line_count = 0;  // 简化：不处理线
        lod->levels[i].line_indices = NULL;
        lod->levels[i].distance_threshold = config->distance_thresholds[i];
        lod->levels[i].simplification_ratio = config->simplification_ratios[i];
    }
    
    // 清理任务数组（但不释放indices，它们已被LOD层级使用）
    free(tasks);
    
    if (final_result != CF_SUCCESS) {
        // 清理已分配的资源
        for (size_t i = 0; i < lod->level_count; i++) {
            free(lod->levels[i].point_indices);
        }
        free(lod->levels);
        free(lod);
        return final_result;
    }
    
    *lod_model = lod;
    return CF_SUCCESS;
}
