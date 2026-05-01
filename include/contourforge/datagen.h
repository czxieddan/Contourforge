/**
 * @file datagen.h
 * @brief Contourforge数据生成模块API
 * @version 1.1.0
 */

#ifndef CF_DATAGEN_H
#define CF_DATAGEN_H

#include "types.h"
#include "core.h"
#include "threading.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 高度图格式 ========== */

/**
 * @brief 高度图格式枚举
 */
typedef enum {
    CF_FORMAT_UNKNOWN = 0,  /**< 未知格式 */
    CF_FORMAT_PNG,          /**< PNG格式 */
    CF_FORMAT_JPEG,         /**< JPEG格式 */
    CF_FORMAT_BMP,          /**< BMP格式 */
    CF_FORMAT_TIFF,         /**< TIFF格式 */
    CF_FORMAT_GEOTIFF,      /**< GeoTIFF格式 */
    CF_FORMAT_RAW           /**< RAW格式 */
} cf_heightmap_format_t;

/**
 * @brief RAW格式数据类型
 */
typedef enum {
    CF_RAW_FORMAT_U8 = 0,   /**< 8位无符号整数 */
    CF_RAW_FORMAT_U16,      /**< 16位无符号整数 */
    CF_RAW_FORMAT_I16,      /**< 16位有符号整数 */
    CF_RAW_FORMAT_U32,      /**< 32位无符号整数 */
    CF_RAW_FORMAT_I32,      /**< 32位有符号整数 */
    CF_RAW_FORMAT_F32       /**< 32位浮点数 */
} cf_raw_format_t;

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
 * @brief 高度图元数据
 */
typedef struct {
    double min_x, max_x;    /**< 经度范围 */
    double min_y, max_y;    /**< 纬度范围 */
    double pixel_scale_x;   /**< X方向分辨率 */
    double pixel_scale_y;   /**< Y方向分辨率 */
    char projection[256];   /**< 投影信息 */
    char datum[64];         /**< 基准面 */
} cf_geo_metadata_t;

/**
 * @brief 检测高度图格式
 * @param filename 文件名
 * @return 格式类型
 */
cf_heightmap_format_t cf_heightmap_detect_format(const char* filename);

/**
 * @brief 获取格式名称
 * @param format 格式类型
 * @return 格式名称字符串
 */
const char* cf_heightmap_format_name(cf_heightmap_format_t format);

/**
 * @brief 加载高度图（自动检测格式）
 * @param filepath 文件路径
 * @param heightmap 输出高度图指针
 * @return 返回码
 */
cf_result_t cf_heightmap_load(
    const char* filepath,
    cf_heightmap_t** heightmap
);

/**
 * @brief 从TIFF文件加载高度图
 * @param filepath 文件路径
 * @param heightmap 输出高度图指针
 * @return 返回码
 */
cf_result_t cf_heightmap_load_tiff(
    const char* filepath,
    cf_heightmap_t** heightmap
);

/**
 * @brief 从RAW文件加载高度图
 * @param filepath 文件路径
 * @param width 图像宽度
 * @param height 图像高度
 * @param format RAW数据格式
 * @param heightmap 输出高度图指针
 * @return 返回码
 */
cf_result_t cf_heightmap_load_raw(
    const char* filepath,
    int width,
    int height,
    cf_raw_format_t format,
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

/* ========== 并行处理API ========== */

/**
 * @brief 并行提取等高线
 * @param heightmap 高度图
 * @param iso_value 等高线高度值
 * @param thread_pool 线程池（NULL则使用单线程）
 * @param point_set 输出点集
 * @param line_set 输出线集
 * @return 返回码
 */
cf_result_t cf_contour_extract_parallel(
    const cf_heightmap_t* heightmap,
    float iso_value,
    cf_thread_pool_t* thread_pool,
    cf_point_set_t* point_set,
    cf_line_set_t* line_set
);

/**
 * @brief 并行Douglas-Peucker线段简化
 * @param points 输入点数组
 * @param count 点数量
 * @param tolerance 容差
 * @param thread_pool 线程池（NULL则使用单线程）
 * @param out_points 输出点数组
 * @param out_count 输出点数量
 * @return 返回码
 */
cf_result_t cf_simplify_douglas_peucker_parallel(
    const cf_point3_t* points,
    size_t count,
    float tolerance,
    cf_thread_pool_t* thread_pool,
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
