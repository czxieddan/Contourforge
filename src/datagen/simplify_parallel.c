/**
 * @file simplify_parallel.c
 * @brief 并行线段简化实现
 */

#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include "../../include/contourforge/threading.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ========== 辅助函数 ========== */

/**
 * @brief 计算点到线段的垂直距离
 */
static float point_to_line_distance(
    cf_point3_t point,
    cf_point3_t line_start,
    cf_point3_t line_end
) {
    float dx = line_end.x - line_start.x;
    float dy = line_end.y - line_start.y;
    float dz = line_end.z - line_start.z;
    
    float px = point.x - line_start.x;
    float py = point.y - line_start.y;
    float pz = point.z - line_start.z;
    
    float line_len_sq = dx * dx + dy * dy + dz * dz;
    
    if (line_len_sq < 1e-10f) {
        return sqrtf(px * px + py * py + pz * pz);
    }
    
    float t = (px * dx + py * dy + pz * dz) / line_len_sq;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    float closest_x = line_start.x + t * dx;
    float closest_y = line_start.y + t * dy;
    float closest_z = line_start.z + t * dz;
    
    float dist_x = point.x - closest_x;
    float dist_y = point.y - closest_y;
    float dist_z = point.z - closest_z;
    
    return sqrtf(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
}

/* ========== 并行简化任务 ========== */

typedef struct {
    const cf_point3_t* points;
    size_t start;
    size_t end;
    float tolerance;
    bool* keep;  // 共享的keep数组
    cf_mutex_t* mutex;  // 保护keep数组
} simplify_task_t;

/**
 * @brief Douglas-Peucker递归简化（线程安全版本）
 */
static void douglas_peucker_recursive_safe(
    const cf_point3_t* points,
    size_t start,
    size_t end,
    float tolerance,
    bool* keep,
    cf_mutex_t* mutex
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
    
    // 如果最大距离超过容差，保留该点并递归处理
    if (max_dist > tolerance) {
        cf_mutex_lock(mutex);
        keep[max_index] = true;
        cf_mutex_unlock(mutex);
        
        douglas_peucker_recursive_safe(points, start, max_index, tolerance, keep, mutex);
        douglas_peucker_recursive_safe(points, max_index, end, tolerance, keep, mutex);
    }
}

/**
 * @brief 简化工作函数
 */
static void simplify_worker(void* arg) {
    simplify_task_t* task = (simplify_task_t*)arg;
    
    douglas_peucker_recursive_safe(
        task->points,
        task->start,
        task->end,
        task->tolerance,
        task->keep,
        task->mutex
    );
}

/**
 * @brief 并行Douglas-Peucker线段简化
 */
cf_result_t cf_simplify_douglas_peucker_parallel(
    const cf_point3_t* points,
    size_t count,
    float tolerance,
    cf_thread_pool_t* thread_pool,
    cf_point3_t** out_points,
    size_t* out_count
) {
    if (!points || count < 2 || !out_points || !out_count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 如果没有线程池，回退到单线程
    if (!thread_pool) {
        return CF_ERROR_INVALID_PARAM;  // 需要实现单线程版本
    }
    
    // 创建keep数组
    bool* keep = (bool*)calloc(count, sizeof(bool));
    if (!keep) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 始终保留首尾点
    keep[0] = true;
    keep[count - 1] = true;
    
    // 创建互斥锁
    cf_mutex_t* mutex;
    cf_result_t result = cf_mutex_create(&mutex);
    if (result != CF_SUCCESS) {
        free(keep);
        return result;
    }
    
    // 确定分段数（基于线程数）
    int num_threads = 4;  // 默认值
    cf_thread_pool_get_stats(thread_pool, &num_threads, NULL);
    if (num_threads <= 0) num_threads = 4;
    
    // 对于小数据集，减少线程数
    if (count < 1000) num_threads = 2;
    if (count < 100) num_threads = 1;
    
    // 创建任务
    simplify_task_t* tasks = (simplify_task_t*)malloc(sizeof(simplify_task_t) * num_threads);
    if (!tasks) {
        cf_mutex_destroy(mutex);
        free(keep);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    // 分段处理
    size_t segment_size = count / num_threads;
    for (int i = 0; i < num_threads; i++) {
        tasks[i].points = points;
        tasks[i].start = i * segment_size;
        tasks[i].end = (i == num_threads - 1) ? (count - 1) : ((i + 1) * segment_size);
        tasks[i].tolerance = tolerance;
        tasks[i].keep = keep;
        tasks[i].mutex = mutex;
        
        // 确保段边界点被保留
        if (i > 0) {
            keep[tasks[i].start] = true;
        }
    }
    
    // 提交任务
    for (int i = 0; i < num_threads; i++) {
        result = cf_thread_pool_submit(thread_pool, simplify_worker, &tasks[i]);
        if (result != CF_SUCCESS) {
            cf_thread_pool_wait(thread_pool);
            free(tasks);
            cf_mutex_destroy(mutex);
            free(keep);
            return result;
        }
    }
    
    // 等待完成
    cf_thread_pool_wait(thread_pool);
    
    // 统计保留的点数
    size_t kept_count = 0;
    for (size_t i = 0; i < count; i++) {
        if (keep[i]) kept_count++;
    }
    
    // 复制保留的点
    cf_point3_t* simplified = (cf_point3_t*)malloc(sizeof(cf_point3_t) * kept_count);
    if (!simplified) {
        free(tasks);
        cf_mutex_destroy(mutex);
        free(keep);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    size_t out_idx = 0;
    for (size_t i = 0; i < count; i++) {
        if (keep[i]) {
            simplified[out_idx++] = points[i];
        }
    }
    
    // 清理
    free(tasks);
    cf_mutex_destroy(mutex);
    free(keep);
    
    *out_points = simplified;
    *out_count = kept_count;
    
    return CF_SUCCESS;
}
