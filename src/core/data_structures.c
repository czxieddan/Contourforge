/**
 * @file data_structures.c
 * @brief Contourforge数据结构实现
 */

#include "contourforge/core.h"
#include "contourforge/types.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

/* ========== 点集实现 ========== */

/**
 * @brief 创建点集
 */
cf_result_t cf_point_set_create(
    size_t initial_capacity,
    cf_point_set_t** point_set
) {
    if (initial_capacity == 0 || point_set == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_point_set_t* ps = (cf_point_set_t*)malloc(sizeof(cf_point_set_t));
    if (ps == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    ps->points = (cf_point3_t*)malloc(sizeof(cf_point3_t) * initial_capacity);
    if (ps->points == NULL) {
        free(ps);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    ps->count = 0;
    ps->capacity = initial_capacity;
    ps->dirty = false;
    
    /* 初始化边界盒为无效值 */
    ps->bounds.min.x = FLT_MAX;
    ps->bounds.min.y = FLT_MAX;
    ps->bounds.min.z = FLT_MAX;
    ps->bounds.max.x = -FLT_MAX;
    ps->bounds.max.y = -FLT_MAX;
    ps->bounds.max.z = -FLT_MAX;
    
    *point_set = ps;
    return CF_SUCCESS;
}

/**
 * @brief 添加点
 */
cf_index_t cf_point_set_add(cf_point_set_t* point_set, cf_point3_t point) {
    if (point_set == NULL) {
        return (cf_index_t)-1;
    }
    
    /* 检查是否需要扩容 */
    if (point_set->count >= point_set->capacity) {
        size_t new_capacity = point_set->capacity * 2;
        cf_point3_t* new_points = (cf_point3_t*)realloc(
            point_set->points,
            sizeof(cf_point3_t) * new_capacity
        );
        
        if (new_points == NULL) {
            return (cf_index_t)-1;
        }
        
        point_set->points = new_points;
        point_set->capacity = new_capacity;
    }
    
    /* 添加点 */
    cf_index_t index = (cf_index_t)point_set->count;
    point_set->points[index] = point;
    point_set->count++;
    point_set->dirty = true;
    
    /* 更新边界盒 */
    if (point.x < point_set->bounds.min.x) point_set->bounds.min.x = point.x;
    if (point.y < point_set->bounds.min.y) point_set->bounds.min.y = point.y;
    if (point.z < point_set->bounds.min.z) point_set->bounds.min.z = point.z;
    if (point.x > point_set->bounds.max.x) point_set->bounds.max.x = point.x;
    if (point.y > point_set->bounds.max.y) point_set->bounds.max.y = point.y;
    if (point.z > point_set->bounds.max.z) point_set->bounds.max.z = point.z;
    
    return index;
}

/**
 * @brief 获取点
 */
const cf_point3_t* cf_point_set_get(const cf_point_set_t* point_set, cf_index_t index) {
    if (point_set == NULL || index >= point_set->count) {
        return NULL;
    }
    
    return &point_set->points[index];
}

/**
 * @brief 更新点
 */
cf_result_t cf_point_set_update(
    cf_point_set_t* point_set,
    cf_index_t index,
    cf_point3_t point
) {
    if (point_set == NULL || index >= point_set->count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    point_set->points[index] = point;
    point_set->dirty = true;
    
    /* 标记需要重新计算边界盒 */
    return CF_SUCCESS;
}

/**
 * @brief 更新边界盒
 */
void cf_point_set_update_bounds(cf_point_set_t* point_set) {
    if (point_set == NULL || point_set->count == 0) {
        return;
    }
    
    /* 重新计算边界盒 */
    point_set->bounds.min.x = FLT_MAX;
    point_set->bounds.min.y = FLT_MAX;
    point_set->bounds.min.z = FLT_MAX;
    point_set->bounds.max.x = -FLT_MAX;
    point_set->bounds.max.y = -FLT_MAX;
    point_set->bounds.max.z = -FLT_MAX;
    
    for (size_t i = 0; i < point_set->count; i++) {
        const cf_point3_t* p = &point_set->points[i];
        
        if (p->x < point_set->bounds.min.x) point_set->bounds.min.x = p->x;
        if (p->y < point_set->bounds.min.y) point_set->bounds.min.y = p->y;
        if (p->z < point_set->bounds.min.z) point_set->bounds.min.z = p->z;
        if (p->x > point_set->bounds.max.x) point_set->bounds.max.x = p->x;
        if (p->y > point_set->bounds.max.y) point_set->bounds.max.y = p->y;
        if (p->z > point_set->bounds.max.z) point_set->bounds.max.z = p->z;
    }
}

/**
 * @brief 清空点集
 */
void cf_point_set_clear(cf_point_set_t* point_set) {
    if (point_set == NULL) {
        return;
    }
    
    point_set->count = 0;
    point_set->dirty = true;
    
    /* 重置边界盒 */
    point_set->bounds.min.x = FLT_MAX;
    point_set->bounds.min.y = FLT_MAX;
    point_set->bounds.min.z = FLT_MAX;
    point_set->bounds.max.x = -FLT_MAX;
    point_set->bounds.max.y = -FLT_MAX;
    point_set->bounds.max.z = -FLT_MAX;
}

/**
 * @brief 销毁点集
 */
void cf_point_set_destroy(cf_point_set_t* point_set) {
    if (point_set == NULL) {
        return;
    }
    
    free(point_set->points);
    free(point_set);
}

/* ========== 线集实现 ========== */

/**
 * @brief 创建线集
 */
cf_result_t cf_line_set_create(
    cf_point_set_t* point_set,
    size_t initial_capacity,
    cf_line_set_t** line_set
) {
    if (point_set == NULL || initial_capacity == 0 || line_set == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_line_set_t* ls = (cf_line_set_t*)malloc(sizeof(cf_line_set_t));
    if (ls == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    ls->lines = (cf_line_t*)malloc(sizeof(cf_line_t) * initial_capacity);
    if (ls->lines == NULL) {
        free(ls);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    ls->count = 0;
    ls->capacity = initial_capacity;
    ls->point_set = point_set;
    
    *line_set = ls;
    return CF_SUCCESS;
}

/**
 * @brief 添加线段
 */
cf_index_t cf_line_set_add(cf_line_set_t* line_set, cf_index_t p1, cf_index_t p2) {
    if (line_set == NULL) {
        return (cf_index_t)-1;
    }
    
    /* 验证点索引 */
    if (p1 >= line_set->point_set->count || p2 >= line_set->point_set->count) {
        return (cf_index_t)-1;
    }
    
    /* 检查是否需要扩容 */
    if (line_set->count >= line_set->capacity) {
        size_t new_capacity = line_set->capacity * 2;
        cf_line_t* new_lines = (cf_line_t*)realloc(
            line_set->lines,
            sizeof(cf_line_t) * new_capacity
        );
        
        if (new_lines == NULL) {
            return (cf_index_t)-1;
        }
        
        line_set->lines = new_lines;
        line_set->capacity = new_capacity;
    }
    
    /* 添加线段 */
    cf_index_t index = (cf_index_t)line_set->count;
    line_set->lines[index].p1 = p1;
    line_set->lines[index].p2 = p2;
    line_set->count++;
    
    return index;
}

/**
 * @brief 获取线段
 */
const cf_line_t* cf_line_set_get(const cf_line_set_t* line_set, cf_index_t index) {
    if (line_set == NULL || index >= line_set->count) {
        return NULL;
    }
    
    return &line_set->lines[index];
}

/**
 * @brief 清空线集
 */
void cf_line_set_clear(cf_line_set_t* line_set) {
    if (line_set == NULL) {
        return;
    }
    
    line_set->count = 0;
}

/**
 * @brief 销毁线集
 */
void cf_line_set_destroy(cf_line_set_t* line_set) {
    if (line_set == NULL) {
        return;
    }
    
    free(line_set->lines);
    free(line_set);
}

/* ========== 面集实现 ========== */

/**
 * @brief 面（三角形）
 */
typedef struct {
    cf_index_t p1;  /**< 顶点1索引 */
    cf_index_t p2;  /**< 顶点2索引 */
    cf_index_t p3;  /**< 顶点3索引 */
} cf_face_t;

/**
 * @brief 面集
 */
typedef struct {
    cf_face_t* faces;               /**< 面数组 */
    size_t count;                   /**< 面数量 */
    size_t capacity;                /**< 容量 */
    cf_point_set_t* point_set;      /**< 关联的点集 */
} cf_face_set_t;

/**
 * @brief 创建面集
 */
cf_result_t cf_face_set_create(
    cf_point_set_t* point_set,
    size_t initial_capacity,
    cf_face_set_t** face_set
) {
    if (point_set == NULL || initial_capacity == 0 || face_set == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_face_set_t* fs = (cf_face_set_t*)malloc(sizeof(cf_face_set_t));
    if (fs == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    fs->faces = (cf_face_t*)malloc(sizeof(cf_face_t) * initial_capacity);
    if (fs->faces == NULL) {
        free(fs);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    fs->count = 0;
    fs->capacity = initial_capacity;
    fs->point_set = point_set;
    
    *face_set = fs;
    return CF_SUCCESS;
}

/**
 * @brief 添加面
 */
cf_index_t cf_face_set_add(
    cf_face_set_t* face_set,
    cf_index_t p1,
    cf_index_t p2,
    cf_index_t p3
) {
    if (face_set == NULL) {
        return (cf_index_t)-1;
    }
    
    /* 验证点索引 */
    if (p1 >= face_set->point_set->count ||
        p2 >= face_set->point_set->count ||
        p3 >= face_set->point_set->count) {
        return (cf_index_t)-1;
    }
    
    /* 检查是否需要扩容 */
    if (face_set->count >= face_set->capacity) {
        size_t new_capacity = face_set->capacity * 2;
        cf_face_t* new_faces = (cf_face_t*)realloc(
            face_set->faces,
            sizeof(cf_face_t) * new_capacity
        );
        
        if (new_faces == NULL) {
            return (cf_index_t)-1;
        }
        
        face_set->faces = new_faces;
        face_set->capacity = new_capacity;
    }
    
    /* 添加面 */
    cf_index_t index = (cf_index_t)face_set->count;
    face_set->faces[index].p1 = p1;
    face_set->faces[index].p2 = p2;
    face_set->faces[index].p3 = p3;
    face_set->count++;
    
    return index;
}

/**
 * @brief 销毁面集
 */
void cf_face_set_destroy(cf_face_set_t* face_set) {
    if (face_set == NULL) {
        return;
    }
    
    free(face_set->faces);
    free(face_set);
}

/* ========== 模型实现 ========== */

/**
 * @brief 创建模型
 */
cf_result_t cf_model_create(const char* name, cf_model_t** model) {
    if (model == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_model_t* m = (cf_model_t*)malloc(sizeof(cf_model_t));
    if (m == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    /* 创建点集 */
    cf_result_t result = cf_point_set_create(1024, &m->points);
    if (CF_FAILED(result)) {
        free(m);
        return result;
    }
    
    /* 创建线集 */
    result = cf_line_set_create(m->points, 1024, &m->lines);
    if (CF_FAILED(result)) {
        cf_point_set_destroy(m->points);
        free(m);
        return result;
    }
    
    /* 复制名称 */
    if (name != NULL) {
        size_t name_len = strlen(name);
        m->name = (char*)malloc(name_len + 1);
        if (m->name == NULL) {
            cf_line_set_destroy(m->lines);
            cf_point_set_destroy(m->points);
            free(m);
            return CF_ERROR_OUT_OF_MEMORY;
        }
        strcpy(m->name, name);
    } else {
        m->name = NULL;
    }
    
    /* 初始化边界盒 */
    m->bounds.min.x = FLT_MAX;
    m->bounds.min.y = FLT_MAX;
    m->bounds.min.z = FLT_MAX;
    m->bounds.max.x = -FLT_MAX;
    m->bounds.max.y = -FLT_MAX;
    m->bounds.max.z = -FLT_MAX;
    
    *model = m;
    return CF_SUCCESS;
}

/**
 * @brief 更新模型边界盒
 */
void cf_model_update_bounds(cf_model_t* model) {
    if (model == NULL || model->points == NULL) {
        return;
    }
    
    /* 更新点集边界盒 */
    cf_point_set_update_bounds(model->points);
    
    /* 复制到模型边界盒 */
    model->bounds = model->points->bounds;
}

/**
 * @brief 获取模型中心点
 */
cf_point3_t cf_model_get_center(const cf_model_t* model) {
    cf_point3_t center = {0.0f, 0.0f, 0.0f};
    
    if (model == NULL) {
        return center;
    }
    
    center.x = (model->bounds.min.x + model->bounds.max.x) * 0.5f;
    center.y = (model->bounds.min.y + model->bounds.max.y) * 0.5f;
    center.z = (model->bounds.min.z + model->bounds.max.z) * 0.5f;
    
    return center;
}

/**
 * @brief 获取模型尺寸
 */
cf_vec3_t cf_model_get_size(const cf_model_t* model) {
    cf_vec3_t size = {0.0f, 0.0f, 0.0f};
    
    if (model == NULL) {
        return size;
    }
    
    size.x = model->bounds.max.x - model->bounds.min.x;
    size.y = model->bounds.max.y - model->bounds.min.y;
    size.z = model->bounds.max.z - model->bounds.min.z;
    
    return size;
}

/**
 * @brief 销毁模型
 */
void cf_model_destroy(cf_model_t* model) {
    if (model == NULL) {
        return;
    }
    
    if (model->lines != NULL) {
        cf_line_set_destroy(model->lines);
    }
    
    if (model->points != NULL) {
        cf_point_set_destroy(model->points);
    }
    
    if (model->name != NULL) {
        free(model->name);
    }
    
    free(model);
}
