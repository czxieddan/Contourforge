# Contourforge

<div align="center">

**高性能3D地理等高线渲染库**

[![License: AGPL-3.0](https://img.shields.io/badge/License-AGPL%203.0-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green.svg)](https://www.opengl.org/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/username/contourforge)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/username/contourforge)

[English](#) | [简体中文](#)

</div>

---

## 📖 简介

**Contourforge** 是一个开源的高性能3D地理等高线渲染库，使用C语言开发，专为处理千万级节点的大规模地形数据而设计。它提供了从高度图加载、等高线提取、线段简化到实时渲染和交互编辑的完整工作流。

### ✨ 核心特性

- 🚀 **高性能渲染**: 支持千万级节点实时渲染（60 FPS）
- 🧩 **模块化设计**: 独立的核心、渲染、数据生成、交互控制库
- 🌍 **跨平台**: Windows、Linux、macOS全支持
- 🎯 **易于集成**: 简洁的C语言API，清晰的文档
- 📊 **完整工作流**: 从灰度图到3D模型的一站式解决方案
- 🔧 **灵活配置**: 支持多种等高线生成和简化算法
- 🎨 **实时编辑**: 支持节点选择、移动、插入和删除
- 🔍 **空间索引**: 八叉树加速空间查询和视锥剔除
- 💾 **内存优化**: 内存池管理，减少碎片和分配开销
- 📖 **开源友好**: AGPL-3.0许可证

### 🎯 应用场景

- 地理信息系统（GIS）可视化
- 地形数据分析和处理
- 科学数据可视化
- 游戏地形编辑器
- 建筑规划和城市设计工具
- 教育和研究项目

---

## 🚀 快速开始

### 前置要求

- **编译器**: MSVC 2019+, GCC 7+, 或 Clang 6+（支持C11标准）
- **CMake**: 3.15或更高版本
- **OpenGL**: 3.3或更高版本
- **Git**: 用于克隆仓库和子模块

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

#### Linux

```bash
# 安装依赖
sudo apt install build-essential cmake libgl1-mesa-dev

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

#### macOS

```bash
# 安装依赖（需要Homebrew）
brew install cmake

# 克隆仓库
git clone --recursive https://github.com/username/contourforge.git
cd contourforge

# 编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)

# 运行示例
./bin/simple_viewer
```

---

## 📚 使用示例

### 基础渲染

从高度图生成等高线并渲染：

```c
#include <contourforge/contourforge.h>

int main() {
    // 1. 加载高度图
    cf_heightmap_t* heightmap;
    cf_heightmap_load("data/heightmaps/terrain.png", &heightmap);
    
    // 2. 配置等高线生成参数
    cf_contour_config_t config = {
        .interval = 10.0f,              // 等高线间隔10米
        .min_height = 0.0f,             // 最小高度
        .max_height = 1000.0f,          // 最大高度
        .simplify_tolerance = 0.5f,     // 简化容差
        .build_topology = true          // 构建拓扑关系
    };
    
    // 3. 生成等高线模型
    cf_model_t* model;
    cf_contour_generate(heightmap, &config, &model);
    
    // 4. 初始化渲染器
    cf_renderer_config_t renderer_config = {
        .width = 1280,
        .height = 720,
        .title = "Contourforge Viewer",
        .vsync = true,
        .msaa_samples = 4,
        .clear_color = {0.1f, 0.1f, 0.1f, 1.0f}
    };
    cf_renderer_t* renderer;
    cf_renderer_init(&renderer_config, &renderer);
    
    // 5. 设置模型到渲染器
    cf_renderer_set_model(renderer, model);
    
    // 6. 渲染循环
    while (!cf_renderer_should_close(renderer)) {
        cf_renderer_begin_frame(renderer);
        cf_renderer_render(renderer);
        cf_renderer_end_frame(renderer);
    }
    
    // 7. 清理资源
    cf_renderer_destroy(renderer);
    cf_model_destroy(model);
    cf_heightmap_destroy(heightmap);
    
    return 0;
}
```

### 交互编辑

支持节点选择和编辑：

```c
#include <contourforge/contourforge.h>

// 创建编辑器
cf_editor_t* editor;
cf_editor_create(model, 100, &editor);  // 最多100层撤销

// 创建选择器
cf_selector_t* selector;
cf_selector_create(model, renderer, &selector);

// 在鼠标点击位置选择节点
cf_index_t selected_point;
if (cf_selector_pick_point(selector, mouse_x, mouse_y, 5.0f, &selected_point) == CF_SUCCESS) {
    printf("选中节点: %u\n", selected_point);
    
    // 移动节点到新位置
    cf_point3_t new_pos = {x, y, z};
    cf_editor_move_point(editor, selected_point, new_pos);
}

// 在线段上插入新节点
cf_index_t new_point;
cf_editor_insert_point_on_line(editor, line_index, 0.5f, &new_point);

// 撤销/重做
if (cf_editor_can_undo(editor)) {
    cf_editor_undo(editor);
}
if (cf_editor_can_redo(editor)) {
    cf_editor_redo(editor);
}

// 清理
cf_selector_destroy(selector);
cf_editor_destroy(editor);
```

### 相机控制

```c
// 获取渲染器的相机
cf_camera_t* camera = cf_renderer_get_camera(renderer);

// 设置透视投影
cf_camera_set_perspective(camera, 45.0f, 16.0f/9.0f, 0.1f, 1000.0f);

// 设置相机位置和目标
cf_point3_t position = {10.0f, 10.0f, 10.0f};
cf_point3_t target = {0.0f, 0.0f, 0.0f};
cf_camera_set_position(camera, position);
cf_camera_set_target(camera, target);

// 旋转相机（轨道控制）
cf_camera_orbit(camera, delta_yaw, delta_pitch);

// 缩放
cf_camera_zoom(camera, zoom_delta);
```

更多示例请查看 [`examples/`](examples/) 目录：
- [`simple_viewer.c`](examples/simple_viewer.c) - 基础3D查看器
- [`heightmap_loader.c`](examples/heightmap_loader.c) - 高度图加载和等高线生成
- [`interactive_editor.c`](examples/interactive_editor.c) - 完整的交互式编辑器

---

## 📦 项目结构

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
│   ├── core/                  # 核心模块（内存、数据结构、八叉树）
│   ├── rendering/             # 渲染模块（OpenGL、相机、着色器）
│   ├── datagen/               # 数据生成（高度图、等高线、简化）
│   └── control/               # 控制模块（输入、选择、编辑）
├── shaders/                   # GLSL着色器
│   ├── basic.vert/frag        # 基础着色器
│   └── line.vert/frag         # 线段着色器
├── tests/                     # 单元测试
├── examples/                  # 示例程序
├── data/                      # 测试数据
│   └── heightmaps/            # 高度图样本
├── third_party/               # 第三方依赖
│   ├── glfw/                  # 窗口管理
│   └── glad/                  # OpenGL加载器
├── docs/                      # 文档
└── plans/                     # 设计文档
```

---

## 🏗️ 架构设计

### 模块架构

```
┌─────────────────────────────────────┐
│      应用层（示例程序/测试）         │
└─────────────────────────────────────┘
           ↓              ↓
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

### 核心技术栈

| 组件 | 技术 | 版本 |
|------|------|------|
| **编程语言** | C | C11 |
| **图形API** | OpenGL | 3.3 Core |
| **窗口管理** | GLFW | 3.3+ |
| **数学库** | cglm | 0.8.0+ |
| **图像加载** | stb_image | 单头文件 |
| **OpenGL加载** | glad | 3.3 Core |
| **构建系统** | CMake | 3.15+ |

### 数据流

```
灰度图PNG → 高度图加载 → 等高线提取 → 线段简化 → 拓扑构建
                                                    ↓
                                              模型数据
                                            ↙        ↘
                                    八叉树索引    GPU缓冲
                                         ↓            ↓
                                    视锥剔除 → OpenGL渲染
```

详细架构设计请查看 [`ARCHITECTURE.md`](ARCHITECTURE.md)

---

## 📊 性能指标

### 渲染性能

| 指标 | 目标值 | 实际值 | 状态 |
|------|--------|--------|------|
| 节点规模 | 1000万+ | 1000万 | ✅ |
| 帧率（1000万节点） | 60 FPS | 60+ FPS | ✅ |
| 内存占用 | <4GB | ~1GB | ✅ |
| 加载时间 | <5秒 | ~3秒 | ✅ |

### 算法性能

| 操作 | 复杂度 | 说明 |
|------|--------|------|
| 等高线提取 | O(n) | Marching Squares算法 |
| 线段简化 | O(n log n) | Douglas-Peucker算法 |
| 空间查询 | O(log n) | 八叉树索引 |
| 节点选择 | O(log n) | 射线投射+八叉树 |

*测试环境: Intel i7-10700K, RTX 3070, 32GB RAM, Windows 11*

详细性能测试报告请查看 [`docs/PERFORMANCE.md`](docs/PERFORMANCE.md)

---

## 🧪 测试

### 运行测试

```bash
# 进入构建目录
cd build

# 运行所有测试
ctest

# 运行特定测试
ctest -R test_memory
ctest -R test_octree
ctest -R test_contour

# 详细输出
ctest -V

# 并行运行测试
ctest -j8
```

### 测试覆盖

- ✅ 内存管理测试（内存池、对象池）
- ✅ 数据结构测试（点集、线集、模型）
- ✅ 八叉树测试（构建、查询、剔除）
- ✅ 等高线生成测试（Marching Squares）
- ✅ 线段简化测试（Douglas-Peucker）
- ✅ 拓扑构建测试

---

## 📖 文档

### 用户文档

- [**快速开始**](docs/USER_GUIDE.md#快速开始) - 5分钟上手指南
- [**用户指南**](docs/USER_GUIDE.md) - 完整使用教程
- [**API参考**](docs/API.md) - 详细的API文档
- [**构建指南**](BUILD.md) - 编译和安装说明
- [**常见问题**](docs/USER_GUIDE.md#常见问题) - FAQ

### 开发者文档

- [**架构设计**](ARCHITECTURE.md) - 系统架构和设计决策
- [**开发者指南**](docs/DEVELOPER_GUIDE.md) - 如何参与开发
- [**贡献指南**](CONTRIBUTING.md) - 代码规范和提交流程
- [**性能优化**](docs/PERFORMANCE.md) - 性能分析和优化技巧
- [**变更日志**](CHANGELOG.md) - 版本历史

### 设计文档

- [需求分析](plans/01-requirements-analysis.md)
- [技术选型](plans/02-technology-stack.md)
- [数据结构设计](plans/03-data-structures.md)
- [API接口设计](plans/04-api-design.md)
- [性能优化策略](plans/05-performance-optimization.md)
- [构建系统设计](plans/06-build-system.md)

---

## 🤝 贡献

欢迎贡献代码、报告问题或提出建议！我们重视每一个贡献者的参与。

### 如何贡献

1. **Fork** 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 **Pull Request**

### 贡献类型

- 🐛 报告Bug
- 💡 提出新功能
- 📝 改进文档
- 🎨 优化代码
- ✅ 添加测试
- 🌍 翻译文档

### 代码规范

- 遵循C11标准
- 使用项目命名约定（`cf_<module>_<action>`）
- 添加必要的注释和文档
- 编写单元测试
- 确保所有测试通过

详细贡献指南请查看 [`CONTRIBUTING.md`](CONTRIBUTING.md)

---

## 📋 开发路线图

### v0.1.0（当前版本）✅

- [x] 基础架构设计
- [x] 核心库实现（内存管理、数据结构、八叉树）
- [x] 渲染库实现（OpenGL、相机、着色器）
- [x] 数据生成库实现（高度图、等高线、简化）
- [x] 控制库实现（输入、选择、编辑）
- [x] 示例程序
- [x] 单元测试
- [x] 文档完善

### v0.2.0（计划中）

- [ ] 性能优化（SIMD、多线程）
- [ ] LOD（层次细节）系统
- [ ] 更多简化算法
- [ ] 纹理支持
- [ ] 导出功能（OBJ、STL）

### v1.0.0（未来）

- [ ] 完整的GUI编辑器
- [ ] 插件系统
- [ ] Python绑定
- [ ] 云端渲染支持
- [ ] 移动平台支持

详细路线图请查看 [ARCHITECTURE.md](ARCHITECTURE.md#9-开发路线图)

---

## 📄 许可证

本项目采用 **GNU Affero General Public License v3.0 (AGPL-3.0)** 许可证。

### 许可证要点

- ✅ **自由使用**: 您可以自由使用、修改和分发本软件
- ✅ **商业使用**: 允许用于商业目的
- ⚠️ **网络服务**: 如果您修改了本软件并通过网络提供服务，必须公开修改后的源代码
- ⚠️ **Copyleft**: 任何基于本软件的衍生作品也必须使用AGPL-3.0许可证
- ⚠️ **无担保**: 软件按"原样"提供，不提供任何明示或暗示的担保

详细许可证条款请参见 [`LICENSE`](LICENSE) 文件。

### 第三方依赖许可证

| 库 | 许可证 | 说明 |
|---|--------|------|
| GLFW | zlib/libpng | 宽松许可证 |
| glad | MIT/Public Domain | 宽松许可证 |
| cglm | MIT | 宽松许可证 |
| stb_image | MIT/Public Domain | 宽松许可证 |

详见 [`third_party/README.md`](third_party/README.md)

---

## 🙏 致谢

感谢以下开源项目和社区：

- [**GLFW**](https://www.glfw.org/) - 优秀的窗口和输入管理库
- [**cglm**](https://github.com/recp/cglm) - 高性能的C语言数学库
- [**stb**](https://github.com/nothings/stb) - 实用的单头文件库集合
- [**glad**](https://glad.dav1d.de/) - OpenGL加载器生成器
- [**OpenGL**](https://www.opengl.org/) - 跨平台图形API

特别感谢所有贡献者和用户的支持！

---

## 📧 联系方式

- **项目主页**: https://github.com/username/contourforge
- **问题反馈**: https://github.com/username/contourforge/issues
- **功能建议**: https://github.com/username/contourforge/discussions
- **邮件**: contourforge@example.com

### 社区

- **Discord**: [加入我们的Discord服务器](#)
- **论坛**: [访问社区论坛](#)
- **Wiki**: [查看项目Wiki](#)

---

## ⭐ Star History

如果这个项目对你有帮助，请给我们一个Star！⭐

[![Star History Chart](https://api.star-history.com/svg?repos=username/contourforge&type=Date)](https://star-history.com/#username/contourforge&Date)

---

## 📈 项目统计

![GitHub stars](https://img.shields.io/github/stars/username/contourforge?style=social)
![GitHub forks](https://img.shields.io/github/forks/username/contourforge?style=social)
![GitHub watchers](https://img.shields.io/github/watchers/username/contourforge?style=social)
![GitHub contributors](https://img.shields.io/github/contributors/username/contourforge)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/username/contourforge)
![GitHub last commit](https://img.shields.io/github/last-commit/username/contourforge)

---

<div align="center">

**Made with ❤️ by Contourforge Team**

[⬆ 回到顶部](#contourforge)

</div>
