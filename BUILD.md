# Contourforge 构建指南

本文档说明如何构建Contourforge项目。

## 前置要求

### 必需工具
- **CMake** 3.15 或更高版本
- **C编译器**（支持C11标准）：
  - Windows: MSVC 2019+, MinGW-w64, Clang
  - Linux: GCC 7+, Clang 6+
  - macOS: Clang (Xcode Command Line Tools)

### 依赖库
在构建前，需要安装或配置以下第三方库（详见 [`third_party/README.md`](third_party/README.md)）：

- OpenGL 3.3+
- GLFW 3.3+
- cglm 0.8.0+
- glad (OpenGL 3.3 Core)
- stb_image (单头文件)

**注意**：当前版本的CMakeLists.txt中第三方库集成部分已注释，需要先配置第三方库后才能完整构建。

## 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/username/contourforge.git
cd contourforge
```

### 2. 配置第三方库

参考 [`third_party/README.md`](third_party/README.md) 配置依赖库。

推荐使用Git Submodules：

```bash
# 添加子模块
git submodule add https://github.com/glfw/glfw.git third_party/glfw
git submodule add https://github.com/recp/cglm.git third_party/cglm

# 初始化子模块
git submodule update --init --recursive
```

### 3. 生成构建文件

#### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
```

#### Windows (MinGW)

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
```

#### Linux/macOS

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### 4. 编译

#### Windows (Visual Studio)

```bash
cmake --build . --config Release
```

或者打开生成的 `Contourforge.sln` 文件，在Visual Studio中编译。

#### Linux/macOS

```bash
make -j$(nproc)
```

### 5. 运行示例

```bash
# Windows
bin\Release\simple_viewer.exe data\heightmaps\terrain.png

# Linux/macOS
./bin/simple_viewer data/heightmaps/terrain.png
```

## 构建选项

CMake提供以下构建选项：

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CF_BUILD_SHARED` | ON | 构建动态库（OFF=静态库） |
| `CF_BUILD_TESTS` | ON | 构建测试程序 |
| `CF_BUILD_EXAMPLES` | ON | 构建示例程序 |
| `CF_BUILD_DOCS` | OFF | 生成API文档（需要Doxygen） |
| `CF_ENABLE_SIMD` | ON | 启用SIMD优化（AVX2） |
| `CF_ENABLE_OPENMP` | ON | 启用OpenMP多线程 |

### 使用示例

```bash
# 构建静态库，禁用示例和测试
cmake .. -DCF_BUILD_SHARED=OFF -DCF_BUILD_EXAMPLES=OFF -DCF_BUILD_TESTS=OFF

# 启用文档生成
cmake .. -DCF_BUILD_DOCS=ON

# 禁用SIMD优化（兼容旧CPU）
cmake .. -DCF_ENABLE_SIMD=OFF
```

## 构建类型

### Debug（调试版本）

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

特性：
- 包含调试符号
- 禁用优化
- 启用断言
- 适合开发和调试

### Release（发布版本）

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

特性：
- 完全优化（-O3）
- 无调试符号
- 禁用断言
- 适合生产环境

### RelWithDebInfo（带调试信息的发布版本）

```bash
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

特性：
- 优化代码
- 包含调试符号
- 适合性能分析

## 测试

### 运行所有测试

```bash
cd build
ctest
```

### 运行特定测试

```bash
# Windows
bin\Release\test_core.exe

# Linux/macOS
./bin/test_core
```

### 详细测试输出

```bash
ctest --verbose
```

## 安装

### 系统安装

```bash
# Linux/macOS
sudo make install

# Windows (以管理员身份运行)
cmake --install . --prefix "C:/Program Files/Contourforge"
```

### 自定义安装路径

```bash
cmake --install . --prefix /path/to/install
```

安装后的目录结构：

```
/usr/local/  (或自定义路径)
├── include/contourforge/
│   ├── contourforge.h
│   ├── core.h
│   ├── rendering.h
│   ├── datagen.h
│   └── control.h
├── lib/
│   ├── libcontourforge_core.so
│   ├── libcontourforge_rendering.so
│   ├── libcontourforge_datagen.so
│   └── libcontourforge_control.so
└── share/contourforge/
    └── shaders/
```

## 故障排除

### CMake找不到OpenGL

**Windows**:
```bash
# 通常OpenGL已包含在Windows SDK中
# 确保安装了Windows SDK
```

**Linux**:
```bash
sudo apt install libgl1-mesa-dev
```

**macOS**:
```bash
# OpenGL已包含在系统中，无需额外安装
```

### 编译错误：找不到GLFW

确保已正确配置第三方库，并在CMakeLists.txt中取消注释相关行：

```cmake
# 取消注释这些行
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/cglm)
```

### 链接错误：未定义的引用

这通常是因为源文件尚未实现。当前版本的源文件是占位实现，需要逐步完成实际功能。

### SIMD编译错误

如果CPU不支持AVX2，禁用SIMD：

```bash
cmake .. -DCF_ENABLE_SIMD=OFF
```

## 交叉编译

### Windows → Linux (WSL)

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/linux-toolchain.cmake
```

### 为ARM架构编译

```bash
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc
```

## 清理构建

```bash
# 删除构建目录
rm -rf build

# 或在build目录中
make clean  # Linux/macOS
cmake --build . --target clean  # 跨平台
```

## 生成安装包

```bash
cd build
cpack
```

生成的包：
- Windows: `Contourforge-1.0.0-win64.zip`
- Linux: `Contourforge-1.0.0-Linux.tar.gz`
- macOS: `Contourforge-1.0.0-Darwin.tar.gz`

## 开发工作流

### 1. 修改代码

编辑源文件后，重新编译：

```bash
cd build
cmake --build .
```

### 2. 运行测试

```bash
ctest
```

### 3. 调试

```bash
# 使用调试器
gdb ./bin/test_core  # Linux
lldb ./bin/test_core  # macOS
```

## 持续集成

项目包含GitHub Actions配置（待添加），支持：
- 多平台自动构建（Windows, Linux, macOS）
- 自动运行测试
- 代码覆盖率报告
- 自动发布

## 性能优化构建

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCF_ENABLE_SIMD=ON \
  -DCF_ENABLE_OPENMP=ON \
  -DCMAKE_C_FLAGS="-O3 -march=native -flto"
```

## 更多信息

- [架构文档](ARCHITECTURE.md)
- [API文档](docs/)
- [第三方依赖](third_party/README.md)
- [贡献指南](CONTRIBUTING.md)

## 获取帮助

- 问题反馈: https://github.com/username/contourforge/issues
- 讨论区: https://github.com/username/contourforge/discussions
- 邮件: contourforge@example.com
