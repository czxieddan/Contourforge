# Contourforge API 参考文档

**版本**: 1.0.0  
**日期**: 2026-04-30

本文档提供Contourforge库的完整API参考。

---

## 目录

1. [通用类型 (types.h)](#1-通用类型-typesh)
2. [核心模块 (core.h)](#2-核心模块-coreh)
3. [渲染模块 (rendering.h)](#3-渲染模块-renderingh)
4. [数据生成模块 (datagen.h)](#4-数据生成模块-datagenh)
5. [控制模块 (control.h)](#5-控制模块-controlh)

---

## 1. 通用类型 (types.h)

### 1.1 基础类型

#### cf_index_t
```c
typedef uint32_t cf_index_t;
```
索引类型，支持最多42亿个元素。

#### cf_result_t
```c
typedef enum {
    CF_SUCCESS = 0,
    CF_ERROR_INVALID_PARAM = -1,
    CF_ERROR_OUT_OF_MEMORY = -2,
    CF_ERROR_FILE_NOT_FOUND = -3,
    CF_ERROR_FILE_IO = -4,
    CF_ERROR_INVALID_FORMAT = -5,
    CF_ERROR_OPENGL = -6,
    CF_ERROR_NOT_INITIALIZED = -7,
    CF_ERROR_ALREADY_INITIALIZED = -8,
    CF_ERROR_NOT_FOUND = -9,
    CF_ERROR_UNKNOWN = -100
} cf_result_t;
```
返回码枚举。

### 1.2 数学类型

#### cf_point3_t
```c
typedef struct {
    float x, y, z;
} cf_point3_t;
```
3D点坐标。

#### cf_vec3_t
```c
typedef struct {
    float x, y, z;
} cf_vec3_t;
```
3D向量。

#### cf_bounds_t
```c
typedef struct {
    cf_point3_t min;
    cf_point3_t max;
} cf_bounds_t;
```
轴对齐边界盒（AABB）。

#### cf_color_t
```c
typedef struct {
    float r, g, b, a;
} cf_color_t;
```
RGBA颜色，范围[0.0, 1.0]。

---

## 2. 核心模块 (core.h)

### 2.1 内存管理

#### cf_memory_pool_create
```c
cf_result_t cf_memory_pool_create(
    size_t block_size,
    size_t block_count,
    cf_memory_pool_t** pool
);
```
创建内存池。

**参数**:
- `block_size`: 每个块的大小（字节）
- `block_count`: 块数量
- `pool`: 输出内存池指针

**返回**: `CF_SUCCESS`或错误码

**示例**:
```c
cf_memory_pool_t* pool;
cf_memory_pool_create(sizeof(cf_point3_t), 10000, &pool);
```

#### cf_memory_pool_alloc
```c
void* cf_memory_pool_alloc(cf_memory_pool_t* pool);
```
从内存池分配一个块。

**返回**: 内存指针，失败返回NULL

#### cf_memory_pool_free
```c
void cf_memory_pool_free(cf_memory_pool_t* pool, void* ptr);
```
释放内存块回内存池。

#### cf_memory_pool_destroy
```c
void cf_memory_pool_destroy(cf_memory_pool_t* pool);
```
销毁内存池。

### 2.2 点集

#### cf_point_set_create
```c
cf_result_t cf_point_set_create(
    size_t initial_capacity,
    cf_point_set_t** point_set
);
```
创建点集。

**参数**:
- `initial_capacity`: 初始容量
- `point_set`: 输出点集指针

#### cf_point_set_add
```c
cf_index_t cf_point_set_add(cf_point_set_t* point_set, cf_point3_t point);
```
添加点到点集。

**返回**: 点的索引

**示例**:
```c
cf_point3_t p = {1.0f, 2.0f, 3.0f};
cf_index_t idx = cf_point_set_add(point_set, p);
```

#### cf_point_set_get
```c
const cf_point3_t* cf_point_set_get(const cf_point_set_t* point_set, cf_index_t index);
```
获取点。

**返回**: 点指针，失败返回NULL

#### cf_point_set_destroy
```c
void cf_point_set_destroy(cf_point_set_t* point_set);
```
销毁点集。

### 2.3 线集

#### cf_line_set_create
```c
cf_result_t cf_line_set_create(
    cf_point_set_t* point_set,
    size_t initial_capacity,
    cf_line_set_t** line_set
);
```
创建线集。

#### cf_line_set_add
```c
cf_index_t cf_line_set_add(cf_line_set_t* line_set, cf_index_t p1, cf_index_t p2);
```
添加线段。

**参数**:
- `p1`: 起点索引
- `p2`: 终点索引

**返回**: 线段索引

#### cf_line_set_destroy
```c
void cf_line_set_destroy(cf_line_set_t* line_set);
```
销毁线集。

### 2.4 模型

#### cf_model_create
```c
cf_result_t cf_model_create(const char* name, cf_model_t** model);
```
创建模型。

**示例**:
```c
cf_model_t* model;
cf_model_create("Terrain", &model);
```

#### cf_model_update_bounds
```c
void cf_model_update_bounds(cf_model_t* model);
```
更新模型边界盒。

#### cf_model_destroy
```c
void cf_model_destroy(cf_model_t* model);
```
销毁模型。

### 2.5 八叉树

#### cf_octree_create
```c
cf_result_t cf_octree_create(
    const cf_point_set_t* point_set,
    const cf_bounds_t* bounds,
    size_t max_points_per_node,
    cf_octree_t** octree
);
```
创建八叉树。

**参数**:
- `point_set`: 点集
- `bounds`: 边界盒
- `max_points_per_node`: 每个节点最大点数
- `octree`: 输出八叉树指针

#### cf_octree_query_frustum
```c
cf_result_t cf_octree_query_frustum(
    const cf_octree_t* octree,
    const float* view_proj_matrix,
    cf_index_t** indices,
    size_t* count
);
```
视锥剔除查询。

**参数**:
- `view_proj_matrix`: 视图投影矩阵（4x4）
- `indices`: 输出可见点索引数组
- `count`: 输出点数量

#### cf_octree_destroy
```c
void cf_octree_destroy(cf_octree_t* octree);
```
销毁八叉树。

---

## 3. 渲染模块 (rendering.h)

### 3.1 渲染器

#### cf_renderer_init
```c
cf_result_t cf_renderer_init(
    const cf_renderer_config_t* config,
    cf_renderer_t** renderer
);
```
初始化渲染器。

**参数**:
```c
typedef struct {
    int width;
    int height;
    const char* title;
    bool vsync;
    int msaa_samples;
    cf_color_t clear_color;
} cf_renderer_config_t;
```

**示例**:
```c
cf_renderer_config_t config = {
    .width = 1280,
    .height = 720,
    .title = "Viewer",
    .vsync = true,
    .msaa_samples = 4,
    .clear_color = {0.1f, 0.1f, 0.1f, 1.0f}
};
cf_renderer_t* renderer;
cf_renderer_init(&config, &renderer);
```

#### cf_renderer_set_model
```c
cf_result_t cf_renderer_set_model(cf_renderer_t* renderer, cf_model_t* model);
```
设置要渲染的模型。

#### cf_renderer_begin_frame
```c
cf_result_t cf_renderer_begin_frame(cf_renderer_t* renderer);
```
开始渲染帧。

#### cf_renderer_render
```c
cf_result_t cf_renderer_render(cf_renderer_t* renderer);
```
执行渲染。

#### cf_renderer_end_frame
```c
cf_result_t cf_renderer_end_frame(cf_renderer_t* renderer);
```
结束渲染帧。

#### cf_renderer_should_close
```c
bool cf_renderer_should_close(const cf_renderer_t* renderer);
```
检查窗口是否应该关闭。

**示例（渲染循环）**:
```c
while (!cf_renderer_should_close(renderer)) {
    cf_renderer_begin_frame(renderer);
    cf_renderer_render(renderer);
    cf_renderer_end_frame(renderer);
}
```

#### cf_renderer_get_camera
```c
cf_camera_t* cf_renderer_get_camera(cf_renderer_t* renderer);
```
获取渲染器的相机。

#### cf_renderer_destroy
```c
void cf_renderer_destroy(cf_renderer_t* renderer);
```
销毁渲染器。

### 3.2 相机

#### cf_camera_create
```c
cf_result_t cf_camera_create(
    cf_camera_type_t type,
    cf_camera_t** camera
);
```
创建相机。

**类型**:
```c
typedef enum {
    CF_CAMERA_PERSPECTIVE,
    CF_CAMERA_ORTHOGRAPHIC
} cf_camera_type_t;
```

#### cf_camera_set_perspective
```c
void cf_camera_set_perspective(
    cf_camera_t* camera,
    float fov,
    float aspect,
    float near,
    float far
);
```
设置透视投影。

**参数**:
- `fov`: 视野角度（度）
- `aspect`: 宽高比
- `near`: 近裁剪面
- `far`: 远裁剪面

**示例**:
```c
cf_camera_set_perspective(camera, 45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
```

#### cf_camera_set_position
```c
void cf_camera_set_position(cf_camera_t* camera, cf_point3_t position);
```
设置相机位置。

#### cf_camera_set_target
```c
void cf_camera_set_target(cf_camera_t* camera, cf_point3_t target);
```
设置相机目标点。

#### cf_camera_orbit
```c
void cf_camera_orbit(cf_camera_t* camera, float yaw, float pitch);
```
轨道旋转相机。

#### cf_camera_zoom
```c
void cf_camera_zoom(cf_camera_t* camera, float delta);
```
缩放相机。

---

## 4. 数据生成模块 (datagen.h)

### 4.1 高度图

#### cf_heightmap_load
```c
cf_result_t cf_heightmap_load(
    const char* filepath,
    cf_heightmap_t** heightmap
);
```
从灰度图加载高度图。

**支持格式**: PNG, JPG, BMP, TGA

**示例**:
```c
cf_heightmap_t* heightmap;
cf_heightmap_load("terrain.png", &heightmap);
```

#### cf_heightmap_sample
```c
float cf_heightmap_sample(
    const cf_heightmap_t* heightmap,
    float x,
    float y
);
```
采样高度值。

**参数**:
- `x`, `y`: 归一化坐标[0.0, 1.0]

**返回**: 高度值

#### cf_heightmap_destroy
```c
void cf_heightmap_destroy(cf_heightmap_t* heightmap);
```
销毁高度图。

### 4.2 等高线生成

#### cf_contour_generate
```c
cf_result_t cf_contour_generate(
    const cf_heightmap_t* heightmap,
    const cf_contour_config_t* config,
    cf_model_t** model
);
```
从高度图生成等高线模型。

**配置**:
```c
typedef struct {
    float interval;             // 等高线间隔
    float min_height;           // 最小高度
    float max_height;           // 最大高度
    float simplify_tolerance;   // 简化容差（0=不简化）
    bool build_topology;        // 是否构建拓扑
} cf_contour_config_t;
```

**示例**:
```c
cf_contour_config_t config = {
    .interval = 10.0f,
    .min_height = 0.0f,
    .max_height = 1000.0f,
    .simplify_tolerance = 0.5f,
    .build_topology = true
};
cf_model_t* model;
cf_contour_generate(heightmap, &config, &model);
```

### 4.3 线段简化

#### cf_simplify_douglas_peucker
```c
cf_result_t cf_simplify_douglas_peucker(
    const cf_point3_t* points,
    size_t count,
    float tolerance,
    cf_point3_t** out_points,
    size_t* out_count
);
```
Douglas-Peucker算法简化线段。

**参数**:
- `points`: 输入点数组
- `count`: 点数量
- `tolerance`: 容差（越大简化越多）
- `out_points`: 输出简化后的点数组
- `out_count`: 输出点数量

**示例**:
```c
cf_point3_t* simplified;
size_t simplified_count;
cf_simplify_douglas_peucker(points, count, 1.0f, &simplified, &simplified_count);
```

### 4.4 拓扑构建

#### cf_topology_build
```c
cf_result_t cf_topology_build(cf_model_t* model);
```
构建模型的拓扑关系。

#### cf_topology_find_neighbors
```c
cf_result_t cf_topology_find_neighbors(
    const cf_model_t* model,
    cf_index_t line_index,
    cf_index_t** neighbors,
    size_t* count
);
```
查找线段的相邻线段。

---

## 5. 控制模块 (control.h)

### 5.1 输入处理

#### cf_input_update
```c
void cf_input_update(cf_renderer_t* renderer, cf_input_state_t* input);
```
更新输入状态。

**输入状态**:
```c
typedef struct {
    double mouse_x;
    double mouse_y;
    double mouse_delta_x;
    double mouse_delta_y;
    bool mouse_buttons[8];
    bool keys[512];
} cf_input_state_t;
```

### 5.2 节点选择

#### cf_selector_create
```c
cf_result_t cf_selector_create(
    cf_model_t* model,
    cf_renderer_t* renderer,
    cf_selector_t** selector
);
```
创建选择器。

#### cf_selector_pick_point
```c
cf_result_t cf_selector_pick_point(
    cf_selector_t* selector,
    double screen_x,
    double screen_y,
    float radius,
    cf_index_t* out_index
);
```
选择点（射线投射）。

**参数**:
- `screen_x`, `screen_y`: 屏幕坐标
- `radius`: 选择半径（世界空间）
- `out_index`: 输出点索引

**返回**: `CF_SUCCESS`或`CF_ERROR_NOT_FOUND`

**示例**:
```c
cf_index_t selected;
if (cf_selector_pick_point(selector, mouse_x, mouse_y, 5.0f, &selected) == CF_SUCCESS) {
    printf("选中点: %u\n", selected);
}
```

#### cf_selector_box_select
```c
cf_result_t cf_selector_box_select(
    cf_selector_t* selector,
    double x1, double y1,
    double x2, double y2,
    cf_index_t** indices,
    size_t* count
);
```
框选点。

#### cf_selector_destroy
```c
void cf_selector_destroy(cf_selector_t* selector);
```
销毁选择器。

### 5.3 编辑器

#### cf_editor_create
```c
cf_result_t cf_editor_create(
    cf_model_t* model,
    size_t max_undo_levels,
    cf_editor_t** editor
);
```
创建编辑器。

**参数**:
- `max_undo_levels`: 最大撤销层数

#### cf_editor_move_point
```c
cf_result_t cf_editor_move_point(
    cf_editor_t* editor,
    cf_index_t point_index,
    cf_point3_t new_position
);
```
移动点。

#### cf_editor_delete_point
```c
cf_result_t cf_editor_delete_point(
    cf_editor_t* editor,
    cf_index_t point_index
);
```
删除点。

#### cf_editor_insert_point_on_line
```c
cf_result_t cf_editor_insert_point_on_line(
    cf_editor_t* editor,
    cf_index_t line_index,
    float t,
    cf_index_t* out_point_index
);
```
在线段上插入点。

**参数**:
- `line_index`: 线段索引
- `t`: 插入位置[0.0, 1.0]
- `out_point_index`: 输出新点索引

#### cf_editor_undo
```c
cf_result_t cf_editor_undo(cf_editor_t* editor);
```
撤销操作。

#### cf_editor_redo
```c
cf_result_t cf_editor_redo(cf_editor_t* editor);
```
重做操作。

#### cf_editor_can_undo
```c
bool cf_editor_can_undo(const cf_editor_t* editor);
```
检查是否可以撤销。

#### cf_editor_can_redo
```c
bool cf_editor_can_redo(const cf_editor_t* editor);
```
检查是否可以重做。

#### cf_editor_has_unsaved_changes
```c
bool cf_editor_has_unsaved_changes(const cf_editor_t* editor);
```
检查是否有未保存的更改。

#### cf_editor_destroy
```c
void cf_editor_destroy(cf_editor_t* editor);
```
销毁编辑器。

---

## 完整示例

### 基础渲染流程

```c
#include <contourforge/contourforge.h>

int main() {
    // 1. 加载高度图
    cf_heightmap_t* heightmap;
    cf_heightmap_load("terrain.png", &heightmap);
    
    // 2. 生成等高线
    cf_contour_config_t config = {
        .interval = 10.0f,
        .min_height = 0.0f,
        .max_height = 1000.0f,
        .simplify_tolerance = 0.5f,
        .build_topology = true
    };
    cf_model_t* model;
    cf_contour_generate(heightmap, &config, &model);
    
    // 3. 初始化渲染器
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.1f, 1.0f}
    };
    cf_renderer_t* renderer;
    cf_renderer_init(&renderer_config, &renderer);
    cf_renderer_set_model(renderer, model);
    
    // 4. 渲染循环
    while (!cf_renderer_should_close(renderer)) {
        cf_renderer_begin_frame(renderer);
        cf_renderer_render(renderer);
        cf_renderer_end_frame(renderer);
    }
    
    // 5. 清理
    cf_renderer_destroy(renderer);
    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
    
    return 0;
}
```

---

## 错误处理

所有返回`cf_result_t`的函数都应检查返回值：

```c
cf_result_t result = cf_model_create("Model", &model);
if (result != CF_SUCCESS) {
    fprintf(stderr, "错误: 无法创建模型 (错误码: %d)\n", result);
    return result;
}
```

或使用宏：

```c
if (CF_FAILED(cf_model_create("Model", &model))) {
    // 处理错误
}
```

---

## 线程安全

默认情况下，Contourforge的API **不是线程安全的**。如果需要在多线程环境中使用，请：

1. 为每个线程创建独立的对象
2. 使用外部同步机制（互斥锁）
3. 仅在主线程调用OpenGL相关函数

---

## 性能建议

1. **使用内存池**: 对于频繁分配的小对象
2. **启用拓扑构建**: 加速邻接查询
3. **调整简化容差**: 平衡质量和性能
4. **使用八叉树**: 大规模数据必须使用
5. **批量操作**: 避免逐个修改节点

---

## 更多信息

- [用户指南](USER_GUIDE.md) - 详细使用教程
- [架构文档](../ARCHITECTURE.md) - 系统设计
- [示例代码](../examples/) - 完整示例程序
- [性能优化](PERFORMANCE.md) - 性能分析

---

**文档版本**: 1.0.0  
**最后更新**: 2026-04-30
