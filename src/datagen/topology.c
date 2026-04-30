/**
 * @file topology.c
 * @brief 拓扑构建模块实现
 */

#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief 构建点到线的拓扑关系
 */
static cf_result_t build_point_to_line_topology(cf_model_t* model) {
    if (!model || !model->points || !model->lines) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 这里可以构建一个哈希表或邻接表
    // 记录每个点连接的线段
    // 暂时简化实现，后续可以优化
    
    return CF_SUCCESS;
}

/**
 * @brief 合并重复的点
 */
static cf_result_t merge_duplicate_points(cf_model_t* model, float tolerance) {
    if (!model || !model->points) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_point_set_t* ps = model->points;
    size_t count = ps->count;
    
    if (count < 2) {
        return CF_SUCCESS;
    }
    
    // 创建映射表（旧索引 -> 新索引）
    cf_index_t* remap = (cf_index_t*)malloc(sizeof(cf_index_t) * count);
    if (!remap) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 初始化映射
    for (size_t i = 0; i < count; i++) {
        remap[i] = (cf_index_t)i;
    }
    
    float tol_sq = tolerance * tolerance;
    
    // 简单的O(n^2)算法查找重复点
    // 对于大规模数据，应该使用空间哈希或八叉树
    for (size_t i = 0; i < count; i++) {
        if (remap[i] != i) continue; // 已经被合并
        
        cf_point3_t p1 = ps->points[i];
        
        for (size_t j = i + 1; j < count; j++) {
            if (remap[j] != j) continue;
            
            cf_point3_t p2 = ps->points[j];
            
            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float dz = p2.z - p1.z;
            float dist_sq = dx * dx + dy * dy + dz * dz;
            
            if (dist_sq < tol_sq) {
                remap[j] = (cf_index_t)i; // 合并到i
            }
        }
    }
    
    // 更新线段的点索引
    if (model->lines) {
        for (size_t i = 0; i < model->lines->count; i++) {
            model->lines->lines[i].p1 = remap[model->lines->lines[i].p1];
            model->lines->lines[i].p2 = remap[model->lines->lines[i].p2];
        }
    }
    
    free(remap);
    
    return CF_SUCCESS;
}

/**
 * @brief 构建拓扑关系
 */
cf_result_t cf_topology_build(cf_model_t* model) {
    if (!model) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 1. 合并重复的点
    cf_result_t result = merge_duplicate_points(model, 0.001f);
    if (result != CF_SUCCESS) {
        return result;
    }
    
    // 2. 构建点到线的拓扑
    result = build_point_to_line_topology(model);
    if (result != CF_SUCCESS) {
        return result;
    }
    
    // 3. 更新边界盒
    cf_model_update_bounds(model);
    
    return CF_SUCCESS;
}

/**
 * @brief 查找相邻线段
 */
cf_result_t cf_topology_find_neighbors(
    const cf_model_t* model,
    cf_index_t line_index,
    cf_index_t** neighbors,
    size_t* count
) {
    if (!model || !model->lines || !neighbors || !count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (line_index >= model->lines->count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_line_t target = model->lines->lines[line_index];
    
    // 查找共享端点的线段
    size_t capacity = 8;
    cf_index_t* result = (cf_index_t*)malloc(sizeof(cf_index_t) * capacity);
    if (!result) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    size_t found = 0;
    
    for (size_t i = 0; i < model->lines->count; i++) {
        if (i == line_index) continue;
        
        cf_line_t line = model->lines->lines[i];
        
        // 检查是否共享端点
        bool shares_point = (line.p1 == target.p1 || line.p1 == target.p2 ||
                            line.p2 == target.p1 || line.p2 == target.p2);
        
        if (shares_point) {
            // 扩容
            if (found >= capacity) {
                capacity *= 2;
                cf_index_t* new_result = (cf_index_t*)realloc(result, sizeof(cf_index_t) * capacity);
                if (!new_result) {
                    free(result);
                    return CF_ERROR_OUT_OF_MEMORY;
                }
                result = new_result;
            }
            
            result[found++] = (cf_index_t)i;
        }
    }
    
    *neighbors = result;
    *count = found;
    
    return CF_SUCCESS;
}
