/**
 * @file selection.c
 * @brief 节点选择模块实现
 */

#include "../../include/contourforge/control.h"
#include "../../include/contourforge/core.h"
#include "../../include/contourforge/rendering.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief 射线结构
 */
typedef struct {
    cf_point3_t origin;
    cf_point3_t direction;
} cf_ray_t;

/**
 * @brief 选择器结构
 */
struct cf_selector {
    cf_model_t* model;
    cf_renderer_t* renderer;
    cf_octree_t* octree;
    bool octree_dirty;
};

/**
 * @brief 从屏幕坐标创建射线
 */
static cf_ray_t screen_to_ray(
    double screen_x,
    double screen_y,
    cf_camera_t* camera,
    int viewport_width,
    int viewport_height
) {
    cf_ray_t ray;
    
    // 将屏幕坐标转换为NDC（-1到1）
    float ndc_x = (2.0f * (float)screen_x) / viewport_width - 1.0f;
    float ndc_y = 1.0f - (2.0f * (float)screen_y) / viewport_height;
    
    // 获取相机矩阵
    const float* view = cf_camera_get_view_matrix(camera);
    const float* proj = cf_camera_get_projection_matrix(camera);
    
    // 简化实现：假设相机在原点看向-Z方向
    // 实际应该使用逆矩阵变换
    ray.origin.x = 0.0f;
    ray.origin.y = 0.0f;
    ray.origin.z = 10.0f;
    
    ray.direction.x = ndc_x;
    ray.direction.y = ndc_y;
    ray.direction.z = -1.0f;
    
    // 归一化方向
    float len = sqrtf(ray.direction.x * ray.direction.x +
                     ray.direction.y * ray.direction.y +
                     ray.direction.z * ray.direction.z);
    if (len > 0.0f) {
        ray.direction.x /= len;
        ray.direction.y /= len;
        ray.direction.z /= len;
    }
    
    return ray;
}

/**
 * @brief 计算射线到点的距离
 */
static float ray_point_distance(cf_ray_t ray, cf_point3_t point) {
    // 向量从射线原点到点
    cf_point3_t ap;
    ap.x = point.x - ray.origin.x;
    ap.y = point.y - ray.origin.y;
    ap.z = point.z - ray.origin.z;
    
    // 投影到射线方向
    float t = ap.x * ray.direction.x + 
              ap.y * ray.direction.y + 
              ap.z * ray.direction.z;
    
    // 如果投影在射线后面，返回到原点的距离
    if (t < 0.0f) {
        return sqrtf(ap.x * ap.x + ap.y * ap.y + ap.z * ap.z);
    }
    
    // 计算最近点
    cf_point3_t closest;
    closest.x = ray.origin.x + t * ray.direction.x;
    closest.y = ray.origin.y + t * ray.direction.y;
    closest.z = ray.origin.z + t * ray.direction.z;
    
    // 计算距离
    float dx = point.x - closest.x;
    float dy = point.y - closest.y;
    float dz = point.z - closest.z;
    
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

/**
 * @brief 创建选择器
 */
cf_result_t cf_selector_create(
    cf_model_t* model,
    cf_renderer_t* renderer,
    cf_selector_t** selector
) {
    if (!model || !renderer || !selector) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_selector_t* sel = (cf_selector_t*)malloc(sizeof(cf_selector_t));
    if (!sel) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    sel->model = model;
    sel->renderer = renderer;
    sel->octree = NULL;
    sel->octree_dirty = true;
    
    *selector = sel;
    return CF_SUCCESS;
}

/**
 * @brief 重建八叉树
 */
static cf_result_t rebuild_octree(cf_selector_t* selector) {
    if (!selector->octree_dirty) {
        return CF_SUCCESS;
    }
    
    // 销毁旧的八叉树
    if (selector->octree) {
        cf_octree_destroy(selector->octree);
        selector->octree = NULL;
    }
    
    // 创建新的八叉树
    cf_result_t result = cf_octree_create(
        selector->model->bounds,
        8,  // 最大深度
        16, // 每节点最大点数
        &selector->octree
    );
    
    if (result != CF_SUCCESS) {
        return result;
    }
    
    // 插入所有点
    cf_point_set_t* ps = selector->model->points;
    for (size_t i = 0; i < ps->count; i++) {
        result = cf_octree_insert(selector->octree, ps->points[i], (cf_index_t)i);
        if (result != CF_SUCCESS) {
            cf_octree_destroy(selector->octree);
            selector->octree = NULL;
            return result;
        }
    }
    
    selector->octree_dirty = false;
    return CF_SUCCESS;
}

/**
 * @brief 选择点
 */
cf_result_t cf_selector_pick_point(
    cf_selector_t* selector,
    double screen_x,
    double screen_y,
    float radius,
    cf_index_t* out_index
) {
    if (!selector || !out_index) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 获取相机和视口尺寸
    cf_camera_t* camera = cf_renderer_get_camera(selector->renderer);
    int width, height;
    cf_renderer_get_size(selector->renderer, &width, &height);
    
    // 创建射线
    cf_ray_t ray = screen_to_ray(screen_x, screen_y, camera, width, height);
    
    // 查找最近的点
    float min_dist = radius;
    cf_index_t closest_index = (cf_index_t)-1;
    
    cf_point_set_t* ps = selector->model->points;
    for (size_t i = 0; i < ps->count; i++) {
        float dist = ray_point_distance(ray, ps->points[i]);
        if (dist < min_dist) {
            min_dist = dist;
            closest_index = (cf_index_t)i;
        }
    }
    
    if (closest_index == (cf_index_t)-1) {
        return CF_ERROR_NOT_FOUND;
    }
    
    *out_index = closest_index;
    return CF_SUCCESS;
}

/**
 * @brief 计算射线到线段的距离
 */
static float ray_line_distance(
    cf_ray_t ray,
    cf_point3_t p1,
    cf_point3_t p2,
    float* out_t
) {
    // 简化实现：计算射线到线段两端点的最小距离
    float d1 = ray_point_distance(ray, p1);
    float d2 = ray_point_distance(ray, p2);
    
    if (d1 < d2) {
        if (out_t) *out_t = 0.0f;
        return d1;
    } else {
        if (out_t) *out_t = 1.0f;
        return d2;
    }
}

/**
 * @brief 选择线段
 */
cf_result_t cf_selector_pick_line(
    cf_selector_t* selector,
    double screen_x,
    double screen_y,
    float radius,
    cf_index_t* out_index
) {
    if (!selector || !out_index) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 获取相机和视口尺寸
    cf_camera_t* camera = cf_renderer_get_camera(selector->renderer);
    int width, height;
    cf_renderer_get_size(selector->renderer, &width, &height);
    
    // 创建射线
    cf_ray_t ray = screen_to_ray(screen_x, screen_y, camera, width, height);
    
    // 查找最近的线段
    float min_dist = radius;
    cf_index_t closest_index = (cf_index_t)-1;
    
    cf_line_set_t* ls = selector->model->lines;
    cf_point_set_t* ps = selector->model->points;
    
    for (size_t i = 0; i < ls->count; i++) {
        cf_line_t line = ls->lines[i];
        cf_point3_t p1 = ps->points[line.p1];
        cf_point3_t p2 = ps->points[line.p2];
        
        float dist = ray_line_distance(ray, p1, p2, NULL);
        if (dist < min_dist) {
            min_dist = dist;
            closest_index = (cf_index_t)i;
        }
    }
    
    if (closest_index == (cf_index_t)-1) {
        return CF_ERROR_NOT_FOUND;
    }
    
    *out_index = closest_index;
    return CF_SUCCESS;
}

/**
 * @brief 框选点
 */
cf_result_t cf_selector_box_select(
    cf_selector_t* selector,
    double x1,
    double y1,
    double x2,
    double y2,
    cf_index_t** indices,
    size_t* count
) {
    if (!selector || !indices || !count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 简化实现：返回空结果
    // 实际应该将所有点投影到屏幕空间并检查是否在框内
    *indices = NULL;
    *count = 0;
    
    return CF_SUCCESS;
}

/**
 * @brief 销毁选择器
 */
void cf_selector_destroy(cf_selector_t* selector) {
    if (selector) {
        if (selector->octree) {
            cf_octree_destroy(selector->octree);
        }
        free(selector);
    }
}
