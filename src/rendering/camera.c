/**
 * @file camera.c
 * @brief Contourforge相机系统实现
 */

#include "contourforge/rendering.h"
#include "contourforge/types.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ========== 矩阵运算辅助函数 ========== */

/**
 * @brief 单位矩阵
 */
static void mat4_identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

/**
 * @brief 透视投影矩阵
 */
static void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov * 0.5f * (float)M_PI / 180.0f);
    
    memset(m, 0, 16 * sizeof(float));
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
}

/**
 * @brief 正交投影矩阵
 */
static void mat4_orthographic(float* m, float left, float right, float bottom, float top, float near, float far) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far - near);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

/**
 * @brief 向量归一化
 */
static void vec3_normalize(float* v) {
    float len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len > 0.0f) {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

/**
 * @brief 向量叉乘
 */
static void vec3_cross(float* result, const float* a, const float* b) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

/**
 * @brief 向量点乘
 */
static float vec3_dot(const float* a, const float* b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/**
 * @brief LookAt视图矩阵
 */
static void mat4_look_at(float* m, const float* eye, const float* target, const float* up) {
    float f[3] = {
        target[0] - eye[0],
        target[1] - eye[1],
        target[2] - eye[2]
    };
    vec3_normalize(f);
    
    float s[3];
    vec3_cross(s, f, up);
    vec3_normalize(s);
    
    float u[3];
    vec3_cross(u, s, f);
    
    memset(m, 0, 16 * sizeof(float));
    m[0] = s[0];
    m[4] = s[1];
    m[8] = s[2];
    m[1] = u[0];
    m[5] = u[1];
    m[9] = u[2];
    m[2] = -f[0];
    m[6] = -f[1];
    m[10] = -f[2];
    m[12] = -vec3_dot(s, eye);
    m[13] = -vec3_dot(u, eye);
    m[14] = vec3_dot(f, eye);
    m[15] = 1.0f;
}

/* ========== 相机结构 ========== */

/**
 * @brief 相机
 */
struct cf_camera {
    cf_camera_type_t type;      /**< 相机类型 */
    
    /* 位置和方向 */
    cf_point3_t position;       /**< 位置 */
    cf_point3_t target;         /**< 目标点 */
    cf_vec3_t up;               /**< 上方向 */
    
    /* 投影参数 */
    union {
        struct {
            float fov;          /**< 视野角度 */
            float aspect;       /**< 宽高比 */
            float near;         /**< 近裁剪面 */
            float far;          /**< 远裁剪面 */
        } perspective;
        
        struct {
            float left;         /**< 左边界 */
            float right;        /**< 右边界 */
            float bottom;       /**< 下边界 */
            float top;          /**< 上边界 */
            float near;         /**< 近裁剪面 */
            float far;          /**< 远裁剪面 */
        } orthographic;
    } projection;
    
    /* 矩阵 */
    float view_matrix[16];      /**< 视图矩阵 */
    float proj_matrix[16];      /**< 投影矩阵 */
    
    bool view_dirty;            /**< 视图矩阵是否需要更新 */
    bool proj_dirty;            /**< 投影矩阵是否需要更新 */
};

/* ========== 相机API ========== */

/**
 * @brief 创建相机
 */
cf_result_t cf_camera_create(
    cf_camera_type_t type,
    cf_camera_t** camera
) {
    if (camera == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_camera_t* cam = (cf_camera_t*)malloc(sizeof(cf_camera_t));
    if (cam == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    cam->type = type;
    
    /* 默认位置和方向 */
    cam->position.x = 0.0f;
    cam->position.y = 0.0f;
    cam->position.z = 5.0f;
    
    cam->target.x = 0.0f;
    cam->target.y = 0.0f;
    cam->target.z = 0.0f;
    
    cam->up.x = 0.0f;
    cam->up.y = 1.0f;
    cam->up.z = 0.0f;
    
    /* 默认投影参数 */
    if (type == CF_CAMERA_PERSPECTIVE) {
        cam->projection.perspective.fov = 45.0f;
        cam->projection.perspective.aspect = 16.0f / 9.0f;
        cam->projection.perspective.near = 0.1f;
        cam->projection.perspective.far = 1000.0f;
    } else {
        cam->projection.orthographic.left = -10.0f;
        cam->projection.orthographic.right = 10.0f;
        cam->projection.orthographic.bottom = -10.0f;
        cam->projection.orthographic.top = 10.0f;
        cam->projection.orthographic.near = 0.1f;
        cam->projection.orthographic.far = 1000.0f;
    }
    
    /* 初始化矩阵 */
    mat4_identity(cam->view_matrix);
    mat4_identity(cam->proj_matrix);
    
    cam->view_dirty = true;
    cam->proj_dirty = true;
    
    *camera = cam;
    return CF_SUCCESS;
}

/**
 * @brief 设置透视投影
 */
void cf_camera_set_perspective(
    cf_camera_t* camera,
    float fov,
    float aspect,
    float near,
    float far
) {
    if (camera == NULL || camera->type != CF_CAMERA_PERSPECTIVE) {
        return;
    }
    
    camera->projection.perspective.fov = fov;
    camera->projection.perspective.aspect = aspect;
    camera->projection.perspective.near = near;
    camera->projection.perspective.far = far;
    camera->proj_dirty = true;
}

/**
 * @brief 设置正交投影
 */
void cf_camera_set_orthographic(
    cf_camera_t* camera,
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far
) {
    if (camera == NULL || camera->type != CF_CAMERA_ORTHOGRAPHIC) {
        return;
    }
    
    camera->projection.orthographic.left = left;
    camera->projection.orthographic.right = right;
    camera->projection.orthographic.bottom = bottom;
    camera->projection.orthographic.top = top;
    camera->projection.orthographic.near = near;
    camera->projection.orthographic.far = far;
    camera->proj_dirty = true;
}

/**
 * @brief 设置相机位置
 */
void cf_camera_set_position(cf_camera_t* camera, cf_point3_t position) {
    if (camera == NULL) {
        return;
    }
    
    camera->position = position;
    camera->view_dirty = true;
}

/**
 * @brief 设置相机目标
 */
void cf_camera_set_target(cf_camera_t* camera, cf_point3_t target) {
    if (camera == NULL) {
        return;
    }
    
    camera->target = target;
    camera->view_dirty = true;
}

/**
 * @brief 设置上方向
 */
void cf_camera_set_up(cf_camera_t* camera, cf_vec3_t up) {
    if (camera == NULL) {
        return;
    }
    
    camera->up = up;
    camera->view_dirty = true;
}

/**
 * @brief 更新视图矩阵
 */
static void update_view_matrix(cf_camera_t* camera) {
    if (!camera->view_dirty) {
        return;
    }
    
    float eye[3] = {camera->position.x, camera->position.y, camera->position.z};
    float target[3] = {camera->target.x, camera->target.y, camera->target.z};
    float up[3] = {camera->up.x, camera->up.y, camera->up.z};
    
    mat4_look_at(camera->view_matrix, eye, target, up);
    camera->view_dirty = false;
}

/**
 * @brief 更新投影矩阵
 */
static void update_projection_matrix(cf_camera_t* camera) {
    if (!camera->proj_dirty) {
        return;
    }
    
    if (camera->type == CF_CAMERA_PERSPECTIVE) {
        mat4_perspective(
            camera->proj_matrix,
            camera->projection.perspective.fov,
            camera->projection.perspective.aspect,
            camera->projection.perspective.near,
            camera->projection.perspective.far
        );
    } else {
        mat4_orthographic(
            camera->proj_matrix,
            camera->projection.orthographic.left,
            camera->projection.orthographic.right,
            camera->projection.orthographic.bottom,
            camera->projection.orthographic.top,
            camera->projection.orthographic.near,
            camera->projection.orthographic.far
        );
    }
    
    camera->proj_dirty = false;
}

/**
 * @brief 获取视图矩阵
 */
const float* cf_camera_get_view_matrix(const cf_camera_t* camera) {
    if (camera == NULL) {
        return NULL;
    }
    
    /* 强制转换为非const以更新矩阵 */
    update_view_matrix((cf_camera_t*)camera);
    return camera->view_matrix;
}

/**
 * @brief 获取投影矩阵
 */
const float* cf_camera_get_projection_matrix(const cf_camera_t* camera) {
    if (camera == NULL) {
        return NULL;
    }
    
    /* 强制转换为非const以更新矩阵 */
    update_projection_matrix((cf_camera_t*)camera);
    return camera->proj_matrix;
}

/**
 * @brief 获取相机位置
 */
cf_point3_t cf_camera_get_position(const cf_camera_t* camera) {
    cf_point3_t pos = {0.0f, 0.0f, 0.0f};
    
    if (camera == NULL) {
        return pos;
    }
    
    return camera->position;
}

/**
 * @brief 获取相机目标
 */
cf_point3_t cf_camera_get_target(const cf_camera_t* camera) {
    cf_point3_t target = {0.0f, 0.0f, 0.0f};
    
    if (camera == NULL) {
        return target;
    }
    
    return camera->target;
}

/**
 * @brief 相机环绕目标旋转
 */
void cf_camera_orbit(cf_camera_t* camera, float delta_yaw, float delta_pitch) {
    if (camera == NULL) {
        return;
    }
    
    /* 计算当前相对位置 */
    float dx = camera->position.x - camera->target.x;
    float dy = camera->position.y - camera->target.y;
    float dz = camera->position.z - camera->target.z;
    
    float radius = sqrtf(dx * dx + dy * dy + dz * dz);
    float yaw = atan2f(dx, dz);
    float pitch = asinf(dy / radius);
    
    /* 应用旋转 */
    yaw += delta_yaw;
    pitch += delta_pitch;
    
    /* 限制pitch角度 */
    const float max_pitch = (float)M_PI * 0.49f;
    if (pitch > max_pitch) pitch = max_pitch;
    if (pitch < -max_pitch) pitch = -max_pitch;
    
    /* 计算新位置 */
    camera->position.x = camera->target.x + radius * sinf(yaw) * cosf(pitch);
    camera->position.y = camera->target.y + radius * sinf(pitch);
    camera->position.z = camera->target.z + radius * cosf(yaw) * cosf(pitch);
    
    camera->view_dirty = true;
}

/**
 * @brief 相机缩放（改变距离）
 */
void cf_camera_zoom(cf_camera_t* camera, float delta) {
    if (camera == NULL) {
        return;
    }
    
    /* 计算方向向量 */
    float dx = camera->position.x - camera->target.x;
    float dy = camera->position.y - camera->target.y;
    float dz = camera->position.z - camera->target.z;
    
    float len = sqrtf(dx * dx + dy * dy + dz * dz);
    if (len < 0.1f) return;
    
    /* 归一化 */
    dx /= len;
    dy /= len;
    dz /= len;
    
    /* 应用缩放 */
    float new_len = len * (1.0f + delta);
    if (new_len < 0.1f) new_len = 0.1f;
    if (new_len > 1000.0f) new_len = 1000.0f;
    
    camera->position.x = camera->target.x + dx * new_len;
    camera->position.y = camera->target.y + dy * new_len;
    camera->position.z = camera->target.z + dz * new_len;
    
    camera->view_dirty = true;
}

/**
 * @brief 销毁相机
 */
void cf_camera_destroy(cf_camera_t* camera) {
    if (camera == NULL) {
        return;
    }
    
    free(camera);
}
