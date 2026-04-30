/**
 * @file buffer.c
 * @brief Contourforge缓冲区管理实现
 */

#include "contourforge/rendering.h"
#include "contourforge/types.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <string.h>

/* ========== VBO（顶点缓冲对象）========== */

/**
 * @brief VBO结构
 */
typedef struct {
    unsigned int id;        /**< OpenGL缓冲ID */
    size_t size;            /**< 缓冲大小（字节） */
    unsigned int usage;     /**< 使用模式 */
} cf_vbo_t;

/**
 * @brief 创建VBO
 */
cf_result_t cf_vbo_create(size_t size, const void* data, cf_vbo_t** vbo) {
    if (size == 0 || vbo == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_vbo_t* v = (cf_vbo_t*)malloc(sizeof(cf_vbo_t));
    if (v == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    glGenBuffers(1, &v->id);
    glBindBuffer(GL_ARRAY_BUFFER, v->id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    v->size = size;
    v->usage = GL_STATIC_DRAW;
    
    *vbo = v;
    return CF_SUCCESS;
}

/**
 * @brief 创建动态VBO
 */
cf_result_t cf_vbo_create_dynamic(size_t size, cf_vbo_t** vbo) {
    if (size == 0 || vbo == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_vbo_t* v = (cf_vbo_t*)malloc(sizeof(cf_vbo_t));
    if (v == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    glGenBuffers(1, &v->id);
    glBindBuffer(GL_ARRAY_BUFFER, v->id);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    v->size = size;
    v->usage = GL_DYNAMIC_DRAW;
    
    *vbo = v;
    return CF_SUCCESS;
}

/**
 * @brief 更新VBO数据
 */
cf_result_t cf_vbo_update(cf_vbo_t* vbo, size_t offset, size_t size, const void* data) {
    if (vbo == NULL || data == NULL || offset + size > vbo->size) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return CF_SUCCESS;
}

/**
 * @brief 绑定VBO
 */
void cf_vbo_bind(const cf_vbo_t* vbo) {
    if (vbo == NULL) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
}

/**
 * @brief 销毁VBO
 */
void cf_vbo_destroy(cf_vbo_t* vbo) {
    if (vbo == NULL) {
        return;
    }
    
    glDeleteBuffers(1, &vbo->id);
    free(vbo);
}

/* ========== EBO（索引缓冲对象）========== */

/**
 * @brief EBO结构
 */
typedef struct {
    unsigned int id;        /**< OpenGL缓冲ID */
    size_t count;           /**< 索引数量 */
    size_t size;            /**< 缓冲大小（字节） */
} cf_ebo_t;

/**
 * @brief 创建EBO
 */
cf_result_t cf_ebo_create(size_t count, const unsigned int* indices, cf_ebo_t** ebo) {
    if (count == 0 || indices == NULL || ebo == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_ebo_t* e = (cf_ebo_t*)malloc(sizeof(cf_ebo_t));
    if (e == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    size_t size = count * sizeof(unsigned int);
    
    glGenBuffers(1, &e->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    e->count = count;
    e->size = size;
    
    *ebo = e;
    return CF_SUCCESS;
}

/**
 * @brief 绑定EBO
 */
void cf_ebo_bind(const cf_ebo_t* ebo) {
    if (ebo == NULL) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return;
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id);
}

/**
 * @brief 获取索引数量
 */
size_t cf_ebo_get_count(const cf_ebo_t* ebo) {
    if (ebo == NULL) {
        return 0;
    }
    
    return ebo->count;
}

/**
 * @brief 销毁EBO
 */
void cf_ebo_destroy(cf_ebo_t* ebo) {
    if (ebo == NULL) {
        return;
    }
    
    glDeleteBuffers(1, &ebo->id);
    free(ebo);
}

/* ========== VAO（顶点数组对象）========== */

/**
 * @brief VAO结构
 */
typedef struct {
    unsigned int id;        /**< OpenGL VAO ID */
    cf_vbo_t* vbo;          /**< 关联的VBO */
    cf_ebo_t* ebo;          /**< 关联的EBO（可选） */
} cf_vao_t;

/**
 * @brief 创建VAO
 */
cf_result_t cf_vao_create(cf_vao_t** vao) {
    if (vao == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_vao_t* v = (cf_vao_t*)malloc(sizeof(cf_vao_t));
    if (v == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    glGenVertexArrays(1, &v->id);
    v->vbo = NULL;
    v->ebo = NULL;
    
    *vao = v;
    return CF_SUCCESS;
}

/**
 * @brief 绑定VAO
 */
void cf_vao_bind(const cf_vao_t* vao) {
    if (vao == NULL) {
        glBindVertexArray(0);
        return;
    }
    
    glBindVertexArray(vao->id);
}

/**
 * @brief 设置顶点属性
 */
void cf_vao_set_attribute(
    cf_vao_t* vao,
    unsigned int index,
    int size,
    unsigned int type,
    bool normalized,
    int stride,
    size_t offset
) {
    if (vao == NULL) {
        return;
    }
    
    glBindVertexArray(vao->id);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(
        index,
        size,
        type,
        normalized ? GL_TRUE : GL_FALSE,
        stride,
        (void*)offset
    );
    glBindVertexArray(0);
}

/**
 * @brief 附加VBO到VAO
 */
void cf_vao_attach_vbo(cf_vao_t* vao, cf_vbo_t* vbo) {
    if (vao == NULL || vbo == NULL) {
        return;
    }
    
    vao->vbo = vbo;
}

/**
 * @brief 附加EBO到VAO
 */
void cf_vao_attach_ebo(cf_vao_t* vao, cf_ebo_t* ebo) {
    if (vao == NULL || ebo == NULL) {
        return;
    }
    
    glBindVertexArray(vao->id);
    cf_ebo_bind(ebo);
    glBindVertexArray(0);
    
    vao->ebo = ebo;
}

/**
 * @brief 销毁VAO
 */
void cf_vao_destroy(cf_vao_t* vao) {
    if (vao == NULL) {
        return;
    }
    
    glDeleteVertexArrays(1, &vao->id);
    free(vao);
}
