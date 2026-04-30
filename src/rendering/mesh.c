/**
 * @file mesh.c
 * @brief Contourforge网格管理实现
 */

#include "contourforge/rendering.h"
#include "contourforge/core.h"
#include "contourforge/types.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <string.h>

/* ========== 网格结构 ========== */

/**
 * @brief 渲染网格
 */
typedef struct {
    unsigned int vao;           /**< VAO */
    unsigned int vbo;           /**< VBO */
    unsigned int ebo;           /**< EBO（可选） */
    
    size_t vertex_count;        /**< 顶点数量 */
    size_t index_count;         /**< 索引数量 */
    
    cf_model_t* model;          /**< 关联的模型 */
    
    bool use_indices;           /**< 是否使用索引 */
    bool dirty;                 /**< 是否需要更新 */
} cf_mesh_t;

/* ========== 网格API ========== */

/**
 * @brief 从模型创建网格（点模式）
 */
cf_result_t cf_mesh_create_from_points(
    cf_model_t* model,
    cf_mesh_t** mesh
) {
    if (model == NULL || model->points == NULL || mesh == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_mesh_t* m = (cf_mesh_t*)malloc(sizeof(cf_mesh_t));
    if (m == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    m->model = model;
    m->vertex_count = model->points->count;
    m->index_count = 0;
    m->use_indices = false;
    m->dirty = false;
    
    /* 创建VAO */
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);
    
    /* 创建VBO */
    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(cf_point3_t) * m->vertex_count,
        model->points->points,
        GL_STATIC_DRAW
    );
    
    /* 设置顶点属性 */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cf_point3_t), (void*)0);
    
    glBindVertexArray(0);
    
    m->ebo = 0;
    
    *mesh = m;
    return CF_SUCCESS;
}

/**
 * @brief 从模型创建网格（线模式）
 */
cf_result_t cf_mesh_create_from_lines(
    cf_model_t* model,
    cf_mesh_t** mesh
) {
    if (model == NULL || model->points == NULL || model->lines == NULL || mesh == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_mesh_t* m = (cf_mesh_t*)malloc(sizeof(cf_mesh_t));
    if (m == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    m->model = model;
    m->vertex_count = model->points->count;
    m->index_count = model->lines->count * 2;
    m->use_indices = true;
    m->dirty = false;
    
    /* 创建VAO */
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);
    
    /* 创建VBO */
    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(cf_point3_t) * m->vertex_count,
        model->points->points,
        GL_STATIC_DRAW
    );
    
    /* 设置顶点属性 */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cf_point3_t), (void*)0);
    
    /* 创建EBO */
    unsigned int* indices = (unsigned int*)malloc(sizeof(unsigned int) * m->index_count);
    if (indices == NULL) {
        glDeleteBuffers(1, &m->vbo);
        glDeleteVertexArrays(1, &m->vao);
        free(m);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    /* 填充索引 */
    for (size_t i = 0; i < model->lines->count; i++) {
        indices[i * 2 + 0] = model->lines->lines[i].p1;
        indices[i * 2 + 1] = model->lines->lines[i].p2;
    }
    
    glGenBuffers(1, &m->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(unsigned int) * m->index_count,
        indices,
        GL_STATIC_DRAW
    );
    
    free(indices);
    
    glBindVertexArray(0);
    
    *mesh = m;
    return CF_SUCCESS;
}

/**
 * @brief 更新网格数据
 */
cf_result_t cf_mesh_update(cf_mesh_t* mesh) {
    if (mesh == NULL || mesh->model == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 更新VBO */
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(cf_point3_t) * mesh->vertex_count,
        mesh->model->points->points
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    mesh->dirty = false;
    
    return CF_SUCCESS;
}

/**
 * @brief 绘制网格（点）
 */
void cf_mesh_draw_points(const cf_mesh_t* mesh) {
    if (mesh == NULL) {
        return;
    }
    
    glBindVertexArray(mesh->vao);
    glDrawArrays(GL_POINTS, 0, (GLsizei)mesh->vertex_count);
    glBindVertexArray(0);
}

/**
 * @brief 绘制网格（线）
 */
void cf_mesh_draw_lines(const cf_mesh_t* mesh) {
    if (mesh == NULL) {
        return;
    }
    
    glBindVertexArray(mesh->vao);
    
    if (mesh->use_indices) {
        glDrawElements(GL_LINES, (GLsizei)mesh->index_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_LINES, 0, (GLsizei)mesh->vertex_count);
    }
    
    glBindVertexArray(0);
}

/**
 * @brief 绘制网格（线框）
 */
void cf_mesh_draw_wireframe(const cf_mesh_t* mesh) {
    if (mesh == NULL) {
        return;
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glBindVertexArray(mesh->vao);
    
    if (mesh->use_indices) {
        glDrawElements(GL_TRIANGLES, (GLsizei)mesh->index_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mesh->vertex_count);
    }
    
    glBindVertexArray(0);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/**
 * @brief 获取顶点数量
 */
size_t cf_mesh_get_vertex_count(const cf_mesh_t* mesh) {
    if (mesh == NULL) {
        return 0;
    }
    
    return mesh->vertex_count;
}

/**
 * @brief 获取索引数量
 */
size_t cf_mesh_get_index_count(const cf_mesh_t* mesh) {
    if (mesh == NULL) {
        return 0;
    }
    
    return mesh->index_count;
}

/**
 * @brief 销毁网格
 */
void cf_mesh_destroy(cf_mesh_t* mesh) {
    if (mesh == NULL) {
        return;
    }
    
    if (mesh->ebo != 0) {
        glDeleteBuffers(1, &mesh->ebo);
    }
    
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteVertexArrays(1, &mesh->vao);
    
    free(mesh);
}
