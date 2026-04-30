# Contourforge 构建指南

本文档提供Contourforge项目的详细构建说明。

---

## 目录

1. [前置要求](#前置要求)
2. [快速开始](#快速开始)
3. [详细构建步骤](#详细构建步骤)
4. [构建选项](#构建选项)
5. [平台特定说明](#平台特定说明)
6. [故障排除](#故障排除)
7. [高级配置](#高级配置)

---

## 前置要求

### 必需工具

| 工具 | 最低版本 | 推荐版本 | 说明 |
|------|---------|---------|------|
| **CMake** | 3.15 | 3.25+ | 构建系统 |
| **C编译器** | C11 | - | 见下表 |
| **Git** | 2.0+ | 最新 | 版本控制 |

### C编译器要求

| 平台 | 编译器 | 最低版本 |
|------|--------|---------|
| **Windows** | MSVC | 2019 (19.20+) |
| | MinGW-w64 | GCC 7+ |
| | Clang | 6+ |
| **Linux** | GCC | 7+ |
| | Clang | 6+ |
| **macOS** | Apple Clang | Xcode 10+ |

### 依赖库

Contourforge依赖以下第三方库（已包含在`third_party/`目录）：

| 库 | 版本 | 用途 | 许可证 |
|---|------|------|--------|
| **GLFW** | 3.3+ | 窗口管理 | zlib/libpng |
| **glad** | OpenGL 3.3 | OpenGL加载器 | MIT/Public Domain |
| **cglm** | 0.8.0+ | 数学库 | MIT |
| **stb_image** | 单头文件 | 图像加载 | MIT/Public Domain |

**注意**: 第三方库通过Git子模块管理，克隆时需要使用`--recursive`选项。

---

## 快速开始

### Windows (Visual Studio)

```bash
# 1. 克隆仓库（包含子模块）
git clone --recursive https://github.com/username/contourforge.git
cd contourforge

# 2. 生成Visual Studio项目
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# 3. 编译
cmake --build . --config Release

# 4. 运行示例
.\bin\Release\simple_viewer.exe
```

### Linux

```bash
# 1. 安装依赖
sudo apt install build-essential cmake libgl1-mesa-dev

# 2. 克隆仓库
git clone --recursive https://github.com/username/contourforge.git
cd contourforge

# 3. 编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. 运行示例
./bin/simple_viewer
```

### macOS

```bash
# 1. 安装依赖
brew install cmake

# 2. 克隆仓库
git clone --recursive https://github.com/username/contourforge.git
cd contourforge

# 3. 编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)

# 4. 运行示例
./bin/simple_viewer
```

---

## 详细构建步骤

### 1. 获取源代码

```bash
# 克隆主仓库和子模块
git clone --recursive https://github.com/username/contourforge.git

# 如果已经克隆但没有子模块
cd contourforge
git submodule update --init --recursive
```

### 2. 创建构建目录

```bash
mkdir build
cd build
```

**最佳实践**: 使用独立的构建目录（out-of-source build）保持源代码目录整洁。

### 3. 配置CMake

#### 基础配置

```bash
# Release构建（推荐）
cmake .. -DCMAKE_BUILD_TYPE=Release

# Debug构建（开发调试）
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 指定生成器（Windows）
cmake .. -G "Visual Studio 17 2022" -A x64
cmake .. -G "MinGW Makefiles"
cmake .. -G "Ninja"
```

#### 自定义选项

```bash
# 构建静态库
cmake .. -DCF_BUILD_SHARED=OFF

# 禁用示例和测试
cmake .. -DCF_BUILD_EXAMPLES=OFF -DCF_BUILD_TESTS=OFF

# 启用所有优化
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCF_ENABLE_SIMD=ON \
         -DCF_ENABLE_OPENMP=ON
```

### 4. 编译项目

```bash
# 跨平台命令
cmake --build . --config Release

# 或使用特定构建工具
make -j8          # Linux/macOS (Make)
ninja             # Ninja
msbuild           # Windows (MSBuild)
```

### 5. 运行测试

```bash
# 运行所有测试
ctest

# 详细输出
ctest -V

# 运行特定测试
ctest -R test_memory

# 并行运行
ctest -j8
```

### 6. 安装（可选）

```bash
# Linux/macOS
sudo cmake --install .

# Windows（以管理员身份）
cmake --install . --prefix "C:/Program Files/Contourforge"

# 自定义安装路径
cmake --install . --prefix /opt/contourforge
```

---

## 构建选项

### CMake选项列表

| 选项 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `CF_BUILD_SHARED` | BOOL | ON | 构建动态库（OFF=静态库） |
| `CF_BUILD_TESTS` | BOOL | ON | 构建单元测试 |
| `CF_BUILD_EXAMPLES` | BOOL | ON | 构建示例程序 |
| `CF_BUILD_DOCS` | BOOL | OFF | 生成API文档（需要Doxygen） |
| `CF_ENABLE_SIMD` | BOOL | ON | 启用SIMD优化（AVX2） |
| `CF_ENABLE_OPENMP` | BOOL | ON | 启用OpenMP多线程 |
| `CMAKE_BUILD_TYPE` | STRING | Release | 构建类型（Debug/Release/RelWithDebInfo） |
| `CMAKE_INSTALL_PREFIX` | PATH | 系统默认 | 安装路径 |

### 使用示例

```bash
# 最小构建（仅核心库）
cmake .. -DCF_BUILD_SHARED=OFF \
         -DCF_BUILD_TESTS=OFF \
         -DCF_BUILD_EXAMPLES=OFF

# 开发构建（Debug+测试）
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCF_BUILD_TESTS=ON

# 性能优化构建
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCF_ENABLE_SIMD=ON \
         -DCF_ENABLE_OPENMP=ON \
         -DCMAKE_C_FLAGS="-O3 -march=native -flto"

# 文档生成
cmake .. -DCF_BUILD_DOCS=ON
```

---

## 平台特定说明

### Windows

#### Visual Studio

```bash
# 生成VS 2022项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 生成VS 2019项目
cmake .. -G "Visual Studio 16 2019" -A x64

# 在IDE中打开
start Contourforge.sln

# 或命令行编译
cmake --build . --config Release
```

#### MinGW

```bash
# 确保MinGW在PATH中
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j8
```

#### MSYS2

```bash
# 安装依赖
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake

# 构建
cmake .. -G "MSYS Makefiles"
make -j8
```

### Linux

#### Ubuntu/Debian

```bash
# 安装依赖
sudo apt update
sudo apt install build-essential cmake libgl1-mesa-dev

# 构建
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Fedora/RHEL

```bash
# 安装依赖
sudo dnf install gcc cmake mesa-libGL-devel

# 构建
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Arch Linux

```bash
# 安装依赖
sudo pacman -S base-devel cmake mesa

# 构建
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### macOS

```bash
# 安装Xcode Command Line Tools
xcode-select --install

# 安装Homebrew（如果没有）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装CMake
brew install cmake

# 构建
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

**注意**: macOS上OpenGL已被标记为deprecated，但仍然可用。未来版本可能迁移到Metal。

---

## 故障排除

### 常见问题

#### 1. CMake找不到OpenGL

**Windows**:
```bash
# OpenGL通常包含在Windows SDK中
# 确保安装了Windows SDK
```

**Linux**:
```bash
sudo apt install libgl1-mesa-dev  # Ubuntu/Debian
sudo dnf install mesa-libGL-devel # Fedora
```

**macOS**:
```bash
# OpenGL已包含在系统中
# 如果仍有问题，重新安装Xcode Command Line Tools
xcode-select --install
```

#### 2. 子模块未初始化

```bash
# 初始化所有子模块
git submodule update --init --recursive

# 或重新克隆
git clone --recursive https://github.com/username/contourforge.git
```

#### 3. 编译错误：找不到GLFW

```bash
# 检查子模块状态
git submodule status

# 如果显示"-"前缀，说明未初始化
git submodule update --init third_party/glfw
```

#### 4. 链接错误：未定义的引用

这通常是因为源文件尚未完全实现。当前版本的某些函数可能是占位实现。

#### 5. SIMD编译错误

```bash
# 如果CPU不支持AVX2，禁用SIMD
cmake .. -DCF_ENABLE_SIMD=OFF
```

#### 6. OpenMP错误

```bash
# 禁用OpenMP
cmake .. -DCF_ENABLE_OPENMP=OFF
```

### 调试构建问题

```bash
# 详细输出
cmake .. --debug-output

# 查看CMake变量
cmake .. -LAH

# 清理构建
rm -rf build/*
cmake ..
```

---

## 高级配置

### 交叉编译

#### Windows → Linux (WSL)

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/linux-toolchain.cmake
```

#### ARM架构

```bash
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
         -DCMAKE_SYSTEM_NAME=Linux \
         -DCMAKE_SYSTEM_PROCESSOR=arm
```

### 自定义编译器

```bash
# 使用Clang
cmake .. -DCMAKE_C_COMPILER=clang

# 使用特定版本的GCC
cmake .. -DCMAKE_C_COMPILER=gcc-11
```

### 自定义编译标志

```bash
# 添加额外的编译标志
cmake .. -DCMAKE_C_FLAGS="-Wall -Wextra -Werror"

# 添加链接标志
cmake .. -DCMAKE_EXE_LINKER_FLAGS="-static"
```

### 使用Ninja构建

```bash
# 安装Ninja
sudo apt install ninja-build  # Linux
brew install ninja            # macOS

# 使用Ninja生成器
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

### 生成编译数据库

```bash
# 用于IDE和静态分析工具
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 生成的compile_commands.json可用于：
# - clangd
# - clang-tidy
# - Visual Studio Code
```

### 静态分析

```bash
# 使用clang-tidy
cmake .. -DCMAKE_C_CLANG_TIDY="clang-tidy;-checks=*"

# 使用cppcheck
cppcheck --enable=all src/
```

### 代码覆盖率

```bash
# 编译时启用覆盖率
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_C_FLAGS="--coverage"

# 运行测试
ctest

# 生成报告
gcov src/**/*.c
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

### 生成安装包

```bash
# 配置CPack
cmake .. -DCPACK_GENERATOR="ZIP;TGZ"

# 生成包
cpack

# 生成的包：
# - Contourforge-1.0.0-win64.zip
# - Contourforge-1.0.0-Linux.tar.gz
```

---

## 外部项目集成

### 使用find_package

```cmake
# 在您的CMakeLists.txt中
find_package(Contourforge REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app
    Contourforge::cf_core
    Contourforge::cf_rendering
    Contourforge::cf_datagen
)
```

### 使用FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    contourforge
    GIT_REPOSITORY https://github.com/username/contourforge.git
    GIT_TAG v0.1.0
)

FetchContent_MakeAvailable(contourforge)

target_link_libraries(my_app contourforge_core)
```

### 使用add_subdirectory

```cmake
# 将Contourforge作为子目录
add_subdirectory(third_party/contourforge)

target_link_libraries(my_app contourforge_core)
```

---

## 构建输出

### 目录结构

```
build/
├── bin/                    # 可执行文件
│   ├── simple_viewer
│   ├── heightmap_loader
│   ├── interactive_editor
│   ├── test_core
│   └── ...
├── lib/                    # 库文件
│   ├── libcontourforge_core.so
│   ├── libcontourforge_rendering.so
│   ├── libcontourforge_datagen.so
│   └── libcontourforge_control.so
└── CMakeFiles/             # CMake生成的文件
```

### 安装目录结构

```
/usr/local/  (或自定义路径)
├── include/contourforge/
│   ├── contourforge.h
│   ├── types.h
│   ├── core.h
│   ├── rendering.h
│   ├── datagen.h
│   └── control.h
├── lib/
│   ├── libcontourforge_core.so
│   ├── libcontourforge_rendering.so
│   ├── libcontourforge_datagen.so
│   ├── libcontourforge_control.so
│   └── cmake/Contourforge/
└── share/contourforge/
    ├── shaders/
    └── examples/
```

---

## 性能优化构建

### 最大性能配置

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCF_ENABLE_SIMD=ON \
  -DCF_ENABLE_OPENMP=ON \
  -DCMAKE_C_FLAGS="-O3 -march=native -flto -ffast-math"
```

### 最小体积配置

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCF_BUILD_SHARED=OFF \
  -DCMAKE_C_FLAGS="-Os -flto"
```

---

## 持续集成

### GitHub Actions（计划中）

```yaml
# .github/workflows/build.yml
name: Build
on: [push, pull_request]
jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    steps:
      - uses: actions/checkout@v2
        with:
          submodules: recursive
      - name: Build
        run: |
          mkdir build && cd build
          cmake .. -DCMAKE_BUILD_TYPE=Release
          cmake --build .
      - name: Test
        run: cd build && ctest
```

---

## 更多资源

- [架构文档](ARCHITECTURE.md) - 系统设计
- [API文档](docs/API.md) - API参考
- [用户指南](docs/USER_GUIDE.md) - 使用教程
- [开发者指南](docs/DEVELOPER_GUIDE.md) - 开发指南
- [贡献指南](CONTRIBUTING.md) - 如何贡献
- [第三方依赖](third_party/README.md) - 依赖说明

---

## 获取帮助

遇到构建问题？

- 📖 查看 [故障排除](#故障排除) 部分
- 🐛 在 [Issues](https://github.com/username/contourforge/issues) 报告问题
- 💬 在 [Discussions](https://github.com/username/contourforge/discussions) 提问
- 📧 发送邮件到 contourforge@example.com

---

**最后更新**: 2026-04-30
**文档版本**: 1.0.0
