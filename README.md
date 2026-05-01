# Contourforge

<div align="center">

**高性能3D地理等高线渲染库**

[![Version](https://img.shields.io/badge/version-v0.3.0-brightgreen.svg)](https://github.com/czxieddan/contourforge/releases/tag/v0.3.0)
[![License: AGPL-3.0](https://img.shields.io/badge/License-AGPL%203.0-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green.svg)](https://www.opengl.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/czxieddan/contourforge)

</div>

---

## 简介

Contourforge是一个开源的高性能3D地理等高线渲染库，使用C11开发，专为处理大规模地形和高度图数据而设计。它提供从高度图加载、等高线提取、线段简化、LOD、多线程处理到实时OpenGL渲染和交互编辑的完整工作流。

## v0.3.0 版本亮点

- **📁 多格式支持**：PNG、JPEG、BMP、TIFF、GeoTIFF识别和RAW高度数据加载。
- **🏷️ 等高线标注**：TrueType字体、3D Billboard文字、自动标注放置、距离LOD和碰撞检测。
- **📊 测试完善**：新增LOD、线程池、标注、格式加载测试，单元测试套件扩展到10个测试程序。

## 核心特性

- **多格式高度图输入**：标准图像格式、8/16/32位TIFF、6种RAW数据类型。
- **等高线生成**：Marching Squares等高线提取，支持自定义间隔、范围和拓扑构建。
- **LOD系统**：多级细节层次、距离选择、统计信息和并行生成。
- **多线程支持**：跨平台线程池、任务队列、并行等高线/LOD/简化处理。
- **3D标注系统**：高度值文字渲染、自动放置、最小间距、可见距离过滤。
- **高性能渲染**：OpenGL 3.3 Core、相机控制、着色器和GPU缓冲管理。
- **交互编辑**：节点选择、移动、插入、删除、撤销和重做。
- **空间索引**：八叉树加速空间查询。
- **模块化设计**：核心、数据生成、渲染、控制模块可独立集成。

## 快速开始

### 前置要求

- MSVC 2019+、GCC 7+或Clang 6+
- CMake 3.15+
- OpenGL 3.3+
- Git

### Windows

```bash
git clone --recursive https://github.com/czxieddan/contourforge.git
cd contourforge
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCF_BUILD_TESTS=ON -DCF_BUILD_EXAMPLES=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\bin\Release\heightmap_loader.exe data\heightmaps\terrain_peaks.png 10.0
```

### Linux

```bash
sudo apt install build-essential cmake libgl1-mesa-dev
git clone --recursive https://github.com/czxieddan/contourforge.git
cd contourforge
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCF_BUILD_TESTS=ON -DCF_BUILD_EXAMPLES=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/bin/heightmap_loader data/heightmaps/terrain_peaks.png 10.0
```

### macOS

```bash
brew install cmake
git clone --recursive https://github.com/czxieddan/contourforge.git
cd contourforge
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCF_BUILD_TESTS=ON -DCF_BUILD_EXAMPLES=ON
cmake --build build -j$(sysctl -n hw.ncpu)
ctest --test-dir build --output-on-failure
./build/bin/heightmap_loader data/heightmaps/terrain_peaks.png 10.0
```

## 使用示例

### 1. 自动加载高度图并生成等高线

```c
#include <contourforge/contourforge.h>

int main(void) {
    cf_heightmap_t* heightmap = NULL;
    if (cf_heightmap_load("data/heightmaps/terrain_peaks.png", &heightmap) != CF_SUCCESS) {
        return 1;
    }

    cf_contour_config_t config = {
        .interval = 10.0f,
        .min_height = heightmap->min_height,
        .max_height = heightmap->max_height,
        .simplify_tolerance = 0.5f,
        .build_topology = true
    };

    cf_model_t* model = NULL;
    cf_contour_generate(heightmap, &config, &model);

    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
    return 0;
}
```

### 2. 加载TIFF和RAW数据

```c
cf_heightmap_format_t format = cf_heightmap_detect_format("terrain.tif");
printf("format=%s\n", cf_heightmap_format_name(format));

cf_heightmap_t* tiff_map = NULL;
cf_heightmap_load_tiff("terrain.tif", &tiff_map);

cf_heightmap_t* raw_map = NULL;
cf_heightmap_load_raw("terrain_u16.raw", 1024, 1024, CF_RAW_FORMAT_U16, &raw_map);
```

### 3. 启用等高线标注

```c
cf_font_t* font = NULL;
cf_font_load("data/fonts/default.ttf", 18.0f, &font);

cf_shader_t* text_shader = NULL;
cf_shader_load("shaders/text.vert", "shaders/text.frag", &text_shader);

cf_text_renderer_t* text_renderer = NULL;
cf_text_renderer_create(font, text_shader, &text_renderer);

cf_label_config_t label_config = {
    .spacing = 50.0f,
    .min_distance = 5.0f,
    .max_distance = 1000.0f,
    .lod_levels = 4,
    .unit = "m",
    .decimal_places = 1,
    .color = {1.0f, 1.0f, 1.0f, 1.0f},
    .size = 18.0f,
    .show_index = false
};

cf_label_manager_t* labels = NULL;
cf_label_manager_create(text_renderer, &label_config, &labels);
cf_label_manager_generate_labels(labels, model, camera);
```

### 4. 创建LOD模型

```c
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
int level = cf_lod_select_level(lod, 120.0f);
```

## 示例程序

| 程序 | 说明 |
|------|------|
| `simple_viewer` | 基础3D查看器 |
| `heightmap_loader` | 高度图加载和等高线生成 |
| `interactive_editor` | 交互式编辑器 |
| `lod_demo` | LOD系统演示 |
| `threading_benchmark` | 多线程性能基准测试 |
| `format_converter` | 高度图格式信息查看和RAW转换 |
| `label_demo` | 等高线标注系统演示 |

## 测试

```bash
ctest --test-dir build -C Debug --output-on-failure
ctest --test-dir build -C Debug -R "test_lod|test_threading|test_label|test_formats" -V
```

v0.3.0测试套件包含：

- `test_core`
- `test_memory`
- `test_octree`
- `test_datagen`
- `test_contour`
- `test_simplify`
- `test_formats`
- `test_lod`
- `test_threading`
- `test_label`

## 项目结构

```text
Contourforge/
├── include/contourforge/      # 公共API头文件
├── src/                       # 源代码实现
│   ├── core/                  # 核心、LOD、线程池
│   ├── rendering/             # OpenGL、文字、标注
│   ├── datagen/               # 高度图、格式加载、等高线、简化
│   └── control/               # 输入、选择、编辑
├── shaders/                   # GLSL着色器
├── tests/                     # 单元测试
├── examples/                  # 示例程序
├── data/                      # 测试高度图和字体
├── third_party/               # 第三方依赖
└── docs/                      # 文档
```

## 性能指标

| 能力 | v0.3.0状态 |
|------|------------|
| LOD渲染性能提升 | 2-5x |
| 多线程数据处理加速 | 2-4x（4核CPU典型场景） |
| RAW 512x512加载 | ~5ms |
| TIFF 512x512加载 | ~20ms（未压缩灰度） |
| 测试套件 | 10个测试程序，Windows Debug验证通过 |

## 文档

- [API参考](docs/API.md)
- [格式支持](docs/FORMAT_SUPPORT.md)
- [标注系统](docs/LABEL_SYSTEM.md)
- [LOD系统](docs/LOD_SYSTEM.md)
- [多线程系统](docs/THREADING.md)
- [用户指南](docs/USER_GUIDE.md)
- [构建指南](BUILD.md)
- [变更日志](CHANGELOG.md)
- [v0.3.0发布说明](RELEASE_NOTES_v0.3.0.md)

## 已知问题

- 标注在极近距离或极高密度线段场景可能重叠。
- TIFF压缩格式支持有限，推荐使用未压缩灰度TIFF。
- GeoTIFF当前仅识别格式，完整坐标系统解析将在后续版本完善。
- OpenGL相关API必须在有效上下文中调用。

## 许可证

本项目采用 **GNU Affero General Public License v3.0 (AGPL-3.0)** 许可证，详见 [LICENSE](LICENSE)。

## 联系方式

- 项目主页：https://github.com/czxieddan/contourforge
- 问题反馈：https://github.com/czxieddan/contourforge/issues
- 发布页：https://github.com/czxieddan/contourforge/releases
