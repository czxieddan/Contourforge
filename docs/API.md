# Contourforge API 参考文档

**版本**: 0.3.0
**日期**: 2026-05-01

本文档提供Contourforge v0.3.0公共API参考。主入口头文件为：

```c
#include <contourforge/contourforge.h>
```

---

## 目录

1. [通用类型](#1-通用类型)
2. [核心模块](#2-核心模块)
3. [LOD系统](#3-lod系统)
4. [线程池模块](#4-线程池模块)
5. [数据生成与格式支持](#5-数据生成与格式支持)
6. [渲染与相机](#6-渲染与相机)
7. [标注系统](#7-标注系统)
8. [控制与编辑](#8-控制与编辑)
9. [完整示例](#9-完整示例)
10. [线程安全和最佳实践](#10-线程安全和最佳实践)

---

## 1. 通用类型

### `cf_result_t`

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

### 数学和颜色类型

```c
typedef uint32_t cf_index_t;

typedef struct { float x, y; } cf_point2_t;
typedef struct { float x, y, z; } cf_point3_t;
typedef struct { float x, y; } cf_vec2_t;
typedef struct { float x, y, z; } cf_vec3_t;
typedef struct { float x, y, z, w; } cf_vec4_t;

typedef struct {
    cf_point3_t min;
    cf_point3_t max;
} cf_bounds_t;

typedef struct {
    float r, g, b, a;
} cf_color_t;
```

---

## 2. 核心模块

### 内存池

```c
cf_result_t cf_memory_pool_create(size_t block_size, size_t block_count, cf_memory_pool_t** pool);
void* cf_memory_pool_alloc(cf_memory_pool_t* pool);
void cf_memory_pool_free(cf_memory_pool_t* pool, void* ptr);
void cf_memory_pool_destroy(cf_memory_pool_t* pool);
```

### 点集

```c
cf_result_t cf_point_set_create(size_t initial_capacity, cf_point_set_t** point_set);
cf_index_t cf_point_set_add(cf_point_set_t* point_set, cf_point3_t point);
const cf_point3_t* cf_point_set_get(const cf_point_set_t* point_set, cf_index_t index);
void cf_point_set_update_bounds(cf_point_set_t* point_set);
void cf_point_set_destroy(cf_point_set_t* point_set);
```

### 线集

```c
cf_result_t cf_line_set_create(cf_point_set_t* point_set, size_t initial_capacity, cf_line_set_t** line_set);
cf_index_t cf_line_set_add(cf_line_set_t* line_set, cf_index_t p1, cf_index_t p2);
void cf_line_set_destroy(cf_line_set_t* line_set);
```

### 模型

```c
cf_result_t cf_model_create(const char* name, cf_model_t** model);
void cf_model_update_bounds(cf_model_t* model);
cf_point3_t cf_model_get_center(const cf_model_t* model);
cf_vec3_t cf_model_get_size(const cf_model_t* model);
void cf_model_destroy(cf_model_t* model);
```

### 八叉树

```c
cf_result_t cf_octree_create(cf_bounds_t bounds, size_t max_depth, size_t max_points, cf_octree_t** octree);
cf_result_t cf_octree_insert(cf_octree_t* octree, cf_point3_t point, cf_index_t index);
cf_result_t cf_octree_query(const cf_octree_t* octree, cf_bounds_t bounds, cf_index_t** indices, size_t* count);
void cf_octree_destroy(cf_octree_t* octree);
```

---

## 3. LOD系统

### 数据结构

```c
typedef struct {
    size_t level_count;
    float* distance_thresholds;
    float* simplification_ratios;
    bool preserve_boundaries;
    bool use_importance_sampling;
} cf_lod_config_t;

typedef struct {
    size_t original_point_count;
    size_t original_line_count;
    size_t* level_point_counts;
    size_t* level_line_counts;
    float* reduction_ratios;
    size_t total_memory_bytes;
} cf_lod_stats_t;
```

### API

```c
cf_result_t cf_lod_create(cf_model_t* base_model, const cf_lod_config_t* config, cf_lod_model_t** lod_model);
cf_result_t cf_lod_create_parallel(cf_model_t* base_model, const cf_lod_config_t* config, cf_thread_pool_t* thread_pool, cf_lod_model_t** lod_model);
int cf_lod_select_level(const cf_lod_model_t* lod_model, float distance);
cf_result_t cf_lod_set_level(cf_lod_model_t* lod_model, int level);
int cf_lod_get_current_level(const cf_lod_model_t* lod_model);
cf_result_t cf_lod_get_stats(const cf_lod_model_t* lod_model, cf_lod_stats_t* stats);
void cf_lod_stats_destroy(cf_lod_stats_t* stats);
void cf_lod_destroy(cf_lod_model_t* lod_model);
```

### 示例

```c
float distances[] = {50.0f, 150.0f, 300.0f};
float ratios[] = {1.0f, 0.5f, 0.25f};
cf_lod_config_t config = {
    .level_count = 3,
    .distance_thresholds = distances,
    .simplification_ratios = ratios,
    .preserve_boundaries = true,
    .use_importance_sampling = true
};

cf_lod_model_t* lod = NULL;
cf_lod_create(model, &config, &lod);
int level = cf_lod_select_level(lod, 120.0f);
cf_lod_set_level(lod, level);
```

---

## 4. 线程池模块

### 线程配置

```c
typedef struct {
    int num_threads;
    bool enable_threading;
    int task_queue_size;
} cf_thread_config_t;

cf_thread_config_t cf_thread_config_default(void);
```

### 线程池

```c
typedef void (*cf_task_func_t)(void* arg);

cf_result_t cf_thread_pool_create(const cf_thread_config_t* config, cf_thread_pool_t** pool);
cf_result_t cf_thread_pool_submit(cf_thread_pool_t* pool, cf_task_func_t func, void* arg);
void cf_thread_pool_wait(cf_thread_pool_t* pool);
void cf_thread_pool_get_stats(const cf_thread_pool_t* pool, int* active_threads, int* pending_tasks);
void cf_thread_pool_destroy(cf_thread_pool_t* pool);
```

### 互斥锁和原子操作

```c
cf_result_t cf_mutex_create(cf_mutex_t** mutex);
void cf_mutex_lock(cf_mutex_t* mutex);
void cf_mutex_unlock(cf_mutex_t* mutex);
void cf_mutex_destroy(cf_mutex_t* mutex);

int cf_atomic_add(cf_atomic_int_t* atomic, int value);
bool cf_atomic_compare_exchange(cf_atomic_int_t* atomic, int expected, int desired);
```

### 示例

```c
static void task(void* arg) {
    cf_atomic_int_t* counter = (cf_atomic_int_t*)arg;
    cf_atomic_add(counter, 1);
}

cf_thread_config_t config = cf_thread_config_default();
config.num_threads = 4;

cf_thread_pool_t* pool = NULL;
cf_thread_pool_create(&config, &pool);

cf_atomic_int_t counter = {0};
cf_thread_pool_submit(pool, task, &counter);
cf_thread_pool_wait(pool);
cf_thread_pool_destroy(pool);
```

---

## 5. 数据生成与格式支持

### 格式枚举

```c
typedef enum {
    CF_FORMAT_UNKNOWN = 0,
    CF_FORMAT_PNG,
    CF_FORMAT_JPEG,
    CF_FORMAT_BMP,
    CF_FORMAT_TIFF,
    CF_FORMAT_GEOTIFF,
    CF_FORMAT_RAW
} cf_heightmap_format_t;

typedef enum {
    CF_RAW_FORMAT_U8 = 0,
    CF_RAW_FORMAT_U16,
    CF_RAW_FORMAT_I16,
    CF_RAW_FORMAT_U32,
    CF_RAW_FORMAT_I32,
    CF_RAW_FORMAT_F32
} cf_raw_format_t;
```

### 高度图

```c
typedef struct {
    float* data;
    int width;
    int height;
    float min_height;
    float max_height;
} cf_heightmap_t;
```

### 格式支持API（v0.3.0）

```c
cf_heightmap_format_t cf_heightmap_detect_format(const char* filename);
const char* cf_heightmap_format_name(cf_heightmap_format_t format);
cf_result_t cf_heightmap_load(const char* filepath, cf_heightmap_t** heightmap);
cf_result_t cf_heightmap_load_tiff(const char* filepath, cf_heightmap_t** heightmap);
cf_result_t cf_heightmap_load_raw(const char* filepath, int width, int height, cf_raw_format_t format, cf_heightmap_t** heightmap);
float cf_heightmap_sample(const cf_heightmap_t* heightmap, float x, float y);
void cf_heightmap_destroy(cf_heightmap_t* heightmap);
```

### 等高线生成

```c
typedef struct {
    float interval;
    float min_height;
    float max_height;
    float simplify_tolerance;
    bool build_topology;
} cf_contour_config_t;

cf_result_t cf_contour_generate(const cf_heightmap_t* heightmap, const cf_contour_config_t* config, cf_model_t** model);
cf_result_t cf_contour_extract_parallel(const cf_heightmap_t* heightmap, float iso_value, cf_thread_pool_t* thread_pool, cf_point_set_t* point_set, cf_line_set_t* line_set);
```

### 线段简化和拓扑

```c
cf_result_t cf_simplify_douglas_peucker(const cf_point3_t* points, size_t count, float tolerance, cf_point3_t** out_points, size_t* out_count);
cf_result_t cf_simplify_visvalingam(const cf_point3_t* points, size_t count, size_t target_count, cf_point3_t** out_points, size_t* out_count);
cf_result_t cf_simplify_douglas_peucker_parallel(const cf_point3_t* points, size_t count, float tolerance, cf_thread_pool_t* thread_pool, cf_point3_t** out_points, size_t* out_count);
cf_result_t cf_topology_build(cf_model_t* model);
cf_result_t cf_topology_find_neighbors(const cf_model_t* model, cf_index_t line_index, cf_index_t** neighbors, size_t* count);
```

---

## 6. 渲染与相机

### 渲染器

```c
cf_result_t cf_renderer_init(const cf_renderer_config_t* config, cf_renderer_t** renderer);
cf_result_t cf_renderer_set_shader(cf_renderer_t* renderer, cf_shader_t* shader);
cf_result_t cf_renderer_set_model(cf_renderer_t* renderer, cf_model_t* model);
cf_result_t cf_renderer_set_lod_model(cf_renderer_t* renderer, cf_lod_model_t* lod_model);
void cf_renderer_set_auto_lod(cf_renderer_t* renderer, bool enable);
void cf_renderer_set_lod_debug(cf_renderer_t* renderer, bool enable);
cf_result_t cf_renderer_begin_frame(cf_renderer_t* renderer);
cf_result_t cf_renderer_render(cf_renderer_t* renderer);
cf_result_t cf_renderer_end_frame(cf_renderer_t* renderer);
bool cf_renderer_should_close(const cf_renderer_t* renderer);
cf_camera_t* cf_renderer_get_camera(cf_renderer_t* renderer);
void cf_renderer_destroy(cf_renderer_t* renderer);
```

### 相机

```c
cf_result_t cf_camera_create(cf_camera_type_t type, cf_camera_t** camera);
void cf_camera_set_perspective(cf_camera_t* camera, float fov, float aspect, float near, float far);
void cf_camera_set_orthographic(cf_camera_t* camera, float left, float right, float bottom, float top, float near, float far);
void cf_camera_set_position(cf_camera_t* camera, cf_point3_t position);
void cf_camera_set_target(cf_camera_t* camera, cf_point3_t target);
cf_point3_t cf_camera_get_position(const cf_camera_t* camera);
cf_point3_t cf_camera_get_target(const cf_camera_t* camera);
const float* cf_camera_get_view_matrix(const cf_camera_t* camera);
const float* cf_camera_get_projection_matrix(const cf_camera_t* camera);
void cf_camera_orbit(cf_camera_t* camera, float delta_yaw, float delta_pitch);
void cf_camera_zoom(cf_camera_t* camera, float delta);
void cf_camera_destroy(cf_camera_t* camera);
```

---

## 7. 标注系统

### 字体和文字渲染

```c
cf_result_t cf_font_load(const char* font_path, float size, cf_font_t** font);
void cf_font_destroy(cf_font_t* font);

cf_result_t cf_text_renderer_create(cf_font_t* font, cf_shader_t* shader, cf_text_renderer_t** renderer);
cf_result_t cf_text_renderer_render_3d(cf_text_renderer_t* renderer, const char* text, cf_point3_t position, cf_color_t color, const float* view_matrix, const float* projection_matrix);
float cf_text_renderer_measure_width(cf_text_renderer_t* renderer, const char* text);
void cf_text_renderer_destroy(cf_text_renderer_t* renderer);
```

### 标注配置

```c
typedef struct {
    float spacing;
    float min_distance;
    float max_distance;
    int lod_levels;
    char unit[16];
    int decimal_places;
    cf_color_t color;
    float size;
    bool show_index;
} cf_label_config_t;
```

### 标注管理器

```c
cf_result_t cf_label_manager_create(cf_text_renderer_t* text_renderer, const cf_label_config_t* config, cf_label_manager_t** manager);
cf_result_t cf_label_manager_generate_labels(cf_label_manager_t* manager, cf_model_t* model, cf_camera_t* camera);
cf_result_t cf_label_manager_update(cf_label_manager_t* manager, cf_camera_t* camera);
cf_result_t cf_label_manager_render(cf_label_manager_t* manager, const float* view_matrix, const float* projection_matrix);
void cf_label_manager_clear(cf_label_manager_t* manager);
void cf_label_manager_destroy(cf_label_manager_t* manager);
```

### 标注放置算法（v0.3.0）

```c
cf_result_t cf_place_labels_on_contour(const cf_line_set_t* lines, float height, float spacing, float min_distance, cf_point3_t* positions, size_t max_positions, size_t* out_count);
float cf_calculate_label_spacing_lod(float camera_distance, float base_spacing, int lod_levels);
cf_result_t cf_filter_labels_by_distance(const cf_point3_t* positions, size_t count, cf_point3_t camera_pos, float min_distance, float max_distance, bool* visible);
```

---

## 8. 控制与编辑

```c
void cf_input_update(cf_renderer_t* renderer, cf_input_state_t* input);

cf_result_t cf_selector_create(cf_model_t* model, cf_renderer_t* renderer, cf_selector_t** selector);
cf_result_t cf_selector_pick_point(cf_selector_t* selector, double screen_x, double screen_y, float radius, cf_index_t* out_index);
cf_result_t cf_selector_box_select(cf_selector_t* selector, double x1, double y1, double x2, double y2, cf_index_t** indices, size_t* count);
void cf_selector_destroy(cf_selector_t* selector);

cf_result_t cf_editor_create(cf_model_t* model, size_t max_undo_levels, cf_editor_t** editor);
cf_result_t cf_editor_move_point(cf_editor_t* editor, cf_index_t point_index, cf_point3_t new_position);
cf_result_t cf_editor_delete_point(cf_editor_t* editor, cf_index_t point_index);
cf_result_t cf_editor_insert_point_on_line(cf_editor_t* editor, cf_index_t line_index, float t, cf_index_t* out_point_index);
cf_result_t cf_editor_undo(cf_editor_t* editor);
cf_result_t cf_editor_redo(cf_editor_t* editor);
bool cf_editor_can_undo(const cf_editor_t* editor);
bool cf_editor_can_redo(const cf_editor_t* editor);
bool cf_editor_has_unsaved_changes(const cf_editor_t* editor);
void cf_editor_destroy(cf_editor_t* editor);
```

---

## 9. 完整示例

```c
#include <contourforge/contourforge.h>

int main(void) {
    cf_heightmap_t* heightmap = NULL;
    if (cf_heightmap_load("data/heightmaps/terrain_peaks.png", &heightmap) != CF_SUCCESS) {
        return 1;
    }

    cf_contour_config_t contour_config = {
        .interval = 10.0f,
        .min_height = heightmap->min_height,
        .max_height = heightmap->max_height,
        .simplify_tolerance = 0.5f,
        .build_topology = true
    };

    cf_model_t* model = NULL;
    if (cf_contour_generate(heightmap, &contour_config, &model) != CF_SUCCESS) {
        cf_heightmap_destroy(heightmap);
        return 1;
    }

    float distances[] = {50.0f, 150.0f, 300.0f};
    float ratios[] = {1.0f, 0.5f, 0.25f};
    cf_lod_config_t lod_config = {
        .level_count = 3,
        .distance_thresholds = distances,
        .simplification_ratios = ratios,
        .preserve_boundaries = true,
        .use_importance_sampling = true
    };

    cf_lod_model_t* lod = NULL;
    cf_lod_create(model, &lod_config, &lod);

    cf_lod_destroy(lod);
    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
    return 0;
}
```

---

## 10. 线程安全和最佳实践

- OpenGL相关API必须在拥有有效OpenGL上下文的线程调用。
- 数据生成、LOD生成和简化适合放在线程池后台执行。
- 模型对象修改不是全局线程安全的，多线程共享时需要外部同步。
- 小数据集不建议强行并行化，线程调度开销可能大于收益。
- RAW加载必须明确提供宽度、高度和数据类型。
- 标注渲染前应先根据相机调用 `cf_label_manager_update()`。

---

## 更多信息

- [格式支持](FORMAT_SUPPORT.md)
- [标注系统](LABEL_SYSTEM.md)
- [LOD系统](LOD_SYSTEM.md)
- [多线程系统](THREADING.md)
- [用户指南](USER_GUIDE.md)
- [性能优化](PERFORMANCE.md)

**文档版本**: 0.3.0
**最后更新**: 2026-05-01
