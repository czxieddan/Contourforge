/**
 * @file contourforge.h
 * @brief Contourforge主头文件 - 包含所有模块
 * @version 1.0.0
 * 
 * 这是Contourforge库的主入口头文件，包含所有子模块的API。
 * 
 * @example
 * ```c
 * #include <contourforge/contourforge.h>
 * 
 * int main() {
 *     // 加载高度图
 *     cf_heightmap_t* heightmap;
 *     cf_heightmap_load("terrain.png", &heightmap);
 *     
 *     // 生成等高线
 *     cf_contour_config_t config = {
 *         .interval = 10.0f,
 *         .min_height = 0.0f,
 *         .max_height = 1000.0f,
 *         .simplify_tolerance = 1.0f,
 *         .build_topology = true
 *     };
 *     cf_model_t* model;
 *     cf_contour_generate(heightmap, &config, &model);
 *     
 *     // 初始化渲染器
 *     cf_renderer_config_t renderer_config = {
 *         .width = 1280,
 *         .height = 720,
 *         .title = "Contourforge Viewer",
 *         .vsync = true,
 *         .msaa_samples = 4
 *     };
 *     cf_renderer_t* renderer;
 *     cf_renderer_init(&renderer_config, &renderer);
 *     cf_renderer_set_model(renderer, model);
 *     
 *     // 渲染循环
 *     while (!cf_renderer_should_close(renderer)) {
 *         cf_renderer_begin_frame(renderer);
 *         cf_renderer_render(renderer);
 *         cf_renderer_end_frame(renderer);
 *     }
 *     
 *     // 清理
 *     cf_renderer_destroy(renderer);
 *     cf_model_destroy(model);
 *     cf_heightmap_destroy(heightmap);
 *     
 *     return 0;
 * }
 * ```
 */

#ifndef CONTOURFORGE_H
#define CONTOURFORGE_H

/* 版本信息 */
#define CONTOURFORGE_VERSION_MAJOR 1
#define CONTOURFORGE_VERSION_MINOR 0
#define CONTOURFORGE_VERSION_PATCH 0
#define CONTOURFORGE_VERSION_STRING "1.0.0"

/* 包含所有子模块 */
#include "types.h"
#include "core.h"
#include "rendering.h"
#include "datagen.h"
#include "control.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取版本字符串
 * @return 版本字符串（例如："1.0.0"）
 */
const char* cf_get_version(void);

/**
 * @brief 获取版本号
 * @param major 主版本号
 * @param minor 次版本号
 * @param patch 补丁版本号
 */
void cf_get_version_number(int* major, int* minor, int* patch);

#ifdef __cplusplus
}
#endif

#endif /* CONTOURFORGE_H */
