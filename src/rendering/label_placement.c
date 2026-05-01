/**
 * @file label_placement.c
 * @brief 标注放置算法实现
 */

#include "contourforge/rendering.h"
#include "contourforge/core.h"
#include "contourforge/types.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ========== 辅助函数 ========== */

/**
 * @brief 计算两点之间的距离
 */
static float distance_3d(cf_point3_t a, cf_point3_t b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

/**
 * @brief 检查两个标注是否重叠
 */
static bool labels_overlap(cf_point3_t pos1, cf_point3_t pos2, float min_distance) {
    return distance_3d(pos1, pos2) < min_distance;
}

/* ========== 标注放置API ========== */

/**
 * @brief 沿等高线放置标注
 * @param lines 等高线线段集合
 * @param height 等高线高度
 * @param spacing 标注间距
 * @param min_distance 最小标注距离（避免重叠）
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
) {
    if (lines == NULL || positions == NULL || out_count == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (lines->count == 0) {
        *out_count = 0;
        return CF_SUCCESS;
    }
    
    size_t label_count = 0;
    float accumulated_length = 0.0f;
    
    /* 遍历所有线段 */
    for (size_t i = 0; i < lines->count && label_count < max_positions; i++) {
        const cf_line_t* line = &lines->lines[i];
        
        /* 遍历线段中的点 */
        for (size_t j = 0; j < line->count - 1 && label_count < max_positions; j++) {
            cf_point3_t p1 = line->points[j];
            cf_point3_t p2 = line->points[j + 1];
            
            /* 计算线段长度 */
            float segment_length = distance_3d(p1, p2);
            
            /* 如果累积长度超过间距，放置标注 */
            while (accumulated_length >= spacing && label_count < max_positions) {
                /* 计算标注在线段上的位置 */
                float t = (spacing - (accumulated_length - segment_length)) / segment_length;
                
                cf_point3_t label_pos;
                label_pos.x = p1.x + t * (p2.x - p1.x);
                label_pos.y = p1.y + t * (p2.y - p1.y);
                label_pos.z = p1.z + t * (p2.z - p1.z);
                
                /* 检查是否与已有标注重叠 */
                bool overlaps = false;
                for (size_t k = 0; k < label_count; k++) {
                    if (labels_overlap(label_pos, positions[k], min_distance)) {
                        overlaps = true;
                        break;
                    }
                }
                
                /* 如果不重叠，添加标注 */
                if (!overlaps) {
                    positions[label_count] = label_pos;
                    label_count++;
                }
                
                accumulated_length -= spacing;
            }
            
            accumulated_length += segment_length;
        }
    }
    
    *out_count = label_count;
    return CF_SUCCESS;
}

/**
 * @brief 根据相机距离计算标注间距（LOD）
 * @param camera_distance 相机到场景中心的距离
 * @param base_spacing 基础间距
 * @param lod_levels LOD层级数
 * @return 调整后的间距
 */
float cf_calculate_label_spacing_lod(
    float camera_distance,
    float base_spacing,
    int lod_levels
) {
    if (lod_levels <= 1) {
        return base_spacing;
    }
    
    /* 根据距离计算LOD层级 */
    if (camera_distance < 100.0f) {
        return base_spacing;
    } else if (camera_distance < 300.0f) {
        return base_spacing * 2.0f;
    } else if (camera_distance < 600.0f) {
        return base_spacing * 4.0f;
    } else if (camera_distance < 1000.0f) {
        return base_spacing * 8.0f;
    } else {
        return base_spacing * 16.0f;
    }
}

/**
 * @brief 过滤标注（根据相机距离和视锥体）
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
) {
    if (positions == NULL || visible == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    for (size_t i = 0; i < count; i++) {
        float dist = distance_3d(positions[i], camera_pos);
        visible[i] = (dist >= min_distance && dist <= max_distance);
    }
    
    return CF_SUCCESS;
}
