/**
 * @file datagen.h
 * @brief Contourforge数据生成模块API
 * @version 1.0.0
 */

#ifndef CF_DATAGEN_H
#define CF_DATAGEN_H

#include "types.h"
#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 高度图 ========== */

/**
 * @brief 高度图
 */
typedef struct {
    float* data;        /**< 高度数据 */
    int width;          /**< 宽度 */
    int height;         /**< 高度 */
    float min_height;   /**< 最小高度 */
    float max_height;   /**< 最大高度 */
} cf_heightmap_t;

/**
 * @brief 加载高度图（从灰度图）
 * @param filepath 文件路径
 * @param heightmap 输出高度图指针
 * @return 返回码
 */
cf_result_t cf_heightmap_load(
    const char* filepath,
    cf_heightmap_t** heightmap
);

/**
 * @brief 从高度图采样高度
 * @param heightmap 高度图
 * @param x X坐标（0-1）
 * @param y Y坐标（0-1）
 * @return 高度值
 */
float cf_heightmap_sample(
    const cf_heightmap_t* heightmap,
    float x,
    float y
);

/**
 * @brief 销毁高度图
 * @param heightmap 高度图
 */
void cf_heightmap_destroy(cf_heightmap_t* heightmap);

/* ========== 等高线生成 ========== */

/**
 * @brief 等高线配置
 */
typedef struct {
    float interval;             /**< 等高线间隔 */
    float min_height;           /**< 最小高度 */
    float max_height;           /**< 最大高度 */
    float simplify_tolerance;   /**< 简化容差（0=不简化） */
    bool build_topology;        /**< 是否构建拓扑 */
} cf_contour_config_t;

/**
 * @brief 从高度图生成等高线
 * @param heightmap 高度图
 * @param config 配置
 * @param model 输出模型指针
 * @return 返回码
 */
cf_result_t cf_contour_generate(
    const cf_heightmap_t* heightmap,
    const cf_contour_config_t* config,
    cf_model_t** model
);

/* ========== 线段简化 ========== */

/**
 * @brief Douglas-Peucker线段简化
 * @param points 输入点数组
 * @param count 点数量
 * @param tolerance 容差
 * @param out_points 输出点数组
 * @param out_count 输出点数量
 * @return 返回码
 */
cf_result_t cf_simplify_douglas_peucker(
    const cf_point3_t* points,
    size_t count,
    float tolerance,
    cf_point3_t** out_points,
    size_t* out_count
);

/**
 * @brief Visvalingam-Whyatt线段简化
 * @param points 输入点数组
 * @param count 点数量
 * @param target_count 目标点数量
 * @param out_points 输出点数组
 * @param out_count 输出点数量
 * @return 返回码
 */
cf_result_t cf_simplify_visvalingam(
    const cf_point3_t* points,
    size_t count,
    size_t target_count,
    cf_point3_t** out_points,
    size_t* out_count
);

/* ========== 拓扑构建 ========== */

/**
 * @brief 构建拓扑关系
 * @param model 模型
 * @return 返回码
 */
cf_result_t cf_topology_build(cf_model_t* model);

/**
 * @brief 查找相邻线段
 * @param model 模型
 * @param line_index 线段索引
 * @param neighbors 输出相邻线段索引数组
 * @param count 输出相邻线段数量
 * @return 返回码
 */
cf_result_t cf_topology_find_neighbors(
    const cf_model_t* model,
    cf_index_t line_index,
    cf_index_t** neighbors,
    size_t* count
);

#ifdef __cplusplus
}
#endif

#endif /* CF_DATAGEN_H */
