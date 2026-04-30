/**
 * @file renderer.c
 * @brief Contourforge渲染器实现
 */

#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "contourforge/rendering.h"
#include "contourforge/core.h"
#include "contourforge/types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 前向声明 */
typedef struct cf_mesh cf_mesh_t;
cf_result_t cf_mesh_create_from_points(cf_model_t* model, cf_mesh_t** mesh);
cf_result_t cf_mesh_create_from_lines(cf_model_t* model, cf_mesh_t** mesh);
void cf_mesh_draw_points(const cf_mesh_t* mesh);
void cf_mesh_draw_lines(const cf_mesh_t* mesh);
void cf_mesh_destroy(cf_mesh_t* mesh);

/* ========== 渲染器结构 ========== */

/**
 * @brief 渲染器
 */
struct cf_renderer {
    GLFWwindow* window;         /**< GLFW窗口 */
    cf_camera_t* camera;        /**< 相机 */
    cf_shader_t* shader;        /**< 着色器 */
    cf_model_t* model;          /**< 当前模型 */
    
    int width;                  /**< 窗口宽度 */
    int height;                 /**< 窗口高度 */
    
    cf_color_t clear_color;     /**< 清屏颜色 */
    
    bool initialized;           /**< 是否已初始化 */
};

/* ========== GLFW回调 ========== */

/**
 * @brief 错误回调
 */
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

/**
 * @brief 窗口大小改变回调
 */
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    
    /* 更新渲染器尺寸 */
    cf_renderer_t* renderer = (cf_renderer_t*)glfwGetWindowUserPointer(window);
    if (renderer != NULL) {
        renderer->width = width;
        renderer->height = height;
        
        /* 更新相机宽高比 */
        if (renderer->camera != NULL) {
            float aspect = (float)width / (float)height;
            cf_camera_set_perspective(renderer->camera, 45.0f, aspect, 0.1f, 1000.0f);
        }
    }
}

/* ========== 渲染器API ========== */

/**
 * @brief 初始化渲染器
 */
cf_result_t cf_renderer_init(
    const cf_renderer_config_t* config,
    cf_renderer_t** renderer
) {
    if (config == NULL || renderer == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 分配渲染器 */
    cf_renderer_t* r = (cf_renderer_t*)malloc(sizeof(cf_renderer_t));
    if (r == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    memset(r, 0, sizeof(cf_renderer_t));
    
    r->width = config->width;
    r->height = config->height;
    r->clear_color = config->clear_color;
    
    /* 初始化GLFW */
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit()) {
        free(r);
        return CF_ERROR_OPENGL;
    }
    
    /* 设置OpenGL版本 */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    /* MSAA */
    if (config->msaa_samples > 0) {
        glfwWindowHint(GLFW_SAMPLES, config->msaa_samples);
    }
    
    /* 创建窗口 */
    r->window = glfwCreateWindow(
        config->width,
        config->height,
        config->title,
        NULL,
        NULL
    );
    
    if (r->window == NULL) {
        glfwTerminate();
        free(r);
        return CF_ERROR_OPENGL;
    }
    
    glfwMakeContextCurrent(r->window);
    
    /* 设置VSync */
    glfwSwapInterval(config->vsync ? 1 : 0);
    
    /* 设置回调 */
    glfwSetWindowUserPointer(r->window, r);
    glfwSetFramebufferSizeCallback(r->window, framebuffer_size_callback);
    
    /* 初始化glad */
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        glfwDestroyWindow(r->window);
        glfwTerminate();
        free(r);
        return CF_ERROR_OPENGL;
    }
    
    /* 设置OpenGL状态 */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    if (config->msaa_samples > 0) {
        glEnable(GL_MULTISAMPLE);
    }
    
    /* 创建默认相机 */
    cf_result_t result = cf_camera_create(CF_CAMERA_PERSPECTIVE, &r->camera);
    if (CF_FAILED(result)) {
        glfwDestroyWindow(r->window);
        glfwTerminate();
        free(r);
        return result;
    }
    
    float aspect = (float)config->width / (float)config->height;
    cf_camera_set_perspective(r->camera, 45.0f, aspect, 0.1f, 1000.0f);
    
    /* 设置默认相机位置 */
    cf_point3_t pos = {0.0f, 0.0f, 10.0f};
    cf_point3_t target = {0.0f, 0.0f, 0.0f};
    cf_camera_set_position(r->camera, pos);
    cf_camera_set_target(r->camera, target);
    
    r->initialized = true;
    
    *renderer = r;
    return CF_SUCCESS;
}

/**
 * @brief 设置着色器
 */
cf_result_t cf_renderer_set_shader(cf_renderer_t* renderer, cf_shader_t* shader) {
    if (renderer == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    renderer->shader = shader;
    return CF_SUCCESS;
}

/**
 * @brief 设置模型
 */
cf_result_t cf_renderer_set_model(cf_renderer_t* renderer, cf_model_t* model) {
    if (renderer == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    renderer->model = model;
    return CF_SUCCESS;
}

/**
 * @brief 获取相机
 */
cf_camera_t* cf_renderer_get_camera(cf_renderer_t* renderer) {
    if (renderer == NULL) {
        return NULL;
    }
    
    return renderer->camera;
}

/**
 * @brief 开始帧
 */
cf_result_t cf_renderer_begin_frame(cf_renderer_t* renderer) {
    if (renderer == NULL || !renderer->initialized) {
        return CF_ERROR_NOT_INITIALIZED;
    }
    
    /* 清屏 */
    glClearColor(
        renderer->clear_color.r,
        renderer->clear_color.g,
        renderer->clear_color.b,
        renderer->clear_color.a
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    return CF_SUCCESS;
}

/**
 * @brief 渲染
 */
cf_result_t cf_renderer_render(cf_renderer_t* renderer) {
    if (renderer == NULL || !renderer->initialized) {
        return CF_ERROR_NOT_INITIALIZED;
    }
    
    if (renderer->model == NULL || renderer->shader == NULL) {
        return CF_SUCCESS;  /* 没有内容可渲染 */
    }
    
    /* 使用着色器 */
    cf_shader_use(renderer->shader);
    
    /* 设置矩阵 */
    const float* view = cf_camera_get_view_matrix(renderer->camera);
    const float* proj = cf_camera_get_projection_matrix(renderer->camera);
    
    cf_shader_set_mat4(renderer->shader, "view", view);
    cf_shader_set_mat4(renderer->shader, "projection", proj);
    
    /* 创建临时网格并渲染 */
    cf_mesh_t* mesh = NULL;
    
    if (renderer->model->lines != NULL && renderer->model->lines->count > 0) {
        /* 渲染线 */
        if (cf_mesh_create_from_lines(renderer->model, &mesh) == CF_SUCCESS) {
            glLineWidth(2.0f);
            cf_mesh_draw_lines(mesh);
            cf_mesh_destroy(mesh);
        }
    } else if (renderer->model->points != NULL && renderer->model->points->count > 0) {
        /* 渲染点 */
        if (cf_mesh_create_from_points(renderer->model, &mesh) == CF_SUCCESS) {
            glPointSize(3.0f);
            cf_mesh_draw_points(mesh);
            cf_mesh_destroy(mesh);
        }
    }
    
    return CF_SUCCESS;
}

/**
 * @brief 结束帧
 */
cf_result_t cf_renderer_end_frame(cf_renderer_t* renderer) {
    if (renderer == NULL || !renderer->initialized) {
        return CF_ERROR_NOT_INITIALIZED;
    }
    
    /* 交换缓冲区 */
    glfwSwapBuffers(renderer->window);
    
    /* 处理事件 */
    glfwPollEvents();
    
    return CF_SUCCESS;
}

/**
 * @brief 检查窗口是否应该关闭
 */
bool cf_renderer_should_close(const cf_renderer_t* renderer) {
    if (renderer == NULL || !renderer->initialized) {
        return true;
    }
    
    return glfwWindowShouldClose(renderer->window);
}

/**
 * @brief 设置清屏颜色
 */
void cf_renderer_set_clear_color(cf_renderer_t* renderer, cf_color_t color) {
    if (renderer == NULL) {
        return;
    }
    
    renderer->clear_color = color;
}

/**
 * @brief 获取窗口尺寸
 */
void cf_renderer_get_size(const cf_renderer_t* renderer, int* width, int* height) {
    if (renderer == NULL) {
        return;
    }
    
    if (width != NULL) {
        *width = renderer->width;
    }
    
    if (height != NULL) {
        *height = renderer->height;
    }
}

/**
 * @brief 获取GLFW窗口
 */
GLFWwindow* cf_renderer_get_window(cf_renderer_t* renderer) {
    if (renderer == NULL) {
        return NULL;
    }
    
    return renderer->window;
}

/**
 * @brief 截图
 */
cf_result_t cf_renderer_screenshot(
    const cf_renderer_t* renderer,
    const char* filename
) {
    if (renderer == NULL || filename == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* TODO: 实现截图功能 */
    /* 需要使用stb_image_write */
    
    return CF_ERROR_NOT_FOUND;
}

/**
 * @brief 销毁渲染器
 */
void cf_renderer_destroy(cf_renderer_t* renderer) {
    if (renderer == NULL) {
        return;
    }
    
    if (renderer->camera != NULL) {
        cf_camera_destroy(renderer->camera);
    }
    
    if (renderer->window != NULL) {
        glfwDestroyWindow(renderer->window);
    }
    
    if (renderer->initialized) {
        glfwTerminate();
    }
    
    free(renderer);
}
