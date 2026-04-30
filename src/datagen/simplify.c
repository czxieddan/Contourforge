/**
 * @file simplify.c
 * @brief 线段简化模块实现
 */

#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief 计算点到线段的垂直距离
 */
static float point_to_line_distance(
    cf_point3_t point,
    cf_point3_t line_start,
    cf_point3_t line_end
) {
    // 向量AB和AP
    float dx = line_end.x - line_start.x;
    float dy = line_end.y - line_start.y;
    float dz = line_end.z - line_start.z;
    
    float px = point.x - line_start.x;
    float py = point.y - line_start.y;
    float pz = point.z - line_start.z;
    
    // 线段长度的平方
    float line_len_sq = dx * dx + dy * dy + dz * dz;
    
    if (line_len_sq < 1e-10f) {
        // 线段退化为点
        return sqrtf(px * px + py * py + pz * pz);
    }
    
    // 计算投影参数t
    float t = (px * dx + py * dy + pz * dz) / line_len_sq;
    
    // 限制t在[0,1]范围内
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    // 计算最近点
    float closest_x = line_start.x + t * dx;
    float closest_y = line_start.y + t * dy;
    float closest_z = line_start.z + t * dz;
    
    // 计算距离
    float dist_x = point.x - closest_x;
    float dist_y = point.y - closest_y;
    float dist_z = point.z - closest_z;
    
    return sqrtf(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
}

/**
 * @brief Douglas-Peucker递归简化
 */
static void douglas_peucker_recursive(
    const cf_point3_t* points,
    size_t start,
    size_t end,
    float tolerance,
    bool* keep
) {
    if (end <= start + 1) {
        return;
    }
    
    // 找到距离起点-终点连线最远的点
    float max_dist = 0.0f;
    size_t max_index = start;
    
    for (size_t i = start + 1; i < end; i++) {
        float dist = point_to_line_distance(
            points[i],
            points[start],
            points[end]
        );
        
        if (dist > max_dist) {
            max_dist = dist;
            max_index = i;
        }
    }
    
    // 如果最大距离超过容差，保留该点并递归处理两段
    if (max_dist > tolerance) {
        keep[max_index] = true;
        douglas_peucker_recursive(points, start, max_index, tolerance, keep);
        douglas_peucker_recursive(points, max_index, end, tolerance, keep);
    }
}

/**
 * @brief Douglas-Peucker线段简化
 */
cf_result_t cf_simplify_douglas_peucker(
    const cf_point3_t* points,
    size_t count,
    float tolerance,
    cf_point3_t** out_points,
    size_t* out_count
) {
    if (!points || count < 2 || !out_points || !out_count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (tolerance <= 0.0f) {
        // 不简化，直接复制
        cf_point3_t* result = (cf_point3_t*)malloc(sizeof(cf_point3_t) * count);
        if (!result) {
            return CF_ERROR_OUT_OF_MEMORY;
        }
        memcpy(result, points, sizeof(cf_point3_t) * count);
        *out_points = result;
        *out_count = count;
        return CF_SUCCESS;
    }
    
    // 标记要保留的点
    bool* keep = (bool*)calloc(count, sizeof(bool));
    if (!keep) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 始终保留起点和终点
    keep[0] = true;
    keep[count - 1] = true;
    
    // 递归简化
    douglas_peucker_recursive(points, 0, count - 1, tolerance, keep);
    
    // 统计保留的点数
    size_t kept_count = 0;
    for (size_t i = 0; i < count; i++) {
        if (keep[i]) {
            kept_count++;
        }
    }
    
    // 分配输出数组
    cf_point3_t* result = (cf_point3_t*)malloc(sizeof(cf_point3_t) * kept_count);
    if (!result) {
        free(keep);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 复制保留的点
    size_t out_idx = 0;
    for (size_t i = 0; i < count; i++) {
        if (keep[i]) {
            result[out_idx++] = points[i];
        }
    }
    
    free(keep);
    
    *out_points = result;
    *out_count = kept_count;
    
    return CF_SUCCESS;
}

/**
 * @brief 计算三点形成的三角形面积
 */
static float triangle_area(cf_point3_t p1, cf_point3_t p2, cf_point3_t p3) {
    // 使用海伦公式或叉积
    float ax = p2.x - p1.x;
    float ay = p2.y - p1.y;
    float az = p2.z - p1.z;
    
    float bx = p3.x - p1.x;
    float by = p3.y - p1.y;
    float bz = p3.z - p1.z;
    
    // 叉积
    float cx = ay * bz - az * by;
    float cy = az * bx - ax * bz;
    float cz = ax * by - ay * bx;
    
    return 0.5f * sqrtf(cx * cx + cy * cy + cz * cz);
}

/**
 * @brief Visvalingam-Whyatt线段简化
 */
cf_result_t cf_simplify_visvalingam(
    const cf_point3_t* points,
    size_t count,
    size_t target_count,
    cf_point3_t** out_points,
    size_t* out_count
) {
    if (!points || count < 3 || target_count < 2 || !out_points || !out_count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (target_count >= count) {
        // 不需要简化
        cf_point3_t* result = (cf_point3_t*)malloc(sizeof(cf_point3_t) * count);
        if (!result) {
            return CF_ERROR_OUT_OF_MEMORY;
        }
        memcpy(result, points, sizeof(cf_point3_t) * count);
        *out_points = result;
        *out_count = count;
        return CF_SUCCESS;
    }
    
    // 创建工作数组
    typedef struct {
        cf_point3_t point;
        float area;
        bool removed;
    } vw_point_t;
    
    vw_point_t* work = (vw_point_t*)malloc(sizeof(vw_point_t) * count);
    if (!work) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 初始化
    for (size_t i = 0; i < count; i++) {
        work[i].point = points[i];
        work[i].removed = false;
        
        if (i == 0 || i == count - 1) {
            work[i].area = INFINITY; // 保护起点和终点
        } else {
            work[i].area = triangle_area(points[i - 1], points[i], points[i + 1]);
        }
    }
    
    // 逐步移除面积最小的点
    size_t remaining = count;
    while (remaining > target_count) {
        // 找到面积最小的点
        float min_area = INFINITY;
        size_t min_idx = 0;
        
        for (size_t i = 1; i < count - 1; i++) {
            if (!work[i].removed && work[i].area < min_area) {
                min_area = work[i].area;
                min_idx = i;
            }
        }
        
        // 移除该点
        work[min_idx].removed = true;
        remaining--;
        
        // 更新相邻点的面积
        // 找到前一个未移除的点
        size_t prev = min_idx - 1;
        while (prev > 0 && work[prev].removed) {
            prev--;
        }
        
        // 找到后一个未移除的点
        size_t next = min_idx + 1;
        while (next < count - 1 && work[next].removed) {
            next++;
        }
        
        // 更新prev的面积
        if (prev > 0) {
            size_t prev_prev = prev - 1;
            while (prev_prev > 0 && work[prev_prev].removed) {
                prev_prev--;
            }
            work[prev].area = triangle_area(
                work[prev_prev].point,
                work[prev].point,
                work[next].point
            );
        }
        
        // 更新next的面积
        if (next < count - 1) {
            size_t next_next = next + 1;
            while (next_next < count - 1 && work[next_next].removed) {
                next_next++;
            }
            work[next].area = triangle_area(
                work[prev].point,
                work[next].point,
                work[next_next].point
            );
        }
    }
    
    // 收集结果
    cf_point3_t* result = (cf_point3_t*)malloc(sizeof(cf_point3_t) * target_count);
    if (!result) {
        free(work);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    size_t out_idx = 0;
    for (size_t i = 0; i < count && out_idx < target_count; i++) {
        if (!work[i].removed) {
            result[out_idx++] = work[i].point;
        }
    }
    
    free(work);
    
    *out_points = result;
    *out_count = out_idx;
    
    return CF_SUCCESS;
}
