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

/**
 * @brief 字体句柄
 */
typedef struct cf_font cf_font_t;

/**
 * @brief 文字渲染器句柄
 */
typedef struct cf_text_renderer cf_text_renderer_t;

/**
 * @brief 标注句柄
 */
typedef struct cf_label cf_label_t;

/**
 * @brief 标注管理器句柄
 */
typedef struct cf_label_manager cf_label_manager_t;

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

/* ========== 文字渲染 ========== */

/**
 * @brief 加载字体
 * @param font_path 字体文件路径（TTF格式）
 * @param size 字体大小（像素）
 * @param font 输出字体指针
 * @return 返回码
 */
cf_result_t cf_font_load(const char* font_path, float size, cf_font_t** font);

/**
 * @brief 销毁字体
 * @param font 字体
 */
void cf_font_destroy(cf_font_t* font);

/**
 * @brief 创建文字渲染器
 * @param font 字体
 * @param shader 文字着色器
 * @param renderer 输出渲染器指针
 * @return 返回码
 */
cf_result_t cf_text_renderer_create(
    cf_font_t* font,
    cf_shader_t* shader,
    cf_text_renderer_t** renderer
);

/**
 * @brief 渲染3D文字（Billboard效果）
 * @param renderer 文字渲染器
 * @param text 文字内容
 * @param position 3D位置
 * @param color 颜色
 * @param view_matrix 视图矩阵
 * @param projection_matrix 投影矩阵
 * @return 返回码
 */
cf_result_t cf_text_renderer_render_3d(
    cf_text_renderer_t* renderer,
    const char* text,
    cf_point3_t position,
    cf_color_t color,
    const float* view_matrix,
    const float* projection_matrix
);

/**
 * @brief 计算文字宽度
 * @param renderer 文字渲染器
 * @param text 文字内容
 * @return 文字宽度（像素）
 */
float cf_text_renderer_measure_width(cf_text_renderer_t* renderer, const char* text);

/**
 * @brief 销毁文字渲染器
 * @param renderer 文字渲染器
 */
void cf_text_renderer_destroy(cf_text_renderer_t* renderer);

/* ========== 标注系统 ========== */

/**
 * @brief 标注配置
 */
typedef struct {
    float spacing;          /**< 标注间距（米） */
    float min_distance;     /**< 最小显示距离 */
    float max_distance;     /**< 最大显示距离 */
    int lod_levels;         /**< LOD层级数 */
    char unit[16];          /**< 单位（"m", "ft"等） */
    int decimal_places;     /**< 小数位数 */
    cf_color_t color;       /**< 标注颜色 */
    float size;             /**< 字体大小 */
    bool show_index;        /**< 是否显示等高线索引 */
} cf_label_config_t;

/**
 * @brief 创建标注管理器
 * @param text_renderer 文字渲染器
 * @param config 标注配置
 * @param manager 输出管理器指针
 * @return 返回码
 */
cf_result_t cf_label_manager_create(
    cf_text_renderer_t* text_renderer,
    const cf_label_config_t* config,
    cf_label_manager_t** manager
);

/**
 * @brief 为等高线生成标注
 * @param manager 标注管理器
 * @param model 等高线模型
 * @param camera 相机（用于LOD计算）
 * @return 返回码
 */
cf_result_t cf_label_manager_generate_labels(
    cf_label_manager_t* manager,
    cf_model_t* model,
    cf_camera_t* camera
);

/**
 * @brief 更新标注（相机变化时调用）
 * @param manager 标注管理器
 * @param camera 相机
 * @return 返回码
 */
cf_result_t cf_label_manager_update(
    cf_label_manager_t* manager,
    cf_camera_t* camera
);

/**
 * @brief 渲染标注
 * @param manager 标注管理器
 * @param view_matrix 视图矩阵
 * @param projection_matrix 投影矩阵
 * @return 返回码
 */
cf_result_t cf_label_manager_render(
    cf_label_manager_t* manager,
    const float* view_matrix,
    const float* projection_matrix
);

/**
 * @brief 清除所有标注
 * @param manager 标注管理器
 */
void cf_label_manager_clear(cf_label_manager_t* manager);

/**
 * @brief 销毁标注管理器
 * @param manager 标注管理器
 */
void cf_label_manager_destroy(cf_label_manager_t* manager);

/**
 * @brief 沿等高线放置标注位置
 * @param lines 等高线线段集合
 * @param height 等高线高度
 * @param spacing 标注间距
 * @param min_distance 最小标注距离（用于避免重叠）
 * @param positions 输出标注位置数组
 * @param max_positions 最大标注数量
 * @param out_count 输出实际标注数量
 * @return 返回码
 */
cf_result_t cf_place_labels_on_contour(
    const cf_line_set_t* lines,
    float height,
    float spacing,
    float min_distance,
    cf_point3_t* positions,
    size_t max_positions,
    size_t* out_count
);

/**
 * @brief 根据相机距离计算标注间距（LOD）
 * @param camera_distance 相机到场景中心的距离
 * @param base_spacing 基础标注间距
 * @param lod_levels LOD层级数
 * @return 调整后的标注间距
 */
float cf_calculate_label_spacing_lod(
    float camera_distance,
    float base_spacing,
    int lod_levels
);

/**
 * @brief 根据距离过滤标注可见性
 * @param positions 标注位置数组
 * @param count 标注数量
 * @param camera_pos 相机位置
 * @param min_distance 最小显示距离
 * @param max_distance 最大显示距离
 * @param visible 输出可见性数组
 * @return 返回码
 */
cf_result_t cf_filter_labels_by_distance(
    const cf_point3_t* positions,
    size_t count,
    cf_point3_t camera_pos,
    float min_distance,
    float max_distance,
    bool* visible
);

#ifdef __cplusplus
}
#endif

#endif /* CF_RENDERING_H */
