/**
 * @file core.h
 * @brief Contourforge核心模块API
 * @version 1.0.0
 */

#ifndef CF_CORE_H
#define CF_CORE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 前向声明 ========== */

/**
 * @brief 线程池句柄（前向声明）
 */
typedef struct cf_thread_pool cf_thread_pool_t;

/* ========== 内存管理 ========== */

/**
 * @brief 内存池句柄
 */
typedef struct cf_memory_pool cf_memory_pool_t;

/**
 * @brief 创建内存池
 * @param block_size 块大小（字节）
 * @param block_count 块数量
 * @param pool 输出内存池指针
 * @return 返回码
 */
cf_result_t cf_memory_pool_create(
    size_t block_size,
    size_t block_count,
    cf_memory_pool_t** pool
);

/**
 * @brief 从内存池分配内存
 * @param pool 内存池
 * @return 分配的内存指针，失败返回NULL
 */
void* cf_memory_pool_alloc(cf_memory_pool_t* pool);

/**
 * @brief 释放内存到内存池
 * @param pool 内存池
 * @param ptr 内存指针
 */
void cf_memory_pool_free(cf_memory_pool_t* pool, void* ptr);

/**
 * @brief 销毁内存池
 * @param pool 内存池
 */
void cf_memory_pool_destroy(cf_memory_pool_t* pool);

/* ========== 点集 ========== */

/**
 * @brief 点集
 */
typedef struct {
    cf_point3_t* points;    /**< 点数组 */
    size_t count;           /**< 点数量 */
    size_t capacity;        /**< 容量 */
    cf_bounds_t bounds;     /**< 边界盒 */
    bool dirty;             /**< 是否需要更新GPU缓冲 */
} cf_point_set_t;

/**
 * @brief 创建点集
 * @param initial_capacity 初始容量
 * @param point_set 输出点集指针
 * @return 返回码
 */
cf_result_t cf_point_set_create(
    size_t initial_capacity,
    cf_point_set_t** point_set
);

/**
 * @brief 添加点
 * @param point_set 点集
 * @param point 点
 * @return 点索引
 */
cf_index_t cf_point_set_add(cf_point_set_t* point_set, cf_point3_t point);

/**
 * @brief 获取点
 * @param point_set 点集
 * @param index 索引
 * @return 点指针，失败返回NULL
 */
const cf_point3_t* cf_point_set_get(const cf_point_set_t* point_set, cf_index_t index);

/**
 * @brief 更新边界盒
 * @param point_set 点集
 */
void cf_point_set_update_bounds(cf_point_set_t* point_set);

/**
 * @brief 销毁点集
 * @param point_set 点集
 */
void cf_point_set_destroy(cf_point_set_t* point_set);

/* ========== 线集 ========== */

/**
 * @brief 线段
 */
typedef struct {
    cf_index_t p1;  /**< 起点索引 */
    cf_index_t p2;  /**< 终点索引 */
} cf_line_t;

/**
 * @brief 线集
 */
typedef struct {
    cf_line_t* lines;               /**< 线段数组 */
    size_t count;                   /**< 线段数量 */
    size_t capacity;                /**< 容量 */
    cf_point_set_t* point_set;      /**< 关联的点集 */
} cf_line_set_t;

/**
 * @brief 创建线集
 * @param point_set 关联的点集
 * @param initial_capacity 初始容量
 * @param line_set 输出线集指针
 * @return 返回码
 */
cf_result_t cf_line_set_create(
    cf_point_set_t* point_set,
    size_t initial_capacity,
    cf_line_set_t** line_set
);

/**
 * @brief 添加线段
 * @param line_set 线集
 * @param p1 起点索引
 * @param p2 终点索引
 * @return 线段索引
 */
cf_index_t cf_line_set_add(cf_line_set_t* line_set, cf_index_t p1, cf_index_t p2);

/**
 * @brief 销毁线集
 * @param line_set 线集
 */
void cf_line_set_destroy(cf_line_set_t* line_set);

/* ========== 模型 ========== */

/**
 * @brief 模型
 */
typedef struct {
    cf_point_set_t* points;     /**< 点集 */
    cf_line_set_t* lines;       /**< 线集 */
    cf_bounds_t bounds;         /**< 整体边界盒 */
    char* name;                 /**< 模型名称 */
} cf_model_t;

/**
 * @brief 创建模型
 * @param name 模型名称
 * @param model 输出模型指针
 * @return 返回码
 */
cf_result_t cf_model_create(const char* name, cf_model_t** model);

/**
 * @brief 更新模型边界盒
 * @param model 模型
 */
void cf_model_update_bounds(cf_model_t* model);

/**
 * @brief 获取模型中心点
 * @param model 模型
 * @return 中心点
 */
cf_point3_t cf_model_get_center(const cf_model_t* model);

/**
 * @brief 获取模型尺寸
 * @param model 模型
 * @return 尺寸向量
 */
cf_vec3_t cf_model_get_size(const cf_model_t* model);

/**
 * @brief 销毁模型
 * @param model 模型
 */
void cf_model_destroy(cf_model_t* model);

/* ========== 八叉树 ========== */

/**
 * @brief 八叉树节点
 */
typedef struct cf_octree_node cf_octree_node_t;

/**
 * @brief 八叉树
 */
typedef struct {
    cf_octree_node_t* root;     /**< 根节点 */
    cf_bounds_t bounds;         /**< 整体边界 */
    size_t max_depth;           /**< 最大深度 */
    size_t max_points;          /**< 每节点最大点数 */
} cf_octree_t;

/**
 * @brief 创建八叉树
 * @param bounds 边界
 * @param max_depth 最大深度
 * @param max_points 每节点最大点数
 * @param octree 输出八叉树指针
 * @return 返回码
 */
cf_result_t cf_octree_create(
    cf_bounds_t bounds,
    size_t max_depth,
    size_t max_points,
    cf_octree_t** octree
);

/**
 * @brief 插入点
 * @param octree 八叉树
 * @param point 点
 * @param index 点索引
 * @return 返回码
 */
cf_result_t cf_octree_insert(
    cf_octree_t* octree,
    cf_point3_t point,
    cf_index_t index
);

/**
 * @brief 查询边界内的点
 * @param octree 八叉树
 * @param bounds 查询边界
 * @param indices 输出索引数组
 * @param count 输出索引数量
 * @return 返回码
 */
cf_result_t cf_octree_query(
    const cf_octree_t* octree,
    cf_bounds_t bounds,
    cf_index_t** indices,
    size_t* count
);

/**
 * @brief 销毁八叉树
 * @param octree 八叉树
 */
void cf_octree_destroy(cf_octree_t* octree);

/* ========== LOD系统 ========== */

/**
 * @brief LOD层级
 */
typedef struct {
    cf_index_t* point_indices;      /**< 点索引数组 */
    size_t point_count;             /**< 点数量 */
    cf_index_t* line_indices;       /**< 线索引数组（成对） */
    size_t line_count;              /**< 线段数量 */
    float distance_threshold;       /**< 距离阈值 */
    float simplification_ratio;     /**< 简化率 */
} cf_lod_level_t;

/**
 * @brief LOD模型
 */
typedef struct {
    cf_model_t* base_model;         /**< 基础模型 */
    cf_lod_level_t* levels;         /**< LOD层级数组 */
    size_t level_count;             /**< 层级数量 */
    int current_level;              /**< 当前使用的层级 */
} cf_lod_model_t;

/**
 * @brief LOD配置
 */
typedef struct {
    size_t level_count;             /**< LOD层级数量 */
    float* distance_thresholds;     /**< 距离阈值数组 */
    float* simplification_ratios;   /**< 简化率数组 */
    bool preserve_boundaries;       /**< 是否保护边界 */
    bool use_importance_sampling;   /**< 是否使用重要性采样 */
} cf_lod_config_t;

/**
 * @brief LOD统计信息
 */
typedef struct {
    size_t original_point_count;    /**< 原始点数量 */
    size_t original_line_count;     /**< 原始线段数量 */
    size_t* level_point_counts;     /**< 各层级点数量 */
    size_t* level_line_counts;      /**< 各层级线段数量 */
    float* reduction_ratios;        /**< 各层级简化率 */
    size_t total_memory_bytes;      /**< 总内存占用 */
} cf_lod_stats_t;

/**
 * @brief 创建LOD模型
 * @param base_model 基础模型
 * @param config LOD配置
 * @param lod_model 输出LOD模型指针
 * @return 返回码
 */
cf_result_t cf_lod_create(
    cf_model_t* base_model,
    const cf_lod_config_t* config,
    cf_lod_model_t** lod_model
);

/**
 * @brief 并行创建LOD模型
 * @param base_model 基础模型
 * @param config LOD配置
 * @param thread_pool 线程池（NULL则使用单线程）
 * @param lod_model 输出LOD模型指针
 * @return 返回码
 */
cf_result_t cf_lod_create_parallel(
    cf_model_t* base_model,
    const cf_lod_config_t* config,
    cf_thread_pool_t* thread_pool,
    cf_lod_model_t** lod_model
);

/**
 * @brief 根据距离选择LOD层级
 * @param lod_model LOD模型
 * @param distance 相机到模型的距离
 * @return LOD层级索引
 */
int cf_lod_select_level(
    const cf_lod_model_t* lod_model,
    float distance
);

/**
 * @brief 设置当前LOD层级
 * @param lod_model LOD模型
 * @param level 层级索引
 * @return 返回码
 */
cf_result_t cf_lod_set_level(
    cf_lod_model_t* lod_model,
    int level
);

/**
 * @brief 获取当前LOD层级
 * @param lod_model LOD模型
 * @return 当前层级索引
 */
int cf_lod_get_current_level(const cf_lod_model_t* lod_model);

/**
 * @brief 获取LOD统计信息
 * @param lod_model LOD模型
 * @param stats 输出统计信息
 * @return 返回码
 */
cf_result_t cf_lod_get_stats(
    const cf_lod_model_t* lod_model,
    cf_lod_stats_t* stats
);

/**
 * @brief 销毁LOD统计信息
 * @param stats 统计信息
 */
void cf_lod_stats_destroy(cf_lod_stats_t* stats);

/**
 * @brief 销毁LOD模型
 * @param lod_model LOD模型
 */
void cf_lod_destroy(cf_lod_model_t* lod_model);

#ifdef __cplusplus
}
#endif

#endif /* CF_CORE_H */
