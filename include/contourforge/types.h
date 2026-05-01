/**
 * @file types.h
 * @brief Contourforge通用类型定义
 * @version 1.0.0
 */

#ifndef CF_TYPES_H
#define CF_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 基础类型 ========== */

/**
 * @brief 索引类型（支持大规模数据）
 */
typedef uint32_t cf_index_t;

/**
 * @brief 返回码
 */
typedef enum {
    CF_SUCCESS = 0,                     /**< 成功 */
    CF_ERROR_INVALID_PARAM = -1,        /**< 无效参数 */
    CF_ERROR_OUT_OF_MEMORY = -2,        /**< 内存不足 */
    CF_ERROR_FILE_NOT_FOUND = -3,       /**< 文件未找到 */
    CF_ERROR_FILE_IO = -4,              /**< 文件I/O错误 */
    CF_ERROR_INVALID_FORMAT = -5,       /**< 无效格式 */
    CF_ERROR_OPENGL = -6,               /**< OpenGL错误 */
    CF_ERROR_NOT_INITIALIZED = -7,      /**< 未初始化 */
    CF_ERROR_ALREADY_INITIALIZED = -8,  /**< 已初始化 */
    CF_ERROR_NOT_FOUND = -9,            /**< 未找到 */
    CF_ERROR_UNKNOWN = -100             /**< 未知错误 */
} cf_result_t;

/* ========== 数学类型 ========== */

/**
 * @brief 2D点
 */
typedef struct {
    float x, y;
} cf_point2_t;

/**
 * @brief 3D点
 */
typedef struct {
    float x, y, z;
} cf_point3_t;

/**
 * @brief 2D向量
 */
typedef struct {
    float x, y;
} cf_vec2_t;

/**
 * @brief 3D向量
 */
typedef struct {
    float x, y, z;
} cf_vec3_t;

/**
 * @brief 4D向量
 */
typedef struct {
    float x, y, z, w;
} cf_vec4_t;

/**
 * @brief 边界盒
 */
typedef struct {
    cf_point3_t min;  /**< 最小点 */
    cf_point3_t max;  /**< 最大点 */
} cf_bounds_t;

/**
 * @brief 颜色（RGBA）
 */
typedef struct {
    float r, g, b, a;
} cf_color_t;

/* ========== 工具宏 ========== */

/**
 * @brief 检查返回码是否成功
 */
#define CF_SUCCESS_CHECK(result) ((result) == CF_SUCCESS)

/**
 * @brief 检查返回码是否失败
 */
#define CF_FAILED(result) ((result) != CF_SUCCESS)

/**
 * @brief 最小值
 */
#ifndef CF_MIN
#define CF_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/**
 * @brief 限制值在范围内
 * @note 不依赖CF_MAX宏，避免与Windows SDK头文件中的CF_MAX定义冲突。
 */
#ifndef CF_CLAMP
#define CF_CLAMP(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))
#endif

#ifdef __cplusplus
}
#endif

#endif /* CF_TYPES_H */
