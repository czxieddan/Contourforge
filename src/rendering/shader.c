/**
 * @file shader.c
 * @brief Contourforge着色器管理实现
 */

#include "contourforge/rendering.h"
#include "contourforge/types.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ========== 着色器结构 ========== */

/**
 * @brief 着色器
 */
struct cf_shader {
    unsigned int program_id;    /**< OpenGL程序ID */
    unsigned int vertex_id;     /**< 顶点着色器ID */
    unsigned int fragment_id;   /**< 片段着色器ID */
};

/* ========== 辅助函数 ========== */

/**
 * @brief 读取文件内容
 */
static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return NULL;
    }
    
    /* 获取文件大小 */
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    /* 分配缓冲区 */
    char* buffer = (char*)malloc(size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    
    /* 读取文件 */
    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    return buffer;
}

/**
 * @brief 编译着色器
 */
static unsigned int compile_shader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    /* 检查编译错误 */
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Shader compilation failed:\n%s\n", info_log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

/**
 * @brief 链接着色器程序
 */
static unsigned int link_program(unsigned int vertex, unsigned int fragment) {
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    
    /* 检查链接错误 */
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Shader linking failed:\n%s\n", info_log);
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

/* ========== 着色器API ========== */

/**
 * @brief 从文件加载着色器
 */
cf_result_t cf_shader_load(
    const char* vertex_path,
    const char* fragment_path,
    cf_shader_t** shader
) {
    if (vertex_path == NULL || fragment_path == NULL || shader == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 读取着色器源码 */
    char* vertex_source = read_file(vertex_path);
    if (vertex_source == NULL) {
        return CF_ERROR_FILE_NOT_FOUND;
    }
    
    char* fragment_source = read_file(fragment_path);
    if (fragment_source == NULL) {
        free(vertex_source);
        return CF_ERROR_FILE_NOT_FOUND;
    }
    
    /* 编译着色器 */
    unsigned int vertex_id = compile_shader(GL_VERTEX_SHADER, vertex_source);
    free(vertex_source);
    
    if (vertex_id == 0) {
        free(fragment_source);
        return CF_ERROR_OPENGL;
    }
    
    unsigned int fragment_id = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    free(fragment_source);
    
    if (fragment_id == 0) {
        glDeleteShader(vertex_id);
        return CF_ERROR_OPENGL;
    }
    
    /* 链接程序 */
    unsigned int program_id = link_program(vertex_id, fragment_id);
    if (program_id == 0) {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return CF_ERROR_OPENGL;
    }
    
    /* 创建着色器对象 */
    cf_shader_t* s = (cf_shader_t*)malloc(sizeof(cf_shader_t));
    if (s == NULL) {
        glDeleteProgram(program_id);
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    s->program_id = program_id;
    s->vertex_id = vertex_id;
    s->fragment_id = fragment_id;
    
    *shader = s;
    return CF_SUCCESS;
}

/**
 * @brief 从字符串创建着色器
 */
cf_result_t cf_shader_create_from_source(
    const char* vertex_source,
    const char* fragment_source,
    cf_shader_t** shader
) {
    if (vertex_source == NULL || fragment_source == NULL || shader == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 编译着色器 */
    unsigned int vertex_id = compile_shader(GL_VERTEX_SHADER, vertex_source);
    if (vertex_id == 0) {
        return CF_ERROR_OPENGL;
    }
    
    unsigned int fragment_id = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    if (fragment_id == 0) {
        glDeleteShader(vertex_id);
        return CF_ERROR_OPENGL;
    }
    
    /* 链接程序 */
    unsigned int program_id = link_program(vertex_id, fragment_id);
    if (program_id == 0) {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return CF_ERROR_OPENGL;
    }
    
    /* 创建着色器对象 */
    cf_shader_t* s = (cf_shader_t*)malloc(sizeof(cf_shader_t));
    if (s == NULL) {
        glDeleteProgram(program_id);
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    s->program_id = program_id;
    s->vertex_id = vertex_id;
    s->fragment_id = fragment_id;
    
    *shader = s;
    return CF_SUCCESS;
}

/**
 * @brief 使用着色器
 */
void cf_shader_use(cf_shader_t* shader) {
    if (shader == NULL) {
        return;
    }
    
    glUseProgram(shader->program_id);
}

/**
 * @brief 设置uniform（整数）
 */
void cf_shader_set_int(cf_shader_t* shader, const char* name, int value) {
    if (shader == NULL || name == NULL) {
        return;
    }
    
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

/**
 * @brief 设置uniform（浮点数）
 */
void cf_shader_set_float(cf_shader_t* shader, const char* name, float value) {
    if (shader == NULL || name == NULL) {
        return;
    }
    
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

/**
 * @brief 设置uniform（vec2）
 */
void cf_shader_set_vec2(cf_shader_t* shader, const char* name, float x, float y) {
    if (shader == NULL || name == NULL) {
        return;
    }
    
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform2f(location, x, y);
    }
}

/**
 * @brief 设置uniform（vec3）
 */
void cf_shader_set_vec3(cf_shader_t* shader, const char* name, float x, float y, float z) {
    if (shader == NULL || name == NULL) {
        return;
    }
    
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

/**
 * @brief 设置uniform（vec4）
 */
void cf_shader_set_vec4(cf_shader_t* shader, const char* name, float x, float y, float z, float w) {
    if (shader == NULL || name == NULL) {
        return;
    }
    
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform4f(location, x, y, z, w);
    }
}

/**
 * @brief 设置uniform（矩阵4x4）
 */
void cf_shader_set_mat4(cf_shader_t* shader, const char* name, const float* matrix) {
    if (shader == NULL || name == NULL || matrix == NULL) {
        return;
    }
    
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

/**
 * @brief 获取程序ID
 */
unsigned int cf_shader_get_program_id(const cf_shader_t* shader) {
    if (shader == NULL) {
        return 0;
    }
    
    return shader->program_id;
}

/**
 * @brief 销毁着色器
 */
void cf_shader_destroy(cf_shader_t* shader) {
    if (shader == NULL) {
        return;
    }
    
    glDeleteProgram(shader->program_id);
    glDeleteShader(shader->vertex_id);
    glDeleteShader(shader->fragment_id);
    
    free(shader);
}
