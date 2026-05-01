# Contourforge 用户指南

欢迎使用Contourforge！本指南将帮助您快速上手并掌握Contourforge的各项功能。

---

## 目录

1. [快速开始](#快速开始)
2. [基本概念](#基本概念)
3. [安装和配置](#安装和配置)
4. [使用教程](#使用教程)
5. [交互操作](#交互操作)
6. [性能优化](#性能优化)
7. [常见问题](#常见问题)

---

## 快速开始

### 5分钟上手

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
    
    // 3. 渲染
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

编译运行：
```bash
gcc -o viewer viewer.c -lcontourforge_core -lcontourforge_rendering -lcontourforge_datagen
./viewer
```

---

## 基本概念

### 数据结构层次

```
模型 (Model)
├── 点集 (Point Set)
│   └── 点 (Point) - 3D坐标 (x, y, z)
└── 线集 (Line Set)
    └── 线段 (Line) - 两个点的索引
```

### 工作流程

```
灰度图 → 高度图 → 等高线提取 → 线段简化 → 模型 → 渲染
```

### 核心概念

#### 1. 高度图 (Heightmap)
- 灰度图像，每个像素代表一个高度值
- 黑色(0) = 最低，白色(255) = 最高
- 支持PNG、JPG、BMP、TGA格式

#### 2. 等高线 (Contour)
- 连接相同高度点的线
- 由等高线间隔决定数量
- 使用Marching Squares算法提取

#### 3. 线段简化 (Simplification)
- 减少点数，保持形状
- Douglas-Peucker算法
- 容差越大，简化越多

#### 4. 拓扑 (Topology)
- 线段之间的连接关系
- 用于查找相邻线段
- 加速编辑操作

---

## 安装和配置

### Windows

```bash
# 使用Visual Studio
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# 运行示例
.\bin\Release\simple_viewer.exe
```

### Linux

```bash
# 安装依赖
sudo apt install build-essential cmake libgl1-mesa-dev

# 编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 运行示例
./bin/simple_viewer
```

### macOS

```bash
# 安装依赖
brew install cmake

# 编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)

# 运行示例
./bin/simple_viewer
```

详细构建说明请参考 [BUILD.md](../BUILD.md)

---

## 使用教程

### 教程1：加载和显示高度图

```c
#include <contourforge/contourforge.h>
#include <stdio.h>

int main() {
    // 加载高度图
    cf_heightmap_t* heightmap;
    cf_result_t result = cf_heightmap_load("data/heightmaps/terrain.png", &heightmap);
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 无法加载高度图\n");
        return 1;
    }
    
    // 打印信息
    printf("高度图信息:\n");
    printf("  尺寸: %d x %d\n", heightmap->width, heightmap->height);
    printf("  高度范围: %.2f - %.2f\n", heightmap->min_height, heightmap->max_height);
    
    // 采样几个点
    printf("\n采样点:\n");
    for (int i = 0; i < 5; i++) {
        float x = i / 4.0f;
        float y = 0.5f;
        float height = cf_heightmap_sample(heightmap, x, y);
        printf("  (%.2f, %.2f) = %.2f\n", x, y, height);
    }
    
    // 清理
    cf_heightmap_destroy(heightmap);
    return 0;
}
```

### 教程2：生成等高线

```c
#include <contourforge/contourforge.h>
#include <stdio.h>

int main() {
    // 加载高度图
    cf_heightmap_t* heightmap;
    cf_heightmap_load("terrain.png", &heightmap);
    
    // 配置等高线生成
    cf_contour_config_t config = {
        .interval = 10.0f,              // 每10米一条等高线
        .min_height = 0.0f,             // 从0米开始
        .max_height = 1000.0f,          // 到1000米结束
        .simplify_tolerance = 0.5f,     // 简化容差
        .build_topology = true          // 构建拓扑关系
    };
    
    // 生成等高线
    cf_model_t* model;
    cf_result_t result = cf_contour_generate(heightmap, &config, &model);
    
    if (result == CF_SUCCESS) {
        printf("等高线生成成功:\n");
        printf("  点数: %zu\n", model->points->count);
        printf("  线段数: %zu\n", model->lines->count);
        printf("  边界: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f)\n",
               model->bounds.min.x, model->bounds.min.y, model->bounds.min.z,
               model->bounds.max.x, model->bounds.max.y, model->bounds.max.z);
    }
    
    // 清理
    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
    return 0;
}
```

### 教程3：相机控制

```c
// 获取相机
cf_camera_t* camera = cf_renderer_get_camera(renderer);

// 设置透视投影
cf_camera_set_perspective(camera, 45.0f, 16.0f/9.0f, 0.1f, 1000.0f);

// 设置相机位置
cf_point3_t position = {10.0f, 10.0f, 10.0f};
cf_point3_t target = {0.0f, 0.0f, 0.0f};
cf_camera_set_position(camera, position);
cf_camera_set_target(camera, target);

// 在渲染循环中更新相机
while (!cf_renderer_should_close(renderer)) {
    // 处理输入
    if (mouse_dragging) {
        float yaw = mouse_delta_x * 0.005f;
        float pitch = mouse_delta_y * 0.005f;
        cf_camera_orbit(camera, yaw, pitch);
    }
    
    if (mouse_wheel_delta != 0) {
        cf_camera_zoom(camera, mouse_wheel_delta * 0.1f);
    }
    
    // 渲染
    cf_renderer_begin_frame(renderer);
    cf_renderer_render(renderer);
    cf_renderer_end_frame(renderer);
}
```

### 教程4：节点选择和编辑

```c
// 创建选择器和编辑器
cf_selector_t* selector;
cf_selector_create(model, renderer, &selector);

cf_editor_t* editor;
cf_editor_create(model, 100, &editor);  // 100层撤销

// 在鼠标点击时选择节点
void on_mouse_click(double mouse_x, double mouse_y) {
    cf_index_t selected_point;
    cf_result_t result = cf_selector_pick_point(
        selector, mouse_x, mouse_y, 5.0f, &selected_point
    );
    
    if (result == CF_SUCCESS) {
        printf("选中节点: %u\n", selected_point);
        
        // 移动节点
        cf_point3_t new_pos = {x, y, z};
        cf_editor_move_point(editor, selected_point, new_pos);
    }
}

// 撤销/重做
void on_key_press(int key) {
    if (key == CF_KEY_Z && ctrl_pressed) {
        if (cf_editor_can_undo(editor)) {
            cf_editor_undo(editor);
            printf("撤销\n");
        }
    }
    
    if (key == CF_KEY_Y && ctrl_pressed) {
        if (cf_editor_can_redo(editor)) {
            cf_editor_redo(editor);
            printf("重做\n");
        }
    }
}

// 清理
cf_selector_destroy(selector);
cf_editor_destroy(editor);
```

---

## 交互操作

### 默认控制

#### 鼠标操作
- **左键拖动**: 旋转视角
- **右键拖动**: 平移视图
- **滚轮**: 缩放
- **左键点击**: 选择节点
- **Shift+左键拖动**: 框选

#### 键盘操作
- **W/A/S/D**: 移动相机
- **Space**: 重置视角
- **Ctrl+Z**: 撤销
- **Ctrl+Y**: 重做
- **Delete**: 删除选中节点
- **ESC**: 退出

### 自定义控制

```c
// 处理输入
cf_input_state_t input = {0};
cf_input_update(renderer, &input);

// 检查按键
if (input.keys[CF_KEY_W]) {
    // W键被按下
}

// 检查鼠标按钮
if (input.mouse_buttons[CF_MOUSE_BUTTON_LEFT]) {
    // 左键被按下
}

// 获取鼠标位置
double mouse_x = input.mouse_x;
double mouse_y = input.mouse_y;

// 获取鼠标移动量
double delta_x = input.mouse_delta_x;
double delta_y = input.mouse_delta_y;
```

---

## 性能优化

### 1. 调整等高线间隔

```c
// 间隔越大，等高线越少，性能越好
cf_contour_config_t config = {
    .interval = 20.0f,  // 从10.0f增加到20.0f
    // ...
};
```

### 2. 使用线段简化

```c
// 增加简化容差
cf_contour_config_t config = {
    .simplify_tolerance = 1.0f,  // 从0.5f增加到1.0f
    // ...
};
```

### 3. 启用视锥剔除

```c
// 创建八叉树
cf_octree_t* octree;
cf_octree_create(model->points, &model->bounds, 100, &octree);

// 查询可见节点
cf_index_t* visible_indices;
size_t visible_count;
cf_octree_query_frustum(octree, view_proj_matrix, &visible_indices, &visible_count);

// 只渲染可见节点
```

### 4. 降低MSAA采样

```c
cf_renderer_config_t config = {
    .msaa_samples = 2,  // 从4降低到2
    // ...
};
```

### 5. 使用静态库

```bash
# 静态链接性能更好
cmake .. -DCF_BUILD_SHARED=OFF
```

---

## 常见问题

### Q1: 编译错误：找不到OpenGL

**Windows**:
- 确保安装了Windows SDK
- OpenGL通常已包含在系统中

**Linux**:
```bash
sudo apt install libgl1-mesa-dev
```

**macOS**:
- OpenGL已包含在系统中

### Q2: 运行时错误：无法加载高度图

检查：
1. 文件路径是否正确
2. 文件格式是否支持（PNG、JPG、BMP、TGA）
3. 文件是否损坏

```c
cf_result_t result = cf_heightmap_load("terrain.png", &heightmap);
if (result != CF_SUCCESS) {
    fprintf(stderr, "错误码: %d\n", result);
    // CF_ERROR_FILE_NOT_FOUND = -3
    // CF_ERROR_INVALID_FORMAT = -5
}
```

### Q3: 性能问题：帧率低

优化建议：
1. 减少等高线数量（增加间隔）
2. 增加简化容差
3. 启用视锥剔除
4. 降低MSAA采样
5. 使用Release构建

### Q4: 内存占用过高

优化建议：
1. 使用线段简化
2. 不构建拓扑（如果不需要）
3. 使用内存池
4. 及时释放不用的资源

### Q5: 如何导出模型？

当前版本暂不支持导出，计划在v0.2.0添加：
- OBJ格式
- STL格式
- PLY格式

### Q6: 支持哪些图像格式？

支持的格式：
- PNG（推荐）
- JPG/JPEG
- BMP
- TGA

不支持：
- GIF
- TIFF
- WebP

### Q7: 如何处理大规模数据？

建议：
1. 分块加载
2. 使用LOD（计划中）
3. 启用视锥剔除
4. 使用八叉树索引

### Q8: 可以用于商业项目吗？

可以，但需要遵守AGPL-3.0许可证：
- 如果修改代码并提供网络服务，必须开源
- 如果只是使用库，不需要开源您的应用

---

## 更多资源

- [API参考](API.md) - 完整API文档
- [架构设计](../ARCHITECTURE.md) - 技术细节
- [示例代码](../examples/) - 更多示例
- [性能优化](PERFORMANCE.md) - 性能分析

---

## 获取帮助

- 📖 查看文档
- 💬 [讨论区](https://github.com/czxieddan/contourforge/discussions)
- 🐛 [问题追踪](https://github.com/czxieddan/contourforge/issues)
- 📧 czxieddan@gmail.com

---

**最后更新**: 2026-04-30
