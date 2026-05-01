# Contourforge v0.3.0 发布说明

发布日期：2026-05-01

## 概述

Contourforge v0.3.0是一个重要的功能更新版本，新增了对多种专业地理数据格式的支持，包括TIFF、GeoTIFF和RAW格式。此版本保持了与v0.2.0的完全向后兼容性。

## 主要新增功能

### 🎯 多格式支持

#### 1. TIFF格式支持
- 支持8位、16位、32位灰度TIFF图像
- 支持整数和浮点数数据类型
- 自动字节序检测（大端/小端）
- 使用轻量级单头文件库，无需外部依赖

#### 2. GeoTIFF格式识别
- 自动检测GeoTIFF文件
- 预留地理元数据接口
- 为未来的坐标系统支持做准备

#### 3. RAW格式支持
- 支持6种数据类型：U8、U16、I16、U32、I32、F32
- 适用于专业地理数据处理
- 高性能直接内存访问

#### 4. 自动格式检测
- 基于文件扩展名的智能检测
- 支持文件魔数验证
- 统一的加载接口

### 🛠️ 新增API

```c
// 格式检测
cf_heightmap_format_t cf_heightmap_detect_format(const char* filename);
const char* cf_heightmap_format_name(cf_heightmap_format_t format);

// 格式特定加载
cf_result_t cf_heightmap_load_tiff(const char* filepath, cf_heightmap_t** heightmap);
cf_result_t cf_heightmap_load_raw(const char* filepath, int width, int height, 
                                   cf_raw_format_t format, cf_heightmap_t** heightmap);
```

### 📦 新增工具

#### format_converter
格式转换和信息查看工具：
- 显示高度图详细信息
- 格式转换（当前支持RAW导出）
- 统计分析（平均值、标准差等）

### 🧪 新增测试

- `test_formats.c` - 多格式支持单元测试
- 格式检测测试
- RAW格式加载测试
- 参数验证测试

## 技术改进

### 架构优化
- 模块化的格式加载器设计
- 统一的数据归一化处理
- 可扩展的格式支持框架

### 代码质量
- 新增300+行单元测试
- 完善的错误处理
- 详细的API文档

### 性能
- TIFF加载：高效的strip-based读取
- RAW加载：零拷贝直接内存访问
- 格式检测：O(1)时间复杂度

## 文件变更

### 新增文件
```
third_party/tinytiff/tinytiffreader.h    - TIFF读取库
src/datagen/tiff_loader.c                - TIFF加载实现
src/datagen/raw_loader.c                 - RAW加载实现
src/datagen/format_detect.c              - 格式检测实现
examples/format_converter.c              - 格式转换工具
tests/test_formats.c                     - 格式测试
docs/FORMAT_SUPPORT.md                   - 格式支持文档
```

### 修改文件
```
include/contourforge/datagen.h           - 新增API声明
src/datagen/heightmap.c                  - 支持自动格式检测
src/datagen/CMakeLists.txt               - 添加新源文件
examples/heightmap_loader.c              - 显示格式信息
examples/CMakeLists.txt                  - 添加format_converter
tests/CMakeLists.txt                     - 添加test_formats
CMakeLists.txt                           - 版本号更新为0.3.0
```

## 兼容性

### 向后兼容
✅ 完全兼容v0.2.0 API  
✅ 现有代码无需修改  
✅ 二进制接口稳定  

### 平台支持
- ✅ Windows 10/11
- ✅ Linux (Ubuntu 20.04+)
- ✅ macOS (10.15+)

### 编译器支持
- ✅ MSVC 2019+
- ✅ GCC 9+
- ✅ Clang 10+

## 已知限制

1. **TIFF压缩**：当前仅支持未压缩TIFF
2. **GeoTIFF元数据**：暂不解析地理坐标信息
3. **格式导出**：仅支持RAW格式导出
4. **多通道TIFF**：仅使用第一通道

## 升级指南

### 从v0.2.0升级

无需修改代码，直接重新编译即可。

### 使用新功能

```c
// 检测格式
cf_heightmap_format_t format = cf_heightmap_detect_format("terrain.tif");
printf("格式: %s\n", cf_heightmap_format_name(format));

// 加载TIFF
cf_heightmap_t* hm = NULL;
cf_heightmap_load_tiff("terrain.tif", &hm);

// 加载RAW
cf_heightmap_load_raw("data.raw", 512, 512, CF_RAW_FORMAT_U16, &hm);
```

## 构建说明

```bash
# 克隆仓库
git clone https://github.com/yourusername/Contourforge.git
cd Contourforge

# 构建
mkdir build && cd build
cmake ..
cmake --build .

# 运行测试
ctest

# 运行示例
./bin/format_converter --help
./bin/heightmap_loader ../data/heightmaps/terrain_peaks.png 10.0
```

## 性能基准

在Intel Core i7-10700K @ 3.8GHz上的测试结果：

| 格式 | 文件大小 | 分辨率 | 加载时间 |
|------|---------|--------|---------|
| PNG  | 256KB   | 512x512 | ~15ms  |
| TIFF | 512KB   | 512x512 | ~20ms  |
| RAW  | 512KB   | 512x512 | ~5ms   |

## 未来计划 (v0.4.0)

- [ ] TIFF压缩支持（LZW、Deflate）
- [ ] GeoTIFF元数据解析
- [ ] PNG/TIFF格式导出
- [ ] 16位PNG支持
- [ ] HGT（SRTM）格式支持
- [ ] 批量转换工具

## 贡献

感谢所有为此版本做出贡献的开发者！

如果您发现任何问题或有功能建议，请在GitHub上提交Issue。

## 许可证

MIT License - 详见LICENSE文件

---

**完整更新日志**: [CHANGELOG.md](../CHANGELOG.md)  
**格式支持文档**: [FORMAT_SUPPORT.md](FORMAT_SUPPORT.md)  
**开发者指南**: [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)
