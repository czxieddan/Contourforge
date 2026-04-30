/**
 * @file rendering.h
 * @brief Contourforge渲染模块API
 * @version 1.0.0
 */

#ifndef CF_RENDERING_H
#define CF_RENDERING_H

#include "types.h"
#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 前向声明 ========== */

/**
 * @brief 渲染器句柄
 */
typedef struct cf_renderer cf_renderer_t;

/**
 * @brief 相机句柄
 */
typedef struct cf_camera cf_camera_t;

/**
 * @brief 着色器句柄
 */
typedef struct cf_shader cf_shader_t;

/* ========== 渲染器 ========== */

/**
 * @brief 渲染器配置
 */
typedef struct {
    int width;                  /**< 窗口宽度 */
    int height;                 /**< 窗口高度 */
    const char* title;          /**< 窗口标题 */
    bool vsync;                 /**< 垂直同步 */
    int msaa_samples;           /**< MSAA采样数（0=禁用） */
    cf_color_t clear_color;     /**< 清屏颜色 */
} cf_renderer_config_t;

/**
 * @brief 初始化渲染器
 * @param config 配置
 * @param renderer 输出渲染器指针
 * @return 返回码
 */
cf_result_t cf_renderer_init(
    const cf_renderer_config_t* config,
    cf_renderer_t** renderer
);

/**
 * @brief 设置着色器
 * @param renderer 渲染器
 * @param shader 着色器
 * @return 返回码
 */
cf_result_t cf_renderer_set_shader(
    cf_renderer_t* renderer,
    cf_shader_t* shader
);

/**
 * @brief 设置模型
 * @param renderer 渲染器
 * @param model 模型
 * @return 返回码
 */
cf_result_t cf_renderer_set_model(
    cf_renderer_t* renderer,
    cf_model_t* model
);

/**
 * @brief 设置LOD模型
 * @param renderer 渲染器
 * @param lod_model LOD模型
 * @return 返回码
 */
cf_result_t cf_renderer_set_lod_model(
    cf_renderer_t* renderer,
    cf_lod_model_t* lod_model
);

/**
 * @brief 启用/禁用自动LOD选择
 * @param renderer 渲染器
 * @param enable 是否启用
 */
void cf_renderer_set_auto_lod(
    cf_renderer_t* renderer,
    bool enable
);

/**
 * @brief 设置LOD调试模式
 * @param renderer 渲染器
 * @param enable 是否启用（显示当前LOD层级）
 */
void cf_renderer_set_lod_debug(
    cf_renderer_t* renderer,
    bool enable
);

/**
 * @brief 开始帧
 * @param renderer 渲染器
 * @return 返回码
 */
cf_result_t cf_renderer_begin_frame(cf_renderer_t* renderer);

/**
 * @brief 渲染
 * @param renderer 渲染器
 * @return 返回码
 */
cf_result_t cf_renderer_render(cf_renderer_t* renderer);

/**
 * @brief 结束帧
 * @param renderer 渲染器
 * @return 返回码
 */
cf_result_t cf_renderer_end_frame(cf_renderer_t* renderer);

/**
 * @brief 检查窗口是否应该关闭
 * @param renderer 渲染器
 * @return true=应该关闭
 */
bool cf_renderer_should_close(const cf_renderer_t* renderer);

/**
 * @brief 获取GLFW窗口句柄（内部使用）
 * @param renderer 渲染器
 * @return 窗口句柄
 */
void* cf_renderer_get_window(const cf_renderer_t* renderer);

/**
 * @brief 获取渲染器的相机
 * @param renderer 渲染器
 * @return 相机指针
 */
cf_camera_t* cf_renderer_get_camera(cf_renderer_t* renderer);

/**
 * @brief 获取窗口尺寸
 * @param renderer 渲染器
 * @param width 输出宽度
 * @param height 输出高度
 */
void cf_renderer_get_size(const cf_renderer_t* renderer, int* width, int* height);

/**
 * @brief 销毁渲染器
 * @param renderer 渲染器
 */
void cf_renderer_destroy(cf_renderer_t* renderer);

/* ========== 相机 ========== */

/**
 * @brief 相机类型
 */
typedef enum {
    CF_CAMERA_PERSPECTIVE,      /**< 透视相机 */
    CF_CAMERA_ORTHOGRAPHIC      /**< 正交相机 */
} cf_camera_type_t;

/**
 * @brief 创建相机
 * @param type 相机类型
 * @param camera 输出相机指针
 * @return 返回码
 */
cf_result_t cf_camera_create(
    cf_camera_type_t type,
    cf_camera_t** camera
);

/**
 * @brief 设置透视投影
 * @param camera 相机
 * @param fov 视野角度（度）
 * @param aspect 宽高比
 * @param near 近裁剪面
 * @param far 远裁剪面
 */
void cf_camera_set_perspective(
    cf_camera_t* camera,
    float fov,
    float aspect,
    float near,
    float far
);

/**
 * @brief 设置正交投影
 * @param camera 相机
 * @param left 左边界
 * @param right 右边界
 * @param bottom 下边界
 * @param top 上边界
 * @param near 近裁剪面
 * @param far 远裁剪面
 */
void cf_camera_set_orthographic(
    cf_camera_t* camera,
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far
);

/**
 * @brief 设置相机位置
 * @param camera 相机
 * @param position 位置
 */
void cf_camera_set_position(cf_camera_t* camera, cf_point3_t position);

/**
 * @brief 设置相机目标
 * @param camera 相机
 * @param target 目标点
 */
void cf_camera_set_target(cf_camera_t* camera, cf_point3_t target);

/**
 * @brief 获取相机位置
 * @param camera 相机
 * @return 相机位置
 */
cf_point3_t cf_camera_get_position(const cf_camera_t* camera);

/**
 * @brief 获取相机目标
 * @param camera 相机
 * @return 相机目标点
 */
cf_point3_t cf_camera_get_target(const cf_camera_t* camera);

/**
 * @brief 获取视图矩阵
 * @param camera 相机
 * @return 视图矩阵指针（4x4）
 */
const float* cf_camera_get_view_matrix(const cf_camera_t* camera);

/**
 * @brief 获取投影矩阵
 * @param camera 相机
 * @return 投影矩阵指针（4x4）
 */
const float* cf_camera_get_projection_matrix(const cf_camera_t* camera);

/**
 * @brief 相机环绕目标旋转
 * @param camera 相机
 * @param delta_yaw 偏航角变化（弧度）
 * @param delta_pitch 俯仰角变化（弧度）
 */
void cf_camera_orbit(cf_camera_t* camera, float delta_yaw, float delta_pitch);

/**
 * @brief 相机缩放（改变距离）
 * @param camera 相机
 * @param delta 缩放增量（负值拉近，正值推远）
 */
void cf_camera_zoom(cf_camera_t* camera, float delta);

/**
 * @brief 销毁相机
 * @param camera 相机
 */
void cf_camera_destroy(cf_camera_t* camera);

/* ========== 着色器 ========== */

/**
 * @brief 加载着色器
 * @param vertex_path 顶点着色器路径
 * @param fragment_path 片段着色器路径
 * @param shader 输出着色器指针
 * @return 返回码
 */
cf_result_t cf_shader_load(
    const char* vertex_path,
    const char* fragment_path,
    cf_shader_t** shader
);

/**
 * @brief 使用着色器
 * @param shader 着色器
 */
void cf_shader_use(cf_shader_t* shader);

/**
 * @brief 设置uniform（整数）
 * @param shader 着色器
 * @param name uniform名称
 * @param value 值
 */
void cf_shader_set_int(cf_shader_t* shader, const char* name, int value);

/**
 * @brief 设置uniform（浮点数）
 * @param shader 着色器
 * @param name uniform名称
 * @param value 值
 */
void cf_shader_set_float(cf_shader_t* shader, const char* name, float value);

/**
 * @brief 设置uniform（矩阵4x4）
 * @param shader 着色器
 * @param name uniform名称
 * @param matrix 矩阵指针
 */
void cf_shader_set_mat4(cf_shader_t* shader, const char* name, const float* matrix);

/**
 * @brief 销毁着色器
 * @param shader 着色器
 */
void cf_shader_destroy(cf_shader_t* shader);

#ifdef __cplusplus
}
#endif

#endif /* CF_RENDERING_H */
