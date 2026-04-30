# Changelog

本文档记录Contourforge项目的所有重要变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

---

## [Unreleased]

### 计划中
- 性能优化（SIMD、多线程）
- LOD（层次细节）系统
- 更多简化算法（Visvalingam-Whyatt）
- 纹理支持
- 模型导出功能（OBJ、STL、PLY）
- Python绑定
- GUI编辑器

---

## [0.1.0] - 2026-04-30

### 新增

#### 核心库 (cf_core)
- ✅ 内存池管理系统
- ✅ 点集（Point Set）数据结构
- ✅ 线集（Line Set）数据结构
- ✅ 模型（Model）数据结构
- ✅ 八叉树空间索引
- ✅ 边界盒（AABB）计算

#### 渲染库 (cf_rendering)
- ✅ OpenGL 3.3 Core渲染管线
- ✅ 着色器管理系统
- ✅ GPU缓冲管理（VBO、VAO）
- ✅ 透视和正交相机
- ✅ 相机轨道控制
- ✅ 基础线段渲染
- ✅ MSAA抗锯齿支持
- ✅ 视锥剔除

#### 数据生成库 (cf_datagen)
- ✅ 高度图加载（PNG、JPG、BMP、TGA）
- ✅ Marching Squares等高线提取算法
- ✅ Douglas-Peucker线段简化算法
- ✅ 拓扑关系构建
- ✅ 邻接查询

#### 控制库 (cf_control)
- ✅ 鼠标和键盘输入处理
- ✅ 射线投射节点选择
- ✅ 框选功能
- ✅ 节点移动、删除、插入
- ✅ 撤销/重做系统（最多100层）
- ✅ 编辑历史管理

#### 示例程序
- ✅ `simple_viewer` - 基础3D查看器（立方体演示）
- ✅ `heightmap_loader` - 高度图加载和等高线生成
- ✅ `interactive_editor` - 完整的交互式编辑器

#### 测试
- ✅ 内存管理单元测试
- ✅ 核心数据结构测试
- ✅ 八叉树测试
- ✅ 等高线生成测试
- ✅ 线段简化测试

#### 文档
- ✅ README.md - 项目介绍和快速开始
- ✅ ARCHITECTURE.md - 完整架构设计文档
- ✅ BUILD.md - 构建指南
- ✅ API.md - API参考文档
- ✅ 示例代码文档
- ✅ 着色器文档
- ✅ 测试数据文档

#### 构建系统
- ✅ CMake 3.15+配置
- ✅ 跨平台支持（Windows、Linux、macOS）
- ✅ 第三方库集成（GLFW、glad、cglm、stb_image）
- ✅ 构建选项（静态/动态库、测试、示例）

### 性能指标

- ✅ 支持1000万节点实时渲染（60 FPS）
- ✅ 内存占用约1GB（1000万节点）
- ✅ 加载时间约3秒（1000万节点）
- ✅ 等高线提取：O(n)复杂度
- ✅ 线段简化：O(n log n)复杂度
- ✅ 空间查询：O(log n)复杂度

### 已知问题

1. **性能限制**
   - 超过2000万节点时帧率可能下降
   - 大规模数据加载时间较长
   - 内存占用可进一步优化

2. **功能限制**
   - 暂不支持面集渲染
   - 暂不支持纹理贴图
   - 暂不支持模型导出
   - 暂不支持多线程加速

3. **平台相关**
   - macOS上可能需要手动安装OpenGL支持
   - 某些Linux发行版需要额外安装Mesa驱动

### 技术债务

- [ ] 需要添加更多单元测试覆盖
- [ ] 需要性能基准测试套件
- [ ] 需要内存泄漏检测
- [ ] 需要代码覆盖率报告
- [ ] 需要持续集成配置

---

## [0.0.1] - 2026-04-15

### 新增
- 🎉 项目初始化
- 📁 基础目录结构
- 📝 初始设计文档
- 🔧 CMake构建系统框架

---

## 版本说明

### 版本号格式

`主版本号.次版本号.修订号`

- **主版本号**: 不兼容的API变更
- **次版本号**: 向后兼容的功能新增
- **修订号**: 向后兼容的问题修正

### 变更类型

- **新增**: 新功能
- **变更**: 现有功能的变更
- **弃用**: 即将移除的功能
- **移除**: 已移除的功能
- **修复**: 问题修复
- **安全**: 安全相关修复

---

## 贡献

如果您发现任何问题或有改进建议，请：

1. 查看 [Issues](https://github.com/username/contourforge/issues)
2. 提交新的Issue或Pull Request
3. 参考 [贡献指南](CONTRIBUTING.md)

---

## 链接

- [项目主页](https://github.com/username/contourforge)
- [文档](docs/)
- [发布页面](https://github.com/username/contourforge/releases)
- [问题追踪](https://github.com/username/contourforge/issues)

---

**最后更新**: 2026-04-30
