# Changelog

本文档记录Contourforge项目的所有重要变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

---

## [Unreleased]

### 计划中
- TIFF压缩支持（LZW、Deflate）
- GeoTIFF完整元数据解析和坐标系统支持
- PNG/TIFF格式导出
- HGT/SRTM格式支持
- 平滑LOD过渡（alpha混合）
- Python/JavaScript绑定
- GUI编辑器

---

## [0.3.0] - 2026-05-01

### 版本亮点
- 📁 多格式支持：TIFF、GeoTIFF识别、RAW等专业地理数据格式。
- 🏷️ 等高线标注：3D场景中显示高度值，支持TrueType字体和Billboard效果。
- 📊 完善测试：新增LOD、线程池、标注、格式加载测试，Windows Debug测试100%通过。

### 新增

#### 多格式支持
- 新增 `cf_heightmap_detect_format()` 自动检测高度图格式。
- 新增 `cf_heightmap_format_name()` 获取格式名称。
- 新增 `cf_heightmap_load_tiff()` 加载8/16/32位灰度TIFF高度图。
- 新增 `cf_heightmap_load_raw()` 加载RAW高度数据。
- 新增 `cf_heightmap_format_t`，包含PNG、JPEG、BMP、TIFF、GeoTIFF和RAW枚举值。
- 新增 `cf_raw_format_t`，支持U8、U16、I16、U32、I32、F32六种RAW数据类型。
- 新增 `cf_geo_metadata_t` 作为GeoTIFF地理元数据接口预留。
- 新增 `format_converter` 示例工具，用于格式信息查看和RAW转换。

#### 等高线标注系统
- 新增 `cf_font_load()` 和 `cf_font_destroy()` 管理TrueType字体。
- 新增 `cf_text_renderer_create()`、`cf_text_renderer_render_3d()`、`cf_text_renderer_measure_width()` 和 `cf_text_renderer_destroy()`。
- 新增 `cf_label_manager_create()`、`cf_label_manager_generate_labels()`、`cf_label_manager_update()`、`cf_label_manager_render()`、`cf_label_manager_clear()` 和 `cf_label_manager_destroy()`。
- 新增 `cf_place_labels_on_contour()` 标注放置算法API。
- 新增 `cf_calculate_label_spacing_lod()` 根据相机距离计算标注LOD间距。
- 新增 `cf_filter_labels_by_distance()` 标注可见距离过滤。
- 新增 `cf_label_config_t`，支持间距、显示距离、单位、小数位、颜色和字号配置。
- 新增 `label_demo` 示例程序。

#### 测试
- 新增 `test_lod.c`：LOD创建销毁、层级选择、统计信息、并行LOD生成。
- 新增 `test_threading.c`：线程池创建销毁、任务提交执行、并发安全性、性能验证。
- 新增 `test_label.c`：字体加载验证、标注生成、标注放置、碰撞/距离过滤。
- 扩展 `test_formats.c`：格式检测、RAW加载、参数验证、多数据类型加载。
- 修复 `test_datagen.c` 缺失标准库头导致的Windows运行崩溃。

#### 文档
- 更新 `README.md`，加入v0.3.0功能、快速开始和新示例。
- 更新 `docs/API.md`，加入格式支持API和标注系统API。
- 完善 `RELEASE_NOTES_v0.3.0.md`。
- 更新 `PROJECT_COMPLETION_REPORT.md`。

### 改进
- `cf_heightmap_load()` 支持自动分派标准图像和TIFF加载器。
- 标注放置逻辑改为基于 `cf_line_set_t` 的点索引结构，兼容核心线集实现。
- `include/contourforge/rendering.h` 公开标注放置、LOD间距和距离过滤API。
- `include/contourforge/contourforge.h` 版本宏更新为0.3.0。
- `src/core/version.c` 运行时版本更新为0.3.0。

### API变更

#### 新增API
- `cf_heightmap_detect_format()`
- `cf_heightmap_format_name()`
- `cf_heightmap_load_tiff()`
- `cf_heightmap_load_raw()`
- `cf_font_load()`
- `cf_font_destroy()`
- `cf_text_renderer_create()`
- `cf_text_renderer_render_3d()`
- `cf_text_renderer_measure_width()`
- `cf_text_renderer_destroy()`
- `cf_label_manager_create()`
- `cf_label_manager_generate_labels()`
- `cf_label_manager_update()`
- `cf_label_manager_render()`
- `cf_label_manager_clear()`
- `cf_label_manager_destroy()`
- `cf_place_labels_on_contour()`
- `cf_calculate_label_spacing_lod()`
- `cf_filter_labels_by_distance()`

#### 废弃API
- 无

### 已知问题
- 标注在极近距离或极高密度线段场景可能重叠。
- TIFF压缩格式支持有限，推荐使用未压缩灰度TIFF。
- GeoTIFF当前仅识别格式，暂不解析完整投影和坐标系统。
- OpenGL相关字体/文字/标注渲染API必须在有效OpenGL上下文中调用。

### 兼容性
- 完全向后兼容v0.2.0 API。
- 支持Windows、Linux、macOS。

---

## [0.2.0] - 2026-05-01

### 新增
- LOD系统：多级细节层次、自动距离选择、均匀采样、重要性采样、边界保护和统计信息API。
- 多线程系统：跨平台线程池、任务队列、并行等高线提取、并行LOD生成、并行线段简化。
- 渲染器LOD集成：LOD模型渲染、自动LOD、LOD调试模式。
- 示例：`lod_demo`、`threading_benchmark`。
- 文档：LOD系统、多线程系统、实现报告、发布说明。

### 改进
- 大规模数据渲染性能提升2-5倍。
- 数据处理阶段多线程加速2-4倍。
- 相机控制和模型辅助函数增强。

### 已知问题
- 小数据集多线程开销可能大于收益。
- 极端视角下LOD选择可能不够精确。
- OpenGL调用仍需在主线程执行。

---

## [0.1.0] - 2026-04-30

### 新增
- 核心库：内存池、点集、线集、模型、八叉树和边界盒。
- 数据生成库：高度图加载、Marching Squares等高线提取、Douglas-Peucker简化和拓扑构建。
- 渲染库：OpenGL 3.3渲染器、着色器、相机、缓冲管理。
- 控制库：输入、选择、编辑、撤销/重做。
- 示例：`simple_viewer`、`heightmap_loader`、`interactive_editor`。
- 测试：核心、内存、八叉树、数据生成、等高线、简化测试。

---

## [0.0.1] - 2026-04-15

### 新增
- 项目初始化。
- 基础目录结构。
- 初始设计文档。
- CMake构建系统框架。

---

## 链接

- 项目主页：https://github.com/czxieddan/contourforge
- 发布页面：https://github.com/czxieddan/contourforge/releases
- 问题追踪：https://github.com/czxieddan/contourforge/issues

**最后更新**: 2026-05-01
