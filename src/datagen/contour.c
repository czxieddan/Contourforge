/**
 * @file contour.c
 * @brief 等高线提取模块实现（Marching Squares算法）
 */

#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Marching Squares查找表
// 每个格子有4个角，每个角可以在等高线上方或下方
// 16种情况（2^4）
static const int MS_EDGE_TABLE[16] = {
    0x0, 0x9, 0x3, 0xA,
    0x6, 0xF, 0x5, 0xC,
    0xC, 0x5, 0xF, 0x6,
    0xA, 0x3, 0x9, 0x0
};

/**
 * @brief 线性插值计算等高线与边的交点
 */
static cf_point3_t interpolate_edge(
    cf_point3_t p1, float h1,
    cf_point3_t p2, float h2,
    float iso_value
) {
    cf_point3_t result;
    
    if (fabsf(h1 - h2) < 1e-6f) {
        result.x = (p1.x + p2.x) * 0.5f;
        result.y = (p1.y + p2.y) * 0.5f;
        result.z = (p1.z + p2.z) * 0.5f;
        return result;
    }
    
    float t = (iso_value - h1) / (h2 - h1);
    result.x = p1.x + t * (p2.x - p1.x);
    result.y = p1.y + t * (p2.y - p1.y);
    result.z = p1.z + t * (p2.z - p1.z);
    
    return result;
}

/**
 * @brief 从高度图提取单条等高线
 */
static cf_result_t extract_contour_level(
    const cf_heightmap_t* heightmap,
    float iso_value,
    cf_point_set_t* point_set,
    cf_line_set_t* line_set
) {
    int width = heightmap->width;
    int height = heightmap->height;
    
    // 遍历每个格子
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            // 获取格子四个角的高度
            float h00 = heightmap->data[y * width + x];
            float h10 = heightmap->data[y * width + (x + 1)];
            float h01 = heightmap->data[(y + 1) * width + x];
            float h11 = heightmap->data[(y + 1) * width + (x + 1)];
            
            // 计算格子配置（哪些角在等高线上方）
            int config = 0;
            if (h00 >= iso_value) config |= 1;
            if (h10 >= iso_value) config |= 2;
            if (h11 >= iso_value) config |= 4;
            if (h01 >= iso_value) config |= 8;
            
            // 跳过完全在上方或下方的格子
            if (config == 0 || config == 15) {
                continue;
            }
            
            // 格子四个角的3D坐标
            cf_point3_t p00 = {(float)x, (float)y, h00};
            cf_point3_t p10 = {(float)(x + 1), (float)y, h10};
            cf_point3_t p01 = {(float)x, (float)(y + 1), h01};
            cf_point3_t p11 = {(float)(x + 1), (float)(y + 1), h11};
            
            // 根据配置生成线段
            int edges = MS_EDGE_TABLE[config];
            
            if (edges == 0) continue;
            
            // 计算边上的交点
            cf_point3_t edge_points[4];
            int edge_count = 0;
            
            // 底边（p00-p10）
            if (edges & 0x1) {
                edge_points[edge_count++] = interpolate_edge(p00, h00, p10, h10, iso_value);
            }
            // 右边（p10-p11）
            if (edges & 0x2) {
                edge_points[edge_count++] = interpolate_edge(p10, h10, p11, h11, iso_value);
            }
            // 顶边（p11-p01）
            if (edges & 0x4) {
                edge_points[edge_count++] = interpolate_edge(p11, h11, p01, h01, iso_value);
            }
            // 左边（p01-p00）
            if (edges & 0x8) {
                edge_points[edge_count++] = interpolate_edge(p01, h01, p00, h00, iso_value);
            }
            
            // 生成线段（通常是2个点形成1条线段）
            if (edge_count >= 2) {
                cf_index_t idx1 = cf_point_set_add(point_set, edge_points[0]);
                cf_index_t idx2 = cf_point_set_add(point_set, edge_points[1]);
                cf_line_set_add(line_set, idx1, idx2);
            }
            
            // 处理鞍点情况（4个点形成2条线段）
            if (edge_count == 4) {
                cf_index_t idx3 = cf_point_set_add(point_set, edge_points[2]);
                cf_index_t idx4 = cf_point_set_add(point_set, edge_points[3]);
                cf_line_set_add(line_set, idx3, idx4);
            }
        }
    }
    
    return CF_SUCCESS;
}

/**
 * @brief 从高度图生成等高线模型
 */
cf_result_t cf_contour_generate(
    const cf_heightmap_t* heightmap,
    const cf_contour_config_t* config,
    cf_model_t** model
) {
    if (!heightmap || !config || !model) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 创建模型
    cf_result_t result = cf_model_create("contour_model", model);
    if (result != CF_SUCCESS) {
        return result;
    }
    
    cf_model_t* m = *model;
    
    // 确定等高线高度范围
    float min_h = config->min_height;
    float max_h = config->max_height;
    
    if (min_h >= max_h) {
        min_h = heightmap->min_height;
        max_h = heightmap->max_height;
    }
    
    // 生成每条等高线
    float interval = config->interval;
    if (interval <= 0.0f) {
        interval = (max_h - min_h) / 10.0f; // 默认10条等高线
    }
    
    int contour_count = 0;
    for (float h = min_h; h <= max_h; h += interval) {
        result = extract_contour_level(heightmap, h, m->points, m->lines);
        if (result != CF_SUCCESS) {
            cf_model_destroy(m);
            *model = NULL;
            return result;
        }
        contour_count++;
    }
    
    // 更新边界盒
    cf_model_update_bounds(m);
    
    // 如果需要简化
    if (config->simplify_tolerance > 0.0f) {
        // 简化将在后续步骤中实现
        // 这里先预留接口
    }
    
    // 如果需要构建拓扑
    if (config->build_topology) {
        result = cf_topology_build(m);
        if (result != CF_SUCCESS) {
            cf_model_destroy(m);
            *model = NULL;
            return result;
        }
    }
    
    return CF_SUCCESS;
}
