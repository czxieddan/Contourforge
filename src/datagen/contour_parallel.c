/**
 * @file contour_parallel.c
 * @brief 并行等高线提取实现
 */

#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include "../../include/contourforge/threading.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ========== 并行提取任务 ========== */

typedef struct {
    const cf_heightmap_t* heightmap;
    float iso_value;
    int start_row;
    int end_row;
    
    // 输出（每个线程独立）
    cf_point3_t* points;
    size_t point_count;
    size_t point_capacity;
    
    cf_line_t* lines;
    size_t line_count;
    size_t line_capacity;
    
    cf_mutex_t* mutex;  // 用于错误报告
    cf_result_t result;
} contour_extract_task_t;

/**
 * @brief 线性插值计算等高线与边的交点
 */
static cf_point3_t interpolate_edge(
    cf_point3_t p1, float h1,
    cf_point3_t p2, float h2,
    float iso_value
) {
    cf_point3_t result;
    
    if (fabsf(h1 - h2) < 1e-6f) {
        result.x = (p1.x + p2.x) * 0.5f;
        result.y = (p1.y + p2.y) * 0.5f;
        result.z = (p1.z + p2.z) * 0.5f;
        return result;
    }
    
    float t = (iso_value - h1) / (h2 - h1);
    result.x = p1.x + t * (p2.x - p1.x);
    result.y = p1.y + t * (p2.y - p1.y);
    result.z = p1.z + t * (p2.z - p1.z);
    
    return result;
}

// Marching Squares查找表
static const int MS_EDGE_TABLE[16] = {
    0x0, 0x9, 0x3, 0xA,
    0x6, 0xF, 0x5, 0xC,
    0xC, 0x5, 0xF, 0x6,
    0xA, 0x3, 0x9, 0x0
};

/**
 * @brief 添加点到本地缓冲区
 */
static cf_index_t task_add_point(contour_extract_task_t* task, cf_point3_t point) {
    if (task->point_count >= task->point_capacity) {
        size_t new_capacity = task->point_capacity * 2;
        cf_point3_t* new_points = (cf_point3_t*)realloc(
            task->points,
            sizeof(cf_point3_t) * new_capacity
        );
        if (!new_points) {
            task->result = CF_ERROR_OUT_OF_MEMORY;
            return 0;
        }
        task->points = new_points;
        task->point_capacity = new_capacity;
    }
    
    task->points[task->point_count] = point;
    return (cf_index_t)(task->point_count++);
}

/**
 * @brief 添加线段到本地缓冲区
 */
static void task_add_line(contour_extract_task_t* task, cf_index_t p1, cf_index_t p2) {
    if (task->line_count >= task->line_capacity) {
        size_t new_capacity = task->line_capacity * 2;
        cf_line_t* new_lines = (cf_line_t*)realloc(
            task->lines,
            sizeof(cf_line_t) * new_capacity
        );
        if (!new_lines) {
            task->result = CF_ERROR_OUT_OF_MEMORY;
            return;
        }
        task->lines = new_lines;
        task->line_capacity = new_capacity;
    }
    
    task->lines[task->line_count].p1 = p1;
    task->lines[task->line_count].p2 = p2;
    task->line_count++;
}

/**
 * @brief 并行提取任务工作函数
 */
static void extract_contour_worker(void* arg) {
    contour_extract_task_t* task = (contour_extract_task_t*)arg;
    
    const cf_heightmap_t* heightmap = task->heightmap;
    float iso_value = task->iso_value;
    int width = heightmap->width;
    
    // 遍历分配的行范围
    for (int y = task->start_row; y < task->end_row && y < heightmap->height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            // 获取格子四个角的高度
            float h00 = heightmap->data[y * width + x];
            float h10 = heightmap->data[y * width + (x + 1)];
            float h01 = heightmap->data[(y + 1) * width + x];
            float h11 = heightmap->data[(y + 1) * width + (x + 1)];
            
            // 计算格子配置
            int config = 0;
            if (h00 >= iso_value) config |= 1;
            if (h10 >= iso_value) config |= 2;
            if (h11 >= iso_value) config |= 4;
            if (h01 >= iso_value) config |= 8;
            
            // 跳过完全在上方或下方的格子
            if (config == 0 || config == 15) {
                continue;
            }
            
            // 格子四个角的3D坐标
            cf_point3_t p00 = {(float)x, (float)y, h00};
            cf_point3_t p10 = {(float)(x + 1), (float)y, h10};
            cf_point3_t p01 = {(float)x, (float)(y + 1), h01};
            cf_point3_t p11 = {(float)(x + 1), (float)(y + 1), h11};
            
            // 根据配置生成线段
            int edges = MS_EDGE_TABLE[config];
            if (edges == 0) continue;
            
            // 计算边上的交点
            cf_point3_t edge_points[4];
            int edge_count = 0;
            
            if (edges & 0x1) {
                edge_points[edge_count++] = interpolate_edge(p00, h00, p10, h10, iso_value);
            }
            if (edges & 0x2) {
                edge_points[edge_count++] = interpolate_edge(p10, h10, p11, h11, iso_value);
            }
            if (edges & 0x4) {
                edge_points[edge_count++] = interpolate_edge(p11, h11, p01, h01, iso_value);
            }
            if (edges & 0x8) {
                edge_points[edge_count++] = interpolate_edge(p01, h01, p00, h00, iso_value);
            }
            
            // 生成线段
            if (edge_count >= 2) {
                cf_index_t idx1 = task_add_point(task, edge_points[0]);
                cf_index_t idx2 = task_add_point(task, edge_points[1]);
                task_add_line(task, idx1, idx2);
            }
            
            // 处理鞍点情况
            if (edge_count == 4) {
                cf_index_t idx3 = task_add_point(task, edge_points[2]);
                cf_index_t idx4 = task_add_point(task, edge_points[3]);
                task_add_line(task, idx3, idx4);
            }
        }
    }
}

/**
 * @brief 并行提取等高线
 */
cf_result_t cf_contour_extract_parallel(
    const cf_heightmap_t* heightmap,
    float iso_value,
    cf_thread_pool_t* thread_pool,
    cf_point_set_t* point_set,
    cf_line_set_t* line_set
) {
    if (!heightmap || !point_set || !line_set) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 如果没有线程池，使用单线程版本
    if (!thread_pool) {
        // 调用原始的单线程实现（需要在contour.c中导出）
        return CF_ERROR_INVALID_PARAM;  // 暂时返回错误
    }
    
    // 获取线程池信息
    int num_threads = 0;
    cf_thread_pool_get_stats(thread_pool, &num_threads, NULL);
    if (num_threads <= 0) num_threads = 4;  // 默认值
    
    // 计算每个线程处理的行数
    int rows_per_thread = (heightmap->height + num_threads - 1) / num_threads;
    if (rows_per_thread < 1) rows_per_thread = 1;
    
    // 创建任务
    contour_extract_task_t* tasks = (contour_extract_task_t*)malloc(
        sizeof(contour_extract_task_t) * num_threads
    );
    if (!tasks) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    cf_mutex_t* error_mutex;
    cf_result_t result = cf_mutex_create(&error_mutex);
    if (result != CF_SUCCESS) {
        free(tasks);
        return result;
    }
    
    // 初始化任务
    for (int i = 0; i < num_threads; i++) {
        tasks[i].heightmap = heightmap;
        tasks[i].iso_value = iso_value;
        tasks[i].start_row = i * rows_per_thread;
        tasks[i].end_row = (i + 1) * rows_per_thread;
        if (tasks[i].end_row > heightmap->height) {
            tasks[i].end_row = heightmap->height;
        }
        
        // 预分配缓冲区
        size_t estimated_points = (size_t)(heightmap->width * rows_per_thread * 2);
        tasks[i].point_capacity = estimated_points > 1024 ? estimated_points : 1024;
        tasks[i].points = (cf_point3_t*)malloc(sizeof(cf_point3_t) * tasks[i].point_capacity);
        tasks[i].point_count = 0;
        
        tasks[i].line_capacity = tasks[i].point_capacity / 2;
        tasks[i].lines = (cf_line_t*)malloc(sizeof(cf_line_t) * tasks[i].line_capacity);
        tasks[i].line_count = 0;
        
        tasks[i].mutex = error_mutex;
        tasks[i].result = CF_SUCCESS;
        
        if (!tasks[i].points || !tasks[i].lines) {
            // 清理已分配的内存
            for (int j = 0; j <= i; j++) {
                free(tasks[j].points);
                free(tasks[j].lines);
            }
            free(tasks);
            cf_mutex_destroy(error_mutex);
            return CF_ERROR_OUT_OF_MEMORY;
        }
    }
    
    // 提交任务到线程池
    for (int i = 0; i < num_threads; i++) {
        result = cf_thread_pool_submit(thread_pool, extract_contour_worker, &tasks[i]);
        if (result != CF_SUCCESS) {
            // 等待已提交的任务完成
            cf_thread_pool_wait(thread_pool);
            
            // 清理
            for (int j = 0; j < num_threads; j++) {
                free(tasks[j].points);
                free(tasks[j].lines);
            }
            free(tasks);
            cf_mutex_destroy(error_mutex);
            return result;
        }
    }
    
    // 等待所有任务完成
    cf_thread_pool_wait(thread_pool);
    
    // 检查错误
    for (int i = 0; i < num_threads; i++) {
        if (tasks[i].result != CF_SUCCESS) {
            result = tasks[i].result;
            break;
        }
    }
    
    // 合并结果
    if (result == CF_SUCCESS) {
        for (int i = 0; i < num_threads; i++) {
            // 记录当前点集的偏移量
            size_t point_offset = point_set->count;
            
            // 添加点
            for (size_t j = 0; j < tasks[i].point_count; j++) {
                cf_point_set_add(point_set, tasks[i].points[j]);
            }
            
            // 添加线段（调整索引）
            for (size_t j = 0; j < tasks[i].line_count; j++) {
                cf_line_set_add(
                    line_set,
                    (cf_index_t)(tasks[i].lines[j].p1 + point_offset),
                    (cf_index_t)(tasks[i].lines[j].p2 + point_offset)
                );
            }
        }
    }
    
    // 清理
    for (int i = 0; i < num_threads; i++) {
        free(tasks[i].points);
        free(tasks[i].lines);
    }
    free(tasks);
    cf_mutex_destroy(error_mutex);
    
    return result;
}
