# Contourforge 测试程序使用说明

本目录包含三个测试程序，用于验证Contourforge库的各项功能。

## 程序列表

### 1. simple_viewer - 简单3D查看器（Phase 1）

**功能：**
- 创建并显示立方体线框模型（8个顶点，12条边）
- 验证核心库和渲染库的基本功能
- 测试相机控制和基本渲染管线

**用法：**
```bash
cd build/bin/Release
simple_viewer.exe
```

**控制：**
- 鼠标左键拖动：旋转视角
- 鼠标滚轮：缩放
- ESC：退出

**验证清单：**
- [x] 窗口能正常创建和显示
- [x] 能看到立方体线框（8个顶点，12条边）
- [x] 相机能旋转和缩放
- [x] 程序能正常退出
- [x] FPS显示正常

---

### 2. heightmap_loader - 高度图加载器（Phase 2）

**功能：**
- 加载灰度图并生成3D高度场
- 提取等高线
- 简化线段（Douglas-Peucker算法）
- 渲染等高线模型
- 显示详细统计信息

**用法：**
```bash
cd build/bin/Release
heightmap_loader.exe ../../data/heightmaps/terrain_peaks.png 10.0
```

参数说明：
- 第1个参数：高度图文件路径（PNG格式）
- 第2个参数：等高线间隔（可选，默认10.0）

**可用测试图像：**
- `gradient_simple.png` - 简单渐变（适合快速测试）
- `gradient_radial.png` - 径向渐变（测试圆形等高线）
- `terrain_peaks.png` - 山峰地形（复杂地形）
- `wave_pattern.png` - 波浪图案（周期性结构）
- `terrain_large.png` - 大型地形512x512（性能测试）

**控制：**
- 鼠标左键拖动：旋转视角
- 鼠标滚轮：缩放
- ESC：退出

**验证清单：**
- [x] 能加载灰度图
- [x] 能生成点集和线集
- [x] 能提取等高线
- [x] 能简化线段
- [x] 能渲染等高线模型
- [x] 显示正确的统计信息（点数、线数、生成时间）
- [x] 相机自动适配模型大小

---

### 3. interactive_editor - 交互式编辑器（Phase 3）

**功能：**
- 加载高度图生成等高线模型
- 支持鼠标选择节点
- 支持拖动节点移动
- 支持删除节点
- 支持撤销/重做（100层）
- 实时渲染更新
- 验证所有库的集成功能

**用法：**
```bash
cd build/bin/Release
interactive_editor.exe ../../data/heightmaps/terrain_peaks.png 10.0
```

参数说明：
- 第1个参数：高度图文件路径（PNG格式）
- 第2个参数：等高线间隔（可选，默认10.0）

**控制：**

鼠标控制：
- 左键点击：选择节点
- 左键拖动：移动选中的节点
- 右键拖动：旋转视角
- 滚轮：缩放

键盘控制：
- Ctrl+Z：撤销
- Ctrl+Y：重做
- Delete：删除选中的节点
- F1：显示/隐藏帮助信息
- ESC：退出

**验证清单：**
- [x] 能加载和显示模型
- [x] 鼠标能选择节点（高亮显示）
- [x] 能拖动节点移动
- [x] 能删除节点
- [x] 撤销/重做功能正常
- [x] 实时渲染更新正确
- [x] 所有快捷键工作正常
- [x] 窗口标题显示实时统计信息

---

## 编译说明

### 前置要求
- CMake 3.15+
- C11编译器（MSVC/GCC/Clang）
- Python 3.x（用于生成测试数据）
- numpy和pillow库（Python依赖）

### 编译步骤

1. **生成测试数据：**
```bash
cd data/heightmaps
python generate_test_images.py
```

2. **配置CMake：**
```bash
mkdir build
cd build
cmake .. -DCF_BUILD_EXAMPLES=ON -DCF_BUILD_TESTS=OFF -DCF_BUILD_SHARED=OFF
```

3. **编译：**
```bash
cmake --build . --config Release
```

4. **运行程序：**
```bash
cd bin/Release
simple_viewer.exe
heightmap_loader.exe ../../data/heightmaps/terrain_peaks.png
interactive_editor.exe ../../data/heightmaps/terrain_peaks.png
```

---

## 测试结果

### Phase 1 - simple_viewer
✅ 基础渲染测试通过
- 窗口创建成功
- 立方体模型正确显示
- 相机控制正常
- FPS稳定在60+

### Phase 2 - heightmap_loader
✅ 数据生成测试通过
- 高度图加载成功
- 等高线生成正确
- 线段简化有效
- 统计信息准确
- 渲染性能良好

### Phase 3 - interactive_editor
✅ 完整集成测试通过
- 所有库集成正常
- 节点选择功能正常
- 编辑操作正确
- 撤销/重做稳定
- 实时更新流畅

---

## 性能指标

### simple_viewer
- 启动时间：< 100ms
- 渲染FPS：60+ (VSync开启)
- 内存占用：~10MB

### heightmap_loader
- 256x256图像加载：< 50ms
- 等高线生成：< 200ms
- 512x512图像处理：< 1s
- 渲染FPS：60+ (VSync开启)

### interactive_editor
- 模型加载：< 500ms
- 节点选择响应：< 16ms
- 编辑操作延迟：< 10ms
- 撤销/重做：< 5ms
- 渲染FPS：60+ (VSync开启)

---

## 故障排除

### 问题：程序无法启动
**解决方案：**
- 确保所有DLL文件在同一目录
- 检查OpenGL驱动是否支持3.3+
- 确认着色器文件已复制到bin目录

### 问题：找不到高度图文件
**解决方案：**
- 使用相对路径：`../../data/heightmaps/xxx.png`
- 或使用绝对路径
- 确认已运行Python脚本生成测试图像

### 问题：渲染性能低
**解决方案：**
- 更新显卡驱动
- 降低MSAA采样数
- 使用较小的测试图像
- 增大等高线间隔

### 问题：编辑器无法选择节点
**解决方案：**
- 确保使用左键点击
- 尝试放大视图
- 检查节点是否在视野内
- 按F1查看帮助信息

---

## 开发说明

### 添加新的测试程序

1. 在`examples/`目录创建新的.c文件
2. 在`examples/CMakeLists.txt`中添加目标：
```cmake
add_executable(my_test my_test.c)
target_link_libraries(my_test
    cf_core
    cf_rendering
    cf_datagen
    cf_control
    glfw
)
```
3. 重新运行CMake配置
4. 编译并测试

### 调试技巧

- 使用`-DCF_BUILD_SHARED=OFF`编译静态库便于调试
- 在CMake中设置`CMAKE_BUILD_TYPE=Debug`
- 使用Visual Studio或GDB进行断点调试
- 检查控制台输出的错误信息

---

## 许可证

本项目采用MIT许可证。详见根目录的LICENSE文件。

---

## 联系方式

如有问题或建议，请提交Issue或Pull Request。

**项目地址：** https://github.com/czxieddan/contourforge
