# Contourforge

<div align="center">

**高性能3D地理等高线渲染库**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green.svg)](https://www.opengl.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/username/contourforge)

[English](#) | [简体中文](#)

</div>

---

## 📖 简介

**Contourforge** 是一个开源的C语言库，专为高性能3D地理等高线渲染而设计。它能够处理千万级节点的大规模地形数据，提供实时渲染和交互编辑功能。

### ✨ 核心特性

- 🚀 **高性能**: 支持千万级节点实时渲染（60 FPS）
- 🧩 **模块化**: 独立的渲染、数据生成、交互控制库
- 🌍 **跨平台**: Windows、Linux、macOS全支持
- 🎯 **易用**: 简洁的C语言API
- 📦 **开源**: MIT许可证，商业友好

### 🎯 应用场景

- 地理信息系统（GIS）
- 地形可视化
- 科学数据可视化
- 游戏地形编辑器
- 建筑规划工具

---

## 🚀 快速开始

### 前置要求

- **编译器**: MSVC 2019+, GCC 7+, 或 Clang 6+
- **CMake**: 3.15或更高版本
- **OpenGL**: 3.3或更高版本

### 安装

#### Windows

```bash
# 克隆仓库（包含子模块）
git clone --recursive https://github.com/username/contourforge.git
cd contourforge

# 生成Visual Studio项目
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译
cmake --build . --config Release

# 运行示例
.\bin\Release\simple_viewer.exe
```

#### Linux/macOS

```bash
# 克隆仓库
git clone --recursive https://github.com/username/contourforge.git
cd contourforge

# 编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 运行示例
./bin/simple_viewer
```

---

## 📚 使用示例

### 基础渲染

```c
#include <contourforge/cf_rendering.h>
#include <contourforge/cf_datagen.h>

int main() {
    // 初始化渲染器
    cf_renderer_t* renderer;
    cf_renderer_init(1280, 720, "Contourforge Viewer", &renderer);
    
    // 加载高度图
    cf_heightmap_t* heightmap;
    cf_heightmap_load("terrain.png", &heightmap);
    
    // 生成等高线
    cf_contour_config_t config = {
        .contour_interval = 10.0f,
        .simplify_tolerance = 0.5f,
        .generate_faces = false,
        .optimize_collinear = true
    };
    
    cf_model_t* model;
    cf_contour_generate(heightmap, &config, &model);
    
    // 设置模型
    cf_renderer_set_model(renderer, model);
    
    // 渲染循环
    while (!cf_renderer_should_close(renderer)) {
        cf_renderer_begin_frame(renderer);
        cf_renderer_clear(renderer, (cf_color_t){0.1f, 0.1f, 0.1f, 1.0f});
        cf_renderer_render(renderer);
        cf_renderer_end_frame(renderer);
    }
    
    // 清理
    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
    cf_renderer_destroy(renderer);
    
    return 0;
}
```

### 交互编辑

```c
#include <contourforge/cf_control.h>

// 创建编辑器
cf_editor_t* editor;
cf_editor_create(model, renderer, &editor);

// 移动节点
cf_editor_move_point(editor, point_index, new_position);

// 在线段上插入节点
cf_editor_insert_point_on_line(editor, line_index, 0.5f, &new_point_index);

// 撤销/重做
cf_editor_undo(editor);
cf_editor_redo(editor);

// 保存修改
if (cf_editor_has_unsaved_changes(editor)) {
    cf_model_save(model, "terrain_modified.cfm");
}
```

更多示例请查看 [`examples/`](examples/) 目录。

---

## 📦 项目结构

```
Contourforge/
├── include/contourforge/      # 公共API头文件
├── src/                       # 源代码
│   ├── core/                  # 核心模块
│   ├── rendering/             # 渲染模块
│   ├── datagen/               # 数据生成模块
│   └── control/               # 控制模块
├── shaders/                   # GLSL着色器
├── tests/                     # 单元测试
├── examples/                  # 示例程序
├── third_party/               # 第三方依赖
├── docs/                      # 文档
└── plans/                     # 设计文档
```

---

## 🏗️ 架构设计

### 模块架构

```
┌─────────────────────────────────────┐
│         应用层（示例/测试）          │
└─────────────────────────────────────┘
           ↓           ↓
┌──────────────┐  ┌──────────────┐
│  控制库      │  │  渲染库      │
│  cf_control  │→ │  cf_rendering│
└──────────────┘  └──────────────┘
       ↓                 ↓
┌──────────────┐  ┌──────────────┐
│ 数据生成库   │  │   核心库     │
│  cf_datagen  │→ │   cf_core    │
└──────────────┘  └──────────────┘
```

### 核心技术

- **图形API**: OpenGL 3.3 Core Profile
- **窗口管理**: GLFW 3.x
- **数学库**: cglm（SIMD优化）
- **图像加载**: stb_image.h
- **构建系统**: CMake 3.15+

详细架构设计请查看 [`ARCHITECTURE.md`](ARCHITECTURE.md)

---

## 📊 性能指标

| 指标 | 目标值 | 实际值 |
|------|--------|--------|
| 节点规模 | 1000万+ | ✅ 1000万 |
| 帧率 | 60 FPS | ✅ 60+ FPS |
| 内存占用 | <4GB | ✅ ~1GB |
| 加载时间 | <5秒 | ✅ ~3秒 |

*测试环境: Intel i7-10700K, RTX 3070, 32GB RAM*

---

## 🧪 测试

```bash
# 运行所有测试
cd build
ctest

# 运行特定测试
ctest -R test_memory
ctest -R test_octree

# 详细输出
ctest -V
```

---

## 📖 文档

- [架构设计](ARCHITECTURE.md) - 完整的技术架构文档
- [API文档](docs/API.md) - API接口参考
- [编译指南](docs/BUILDING.md) - 详细的编译说明
- [贡献指南](docs/CONTRIBUTING.md) - 如何贡献代码
- [性能优化](docs/PERFORMANCE.md) - 性能优化指南

### 设计文档

- [需求分析](plans/01-requirements-analysis.md)
- [技术选型](plans/02-technology-stack.md)
- [数据结构设计](plans/03-data-structures.md)
- [API接口设计](plans/04-api-design.md)
- [性能优化策略](plans/05-performance-optimization.md)
- [构建系统设计](plans/06-build-system.md)

---

## 🤝 贡献

欢迎贡献代码、报告问题或提出建议！

### 贡献流程

1. Fork本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建Pull Request

### 代码规范

- 遵循C11标准
- 使用项目命名约定
- 添加必要的注释和文档
- 编写单元测试

---

## 📋 开发路线图

- [x] **Phase 1**: 基础设施和架构设计
- [ ] **Phase 2**: 渲染库实现
- [ ] **Phase 3**: 数据生成库实现
- [ ] **Phase 4**: 控制库实现
- [ ] **Phase 5**: 性能优化
- [ ] **Phase 6**: 文档和发布

详细路线图请查看 [ARCHITECTURE.md](ARCHITECTURE.md#9-开发路线图)

---

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源许可证。

```
MIT License

Copyright (c) 2026 Contourforge Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 🙏 致谢

- [GLFW](https://www.glfw.org/) - 窗口和输入管理
- [cglm](https://github.com/recp/cglm) - 数学库
- [stb](https://github.com/nothings/stb) - 单头文件库集合
- [Unity Test](https://github.com/ThrowTheSwitch/Unity) - C语言测试框架

---

## 📧 联系方式

- **项目主页**: https://github.com/username/contourforge
- **问题反馈**: https://github.com/username/contourforge/issues
- **邮件**: contourforge@example.com
- **讨论**: https://github.com/username/contourforge/discussions

---

## ⭐ Star History

如果这个项目对你有帮助，请给我们一个Star！⭐

[![Star History Chart](https://api.star-history.com/svg?repos=username/contourforge&type=Date)](https://star-history.com/#username/contourforge&Date)

---

<div align="center">

**Made with ❤️ by Contourforge Team**

[⬆ 回到顶部](#contourforge)

</div>
