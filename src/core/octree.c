/**
 * @file octree.c
 * @brief Contourforge八叉树空间索引实现
 */

#include "contourforge/core.h"
#include "contourforge/types.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ========== 八叉树节点 ========== */

/**
 * @brief 八叉树节点
 */
struct cf_octree_node {
    cf_bounds_t bounds;                 /**< 节点边界 */
    cf_octree_node_t* children[8];      /**< 8个子节点 */
    cf_index_t* indices;                /**< 点索引数组 */
    size_t count;                       /**< 点数量 */
    size_t capacity;                    /**< 容量 */
    bool is_leaf;                       /**< 是否为叶子节点 */
};

/* ========== 辅助函数 ========== */

/**
 * @brief 检查点是否在边界内
 */
static bool point_in_bounds(cf_point3_t point, cf_bounds_t bounds) {
    return point.x >= bounds.min.x && point.x <= bounds.max.x &&
           point.y >= bounds.min.y && point.y <= bounds.max.y &&
           point.z >= bounds.min.z && point.z <= bounds.max.z;
}

/**
 * @brief 检查两个边界是否相交
 */
static bool bounds_intersect(cf_bounds_t a, cf_bounds_t b) {
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y &&
           a.min.z <= b.max.z && a.max.z >= b.min.z;
}

/**
 * @brief 计算子节点边界
 */
static cf_bounds_t get_child_bounds(cf_bounds_t parent, int octant) {
    cf_bounds_t child;
    
    float mid_x = (parent.min.x + parent.max.x) * 0.5f;
    float mid_y = (parent.min.y + parent.max.y) * 0.5f;
    float mid_z = (parent.min.z + parent.max.z) * 0.5f;
    
    /* 根据八分位确定子节点边界 */
    child.min.x = (octant & 1) ? mid_x : parent.min.x;
    child.max.x = (octant & 1) ? parent.max.x : mid_x;
    
    child.min.y = (octant & 2) ? mid_y : parent.min.y;
    child.max.y = (octant & 2) ? parent.max.y : mid_y;
    
    child.min.z = (octant & 4) ? mid_z : parent.min.z;
    child.max.z = (octant & 4) ? parent.max.z : mid_z;
    
    return child;
}

/**
 * @brief 确定点所在的八分位
 */
static int get_octant(cf_point3_t point, cf_bounds_t bounds) {
    int octant = 0;
    
    float mid_x = (bounds.min.x + bounds.max.x) * 0.5f;
    float mid_y = (bounds.min.y + bounds.max.y) * 0.5f;
    float mid_z = (bounds.min.z + bounds.max.z) * 0.5f;
    
    if (point.x >= mid_x) octant |= 1;
    if (point.y >= mid_y) octant |= 2;
    if (point.z >= mid_z) octant |= 4;
    
    return octant;
}

/**
 * @brief 创建八叉树节点
 */
static cf_octree_node_t* create_node(cf_bounds_t bounds, size_t max_points) {
    cf_octree_node_t* node = (cf_octree_node_t*)malloc(sizeof(cf_octree_node_t));
    if (node == NULL) {
        return NULL;
    }
    
    node->bounds = bounds;
    node->is_leaf = true;
    node->count = 0;
    node->capacity = max_points;
    
    node->indices = (cf_index_t*)malloc(sizeof(cf_index_t) * max_points);
    if (node->indices == NULL) {
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < 8; i++) {
        node->children[i] = NULL;
    }
    
    return node;
}

/**
 * @brief 分裂节点
 */
static cf_result_t split_node(
    cf_octree_node_t* node,
    size_t max_depth,
    size_t current_depth,
    size_t max_points,
    const cf_point3_t* points
) {
    if (node == NULL || !node->is_leaf || current_depth >= max_depth) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 创建8个子节点 */
    for (int i = 0; i < 8; i++) {
        cf_bounds_t child_bounds = get_child_bounds(node->bounds, i);
        node->children[i] = create_node(child_bounds, max_points);
        
        if (node->children[i] == NULL) {
            /* 清理已创建的子节点 */
            for (int j = 0; j < i; j++) {
                free(node->children[j]->indices);
                free(node->children[j]);
            }
            return CF_ERROR_OUT_OF_MEMORY;
        }
    }
    
    /* 将当前节点的点分配到子节点 */
    for (size_t i = 0; i < node->count; i++) {
        cf_index_t idx = node->indices[i];
        cf_point3_t point = points[idx];
        
        int octant = get_octant(point, node->bounds);
        cf_octree_node_t* child = node->children[octant];
        
        child->indices[child->count++] = idx;
    }
    
    /* 释放当前节点的索引数组 */
    free(node->indices);
    node->indices = NULL;
    node->count = 0;
    node->capacity = 0;
    node->is_leaf = false;
    
    return CF_SUCCESS;
}

/**
 * @brief 递归插入点
 */
static cf_result_t insert_recursive(
    cf_octree_node_t* node,
    cf_point3_t point,
    cf_index_t index,
    size_t max_depth,
    size_t current_depth,
    size_t max_points,
    const cf_point3_t* points
) {
    if (node == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 检查点是否在节点边界内 */
    if (!point_in_bounds(point, node->bounds)) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    if (node->is_leaf) {
        /* 叶子节点：添加点 */
        if (node->count < node->capacity) {
            node->indices[node->count++] = index;
            return CF_SUCCESS;
        }
        
        /* 节点已满，需要分裂 */
        if (current_depth < max_depth) {
            cf_result_t result = split_node(node, max_depth, current_depth, max_points, points);
            if (CF_FAILED(result)) {
                return result;
            }
            
            /* 分裂后继续插入 */
            int octant = get_octant(point, node->bounds);
            return insert_recursive(
                node->children[octant],
                point,
                index,
                max_depth,
                current_depth + 1,
                max_points,
                points
            );
        } else {
            /* 已达到最大深度，强制添加 */
            /* 扩容索引数组 */
            size_t new_capacity = node->capacity * 2;
            cf_index_t* new_indices = (cf_index_t*)realloc(
                node->indices,
                sizeof(cf_index_t) * new_capacity
            );
            
            if (new_indices == NULL) {
                return CF_ERROR_OUT_OF_MEMORY;
            }
            
            node->indices = new_indices;
            node->capacity = new_capacity;
            node->indices[node->count++] = index;
            
            return CF_SUCCESS;
        }
    } else {
        /* 非叶子节点：递归插入到子节点 */
        int octant = get_octant(point, node->bounds);
        return insert_recursive(
            node->children[octant],
            point,
            index,
            max_depth,
            current_depth + 1,
            max_points,
            points
        );
    }
}

/**
 * @brief 递归查询
 */
static void query_recursive(
    const cf_octree_node_t* node,
    cf_bounds_t query_bounds,
    cf_index_t** indices,
    size_t* count,
    size_t* capacity
) {
    if (node == NULL) {
        return;
    }
    
    /* 检查节点边界是否与查询边界相交 */
    if (!bounds_intersect(node->bounds, query_bounds)) {
        return;
    }
    
    if (node->is_leaf) {
        /* 叶子节点：添加所有点 */
        for (size_t i = 0; i < node->count; i++) {
            /* 检查是否需要扩容 */
            if (*count >= *capacity) {
                size_t new_capacity = (*capacity) * 2;
                cf_index_t* new_indices = (cf_index_t*)realloc(
                    *indices,
                    sizeof(cf_index_t) * new_capacity
                );
                
                if (new_indices == NULL) {
                    return;
                }
                
                *indices = new_indices;
                *capacity = new_capacity;
            }
            
            (*indices)[(*count)++] = node->indices[i];
        }
    } else {
        /* 非叶子节点：递归查询子节点 */
        for (int i = 0; i < 8; i++) {
            query_recursive(node->children[i], query_bounds, indices, count, capacity);
        }
    }
}

/**
 * @brief 递归销毁节点
 */
static void destroy_node(cf_octree_node_t* node) {
    if (node == NULL) {
        return;
    }
    
    if (node->is_leaf) {
        free(node->indices);
    } else {
        for (int i = 0; i < 8; i++) {
            destroy_node(node->children[i]);
        }
    }
    
    free(node);
}

/* ========== 八叉树实现 ========== */

/**
 * @brief 创建八叉树
 */
cf_result_t cf_octree_create(
    cf_bounds_t bounds,
    size_t max_depth,
    size_t max_points,
    cf_octree_t** octree
) {
    if (max_depth == 0 || max_points == 0 || octree == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_octree_t* tree = (cf_octree_t*)malloc(sizeof(cf_octree_t));
    if (tree == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    tree->bounds = bounds;
    tree->max_depth = max_depth;
    tree->max_points = max_points;
    
    tree->root = create_node(bounds, max_points);
    if (tree->root == NULL) {
        free(tree);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    *octree = tree;
    return CF_SUCCESS;
}

/**
 * @brief 插入点（需要点数组用于分裂时重新分配）
 */
cf_result_t cf_octree_insert_with_points(
    cf_octree_t* octree,
    cf_point3_t point,
    cf_index_t index,
    const cf_point3_t* points
) {
    if (octree == NULL || points == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    return insert_recursive(
        octree->root,
        point,
        index,
        octree->max_depth,
        0,
        octree->max_points,
        points
    );
}

/**
 * @brief 插入点（简化版本）
 */
cf_result_t cf_octree_insert(
    cf_octree_t* octree,
    cf_point3_t point,
    cf_index_t index
) {
    /* 注意：这个版本不支持节点分裂，因为没有点数组 */
    /* 实际使用时应该使用cf_octree_insert_with_points */
    if (octree == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 简单实现：只在叶子节点添加 */
    cf_octree_node_t* node = octree->root;
    size_t depth = 0;
    
    while (!node->is_leaf && depth < octree->max_depth) {
        int octant = get_octant(point, node->bounds);
        node = node->children[octant];
        depth++;
    }
    
    if (node->is_leaf) {
        if (node->count < node->capacity) {
            node->indices[node->count++] = index;
            return CF_SUCCESS;
        } else {
            /* 节点已满，需要扩容 */
            size_t new_capacity = node->capacity * 2;
            cf_index_t* new_indices = (cf_index_t*)realloc(
                node->indices,
                sizeof(cf_index_t) * new_capacity
            );
            
            if (new_indices == NULL) {
                return CF_ERROR_OUT_OF_MEMORY;
            }
            
            node->indices = new_indices;
            node->capacity = new_capacity;
            node->indices[node->count++] = index;
            
            return CF_SUCCESS;
        }
    }
    
    return CF_ERROR_UNKNOWN;
}

/**
 * @brief 查询边界内的点
 */
cf_result_t cf_octree_query(
    const cf_octree_t* octree,
    cf_bounds_t bounds,
    cf_index_t** indices,
    size_t* count
) {
    if (octree == NULL || indices == NULL || count == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 初始化结果数组 */
    size_t capacity = 64;
    *indices = (cf_index_t*)malloc(sizeof(cf_index_t) * capacity);
    if (*indices == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    *count = 0;
    
    /* 递归查询 */
    query_recursive(octree->root, bounds, indices, count, &capacity);
    
    return CF_SUCCESS;
}

/**
 * @brief 清空八叉树
 */
void cf_octree_clear(cf_octree_t* octree) {
    if (octree == NULL) {
        return;
    }
    
    /* 销毁根节点并重新创建 */
    destroy_node(octree->root);
    octree->root = create_node(octree->bounds, octree->max_points);
}

/**
 * @brief 销毁八叉树
 */
void cf_octree_destroy(cf_octree_t* octree) {
    if (octree == NULL) {
        return;
    }
    
    destroy_node(octree->root);
    free(octree);
}

/**
 * @brief 获取八叉树统计信息
 */
void cf_octree_get_stats(
    const cf_octree_t* octree,
    size_t* node_count,
    size_t* leaf_count,
    size_t* max_depth_reached
) {
    /* TODO: 实现统计信息收集 */
    if (octree == NULL) {
        return;
    }
    
    if (node_count != NULL) *node_count = 0;
    if (leaf_count != NULL) *leaf_count = 0;
    if (max_depth_reached != NULL) *max_depth_reached = 0;
}
