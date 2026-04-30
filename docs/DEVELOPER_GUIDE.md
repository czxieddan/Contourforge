# Contourforge 开发者指南

本指南面向希望参与Contourforge开发或深入了解其内部实现的开发者。

---

## 目录

1. [项目结构](#项目结构)
2. [开发环境设置](#开发环境设置)
3. [编码规范](#编码规范)
4. [模块详解](#模块详解)
5. [添加新功能](#添加新功能)
6. [调试技巧](#调试技巧)
7. [性能分析](#性能分析)
8. [测试指南](#测试指南)

---

## 项目结构

### 目录组织

```
Contourforge/
├── include/contourforge/      # 公共API头文件
│   ├── contourforge.h         # 主头文件
│   ├── types.h                # 类型定义
│   ├── core.h                 # 核心模块API
│   ├── rendering.h            # 渲染模块API
│   ├── datagen.h              # 数据生成API
│   └── control.h              # 控制模块API
├── src/                       # 源代码实现
│   ├── core/                  # 核心模块
│   │   ├── memory.c           # 内存管理
│   │   ├── data_structures.c  # 数据结构
│   │   ├── octree.c           # 八叉树
│   │   └── version.c          # 版本信息
│   ├── rendering/             # 渲染模块
│   │   ├── renderer.c         # 渲染器
│   │   ├── camera.c           # 相机
│   │   ├── shader.c           # 着色器
│   │   ├── buffer.c           # GPU缓冲
│   │   └── mesh.c             # 网格
│   ├── datagen/               # 数据生成
│   │   ├── heightmap.c        # 高度图
│   │   ├── contour.c          # 等高线
│   │   ├── simplify.c         # 简化
│   │   └── topology.c         # 拓扑
│   └── control/               # 控制模块
│       ├── input.c            # 输入处理
│       ├── selection.c        # 选择
│       └── editor.c           # 编辑器
├── shaders/                   # GLSL着色器
├── tests/                     # 单元测试
├── examples/                  # 示例程序
├── data/                      # 测试数据
├── third_party/               # 第三方依赖
├── docs/                      # 文档
└── plans/                     # 设计文档
```

### 模块依赖

```
cf_control → cf_rendering → cf_core
           ↘              ↗
            cf_datagen →
```

**依赖规则**:
- 核心库不依赖其他模块
- 避免循环依赖
- 使用前向声明减少头文件依赖

---

## 开发环境设置

### 必需工具

- **编译器**: GCC 7+, Clang 6+, MSVC 2019+
- **CMake**: 3.15+
- **Git**: 版本控制
- **调试器**: GDB, LLDB, Visual Studio Debugger

### 推荐工具

- **IDE**: Visual Studio Code, CLion, Visual Studio
- **静态分析**: Clang-Tidy, Cppcheck
- **内存检查**: Valgrind, AddressSanitizer
- **性能分析**: Perf, VTune, Instruments

### 设置步骤

```bash
# 1. 克隆仓库
git clone https://github.com/username/contourforge.git
cd contourforge

# 2. 初始化子模块
git submodule update --init --recursive

# 3. 创建构建目录
mkdir build && cd build

# 4. 配置（Debug模式）
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCF_BUILD_TESTS=ON \
         -DCF_BUILD_EXAMPLES=ON

# 5. 编译
cmake --build . -j8

# 6. 运行测试
ctest -V
```

### IDE配置

#### Visual Studio Code

`.vscode/settings.json`:
```json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.configureOnOpen": true,
    "files.associations": {
        "*.h": "c"
    }
}
```

#### CLion

- 自动检测CMake项目
- 配置工具链
- 设置代码风格

---

## 编码规范

### 命名约定

```c
// 函数: cf_<module>_<action>
cf_result_t cf_model_create(const char* name, cf_model_t** model);

// 类型: cf_<name>_t
typedef struct cf_model cf_model_t;

// 枚举: CF_<NAME>
typedef enum {
    CF_SUCCESS = 0,
    CF_ERROR_INVALID_PARAM = -1
} cf_result_t;

// 宏: CF_<NAME>
#define CF_MAX_PATH 260

// 私有函数: cf_<module>_<name>_internal
static void cf_model_update_internal(cf_model_t* model);
```

### 代码风格

```c
// 缩进: 4个空格
// 大括号: K&R风格
if (condition) {
    // 代码
} else {
    // 代码
}

// 函数定义
cf_result_t cf_model_create(
    const char* name,
    cf_model_t** model
) {
    // 参数检查
    if (!model) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 实现
    // ...
    
    return CF_SUCCESS;
}

// 结构体定义
typedef struct {
    float x, y, z;  // 简单字段可以一行
} cf_point3_t;

typedef struct {
    cf_point3_t* points;    // 复杂字段分行
    size_t count;
    size_t capacity;
} cf_point_set_t;
```

### 注释规范

```c
/**
 * @brief 创建模型
 * 
 * 分配内存并初始化模型结构。模型包含点集和线集。
 * 
 * @param name 模型名称（可以为NULL）
 * @param model 输出模型指针
 * @return CF_SUCCESS 或错误码
 * 
 * @note 使用完毕后必须调用 cf_model_destroy() 释放
 * @warning 不要手动释放model指向的内存
 * 
 * @see cf_model_destroy
 * 
 * @example
 * ```c
 * cf_model_t* model;
 * if (cf_model_create("Terrain", &model) == CF_SUCCESS) {
 *     // 使用模型
 *     cf_model_destroy(model);
 * }
 * ```
 */
cf_result_t cf_model_create(const char* name, cf_model_t** model);
```

---

## 模块详解

### 核心模块 (cf_core)

#### 内存管理

```c
// memory.c
struct cf_memory_pool {
    void* memory;               // 内存块
    size_t block_size;          // 块大小
    size_t block_count;         // 块数量
    uint64_t* free_bitmap;      // 空闲位图
    size_t free_count;          // 空闲块数
};

// 分配算法：位图扫描
void* cf_memory_pool_alloc(cf_memory_pool_t* pool) {
    // 1. 扫描位图找到空闲块
    // 2. 标记为已使用
    // 3. 返回内存指针
}
```

#### 八叉树

```c
// octree.c
struct cf_octree_node {
    cf_bounds_t bounds;                 // 节点边界
    cf_index_t* point_indices;          // 点索引
    size_t point_count;                 // 点数量
    struct cf_octree_node* children[8]; // 8个子节点
    bool is_leaf;                       // 是否叶节点
};

// 构建算法：递归分割
void build_octree_recursive(
    cf_octree_node_t* node,
    const cf_point_set_t* points,
    size_t max_points
) {
    if (node->point_count <= max_points) {
        node->is_leaf = true;
        return;
    }
    
    // 分割成8个子节点
    // 递归构建
}
```

### 渲染模块 (cf_rendering)

#### 渲染管线

```c
// renderer.c
struct cf_renderer {
    GLFWwindow* window;         // GLFW窗口
    cf_camera_t* camera;        // 相机
    cf_shader_t* shader;        // 着色器
    cf_model_t* model;          // 模型
    GLuint vao, vbo;            // OpenGL对象
    int width, height;          // 窗口尺寸
};

// 渲染流程
void cf_renderer_render(cf_renderer_t* renderer) {
    // 1. 更新相机矩阵
    // 2. 绑定着色器
    // 3. 设置uniform
    // 4. 绑定VAO
    // 5. 绘制
    glDrawArrays(GL_LINES, 0, line_count * 2);
}
```

### 数据生成模块 (cf_datagen)

#### Marching Squares算法

```c
// contour.c
void extract_contours(
    const cf_heightmap_t* heightmap,
    float level,
    cf_model_t* model
) {
    // 遍历每个网格单元
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            // 获取4个角的高度
            float h00 = sample(x, y);
            float h10 = sample(x+1, y);
            float h11 = sample(x+1, y+1);
            float h01 = sample(x, y+1);
            
            // 计算case索引
            int case_index = 0;
            if (h00 >= level) case_index |= 1;
            if (h10 >= level) case_index |= 2;
            if (h11 >= level) case_index |= 4;
            if (h01 >= level) case_index |= 8;
            
            // 根据case生成线段
            generate_lines(case_index, x, y, level, model);
        }
    }
}
```

---

## 添加新功能

### 示例：添加新的简化算法

#### 1. 在头文件中声明

```c
// include/contourforge/datagen.h
/**
 * @brief Visvalingam-Whyatt线段简化
 */
cf_result_t cf_simplify_visvalingam(
    const cf_point3_t* points,
    size_t count,
    size_t target_count,
    cf_point3_t** out_points,
    size_t* out_count
);
```

#### 2. 实现功能

```c
// src/datagen/simplify.c
cf_result_t cf_simplify_visvalingam(
    const cf_point3_t* points,
    size_t count,
    size_t target_count,
    cf_point3_t** out_points,
    size_t* out_count
) {
    // 参数检查
    if (!points || !out_points || !out_count) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    // 实现算法
    // ...
    
    return CF_SUCCESS;
}
```

#### 3. 编写测试

```c
// tests/test_simplify.c
void test_visvalingam_basic(void) {
    cf_point3_t points[] = {
        {0, 0, 0}, {1, 1, 0}, {2, 0, 0}, {3, 1, 0}, {4, 0, 0}
    };
    
    cf_point3_t* simplified;
    size_t simplified_count;
    
    cf_result_t result = cf_simplify_visvalingam(
        points, 5, 3, &simplified, &simplified_count
    );
    
    TEST_ASSERT_EQUAL(CF_SUCCESS, result);
    TEST_ASSERT_EQUAL(3, simplified_count);
    
    free(simplified);
}
```

#### 4. 更新文档

- 更新 `docs/API.md`
- 添加使用示例
- 更新 `CHANGELOG.md`

---

## 调试技巧

### GDB调试

```bash
# 编译Debug版本
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 启动GDB
gdb ./bin/test_core

# 常用命令
(gdb) break cf_model_create    # 设置断点
(gdb) run                       # 运行
(gdb) next                      # 单步执行
(gdb) print model               # 打印变量
(gdb) backtrace                 # 查看调用栈
```

### Valgrind内存检查

```bash
# 检查内存泄漏
valgrind --leak-check=full ./bin/test_memory

# 检查未初始化内存
valgrind --track-origins=yes ./bin/test_core
```

### AddressSanitizer

```bash
# 编译时启用
cmake .. -DCMAKE_C_FLAGS="-fsanitize=address -g"

# 运行测试
./bin/test_memory
```

### 日志调试

```c
// 添加调试日志
#ifdef DEBUG
#define CF_LOG(fmt, ...) \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define CF_LOG(fmt, ...)
#endif

// 使用
CF_LOG("Creating model: %s", name);
CF_LOG("Point count: %zu", model->points->count);
```

---

## 性能分析

### Linux Perf

```bash
# 记录性能数据
perf record -g ./bin/heightmap_loader terrain.png

# 查看报告
perf report

# 查看热点函数
perf top
```

### 性能计时

```c
#include <time.h>

// 计时宏
#define CF_TIME_START(name) \
    clock_t _time_##name = clock()

#define CF_TIME_END(name) \
    do { \
        clock_t _elapsed = clock() - _time_##name; \
        printf("%s: %.3f ms\n", #name, \
               _elapsed * 1000.0 / CLOCKS_PER_SEC); \
    } while(0)

// 使用
CF_TIME_START(contour_generation);
cf_contour_generate(heightmap, &config, &model);
CF_TIME_END(contour_generation);
```

---

## 测试指南

### 单元测试结构

```c
// tests/test_module.c
#include "unity.h"
#include <contourforge/module.h>

void setUp(void) {
    // 每个测试前执行
}

void tearDown(void) {
    // 每个测试后执行
}

void test_function_basic(void) {
    // 测试基本功能
}

void test_function_edge_cases(void) {
    // 测试边界情况
}

void test_function_error_handling(void) {
    // 测试错误处理
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_basic);
    RUN_TEST(test_function_edge_cases);
    RUN_TEST(test_function_error_handling);
    return UNITY_END();
}
```

### 测试覆盖率

```bash
# 编译时启用覆盖率
cmake .. -DCMAKE_C_FLAGS="--coverage"

# 运行测试
ctest

# 生成报告
gcov src/core/*.c
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

---

## 更多资源

- [架构文档](../ARCHITECTURE.md)
- [API参考](API.md)
- [贡献指南](../CONTRIBUTING.md)
- [性能优化](PERFORMANCE.md)

---

**最后更新**: 2026-04-30
