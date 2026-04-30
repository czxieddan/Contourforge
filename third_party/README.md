# Contourforge 第三方依赖

本目录包含Contourforge项目所需的第三方库。

## 依赖列表

### 1. GLFW (窗口管理)
- **版本**: 3.3+
- **许可证**: zlib/libpng
- **用途**: 窗口创建、OpenGL上下文管理、输入处理
- **官网**: https://www.glfw.org/

### 2. glad (OpenGL加载器)
- **版本**: OpenGL 3.3 Core
- **许可证**: MIT/Public Domain
- **用途**: 加载OpenGL函数指针
- **官网**: https://glad.dav1d.de/

### 3. cglm (数学库)
- **版本**: 0.9.0+
- **许可证**: MIT
- **用途**: 向量、矩阵运算，SIMD优化
- **官网**: https://github.com/recp/cglm

### 4. stb_image (图像加载)
- **版本**: 单头文件库
- **许可证**: MIT/Public Domain
- **用途**: 加载PNG/JPG等图像格式
- **官网**: https://github.com/nothings/stb

### 5. Unity Test (可选，测试框架)
- **版本**: 2.5.0+
- **许可证**: MIT
- **用途**: C语言单元测试
- **官网**: https://github.com/ThrowTheSwitch/Unity

## 集成方式

### 方式1: Git Submodules（推荐）

```bash
# 添加GLFW
git submodule add https://github.com/glfw/glfw.git third_party/glfw

# 添加cglm
git submodule add https://github.com/recp/cglm.git third_party/cglm

# 添加Unity Test
git submodule add https://github.com/ThrowTheSwitch/Unity.git third_party/unity

# 初始化和更新子模块
git submodule update --init --recursive
```

### 方式2: 手动下载

1. **GLFW**: 从 https://www.glfw.org/download.html 下载源码，解压到 `third_party/glfw/`
2. **cglm**: 从 https://github.com/recp/cglm/releases 下载，解压到 `third_party/cglm/`
3. **stb**: 从 https://github.com/nothings/stb 下载 `stb_image.h`，放到 `third_party/stb/`
4. **Unity**: 从 https://github.com/ThrowTheSwitch/Unity/releases 下载，解压到 `third_party/unity/`

### 方式3: 系统包管理器

#### Windows (vcpkg)
```bash
vcpkg install glfw3:x64-windows
vcpkg install cglm:x64-windows
```

#### Linux (apt)
```bash
sudo apt install libglfw3-dev
sudo apt install libcglm-dev
```

#### macOS (Homebrew)
```bash
brew install glfw
brew install cglm
```

## glad配置

glad需要手动生成，步骤如下：

1. 访问 https://glad.dav1d.de/
2. 配置：
   - Language: C/C++
   - Specification: OpenGL
   - API gl: Version 3.3
   - Profile: Core
3. 勾选 "Generate a loader"
4. 点击 "Generate"
5. 下载生成的文件，解压到 `third_party/glad/`

目录结构应为：
```
third_party/glad/
├── include/
│   ├── glad/
│   │   └── glad.h
│   └── KHR/
│       └── khrplatform.h
└── src/
    └── glad.c
```

## stb_image配置

stb_image是单头文件库，只需下载一个文件：

```bash
# 创建目录
mkdir -p third_party/stb

# 下载stb_image.h
curl -o third_party/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
```

## CMake集成

在使用Git Submodules后，需要在根目录的`CMakeLists.txt`中取消注释以下行：

```cmake
# 添加第三方库
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/cglm)

# 如果构建测试
if(CF_BUILD_TESTS)
    add_subdirectory(third_party/unity)
endif()
```

对于glad，需要手动添加：

```cmake
# glad
add_library(glad third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)
```

## 目录结构

完整的第三方库目录结构：

```
third_party/
├── README.md           # 本文件
├── glfw/               # GLFW源码（子模块）
├── cglm/               # cglm源码（子模块）
├── glad/               # glad生成的文件
│   ├── include/
│   │   ├── glad/
│   │   └── KHR/
│   └── src/
├── stb/                # stb单头文件库
│   └── stb_image.h
└── unity/              # Unity测试框架（子模块，可选）
```

## 许可证兼容性

所有第三方库均使用MIT或更宽松的许可证，与Contourforge的MIT许可证兼容。

## 版本要求

| 库 | 最低版本 | 推荐版本 |
|---|---------|---------|
| GLFW | 3.3.0 | 3.3.8+ |
| cglm | 0.8.0 | 0.9.0+ |
| glad | OpenGL 3.3 | OpenGL 3.3 |
| stb_image | 2.27 | 最新 |
| Unity | 2.5.0 | 2.5.2+ |

## 故障排除

### GLFW编译错误
- 确保安装了OpenGL开发库
- Windows: 通常已包含在SDK中
- Linux: `sudo apt install libgl1-mesa-dev`
- macOS: 系统自带

### cglm找不到
- 确保子模块已正确初始化：`git submodule update --init`
- 检查CMakeLists.txt中的路径是否正确

### glad链接错误
- 确保glad.c已添加到项目中
- 检查include路径是否正确设置

## 更新依赖

```bash
# 更新所有子模块到最新版本
git submodule update --remote --merge

# 更新特定子模块
git submodule update --remote third_party/glfw
```

## 移除依赖

如果需要移除某个子模块：

```bash
# 1. 删除子模块配置
git submodule deinit -f third_party/glfw

# 2. 删除.git/modules中的子模块
rm -rf .git/modules/third_party/glfw

# 3. 删除工作目录中的子模块
git rm -f third_party/glfw
```

## 贡献

如果添加新的第三方依赖，请：
1. 更新本README文件
2. 确保许可证兼容
3. 在CMakeLists.txt中添加相应配置
4. 更新文档说明如何集成
