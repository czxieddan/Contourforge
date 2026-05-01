# Contourforge v0.3.0 - 多格式支持

## 新增功能

### 1. TIFF格式支持

v0.3.0版本新增了对TIFF和GeoTIFF格式的支持，使用轻量级的单头文件库`tinytiffreader.h`。

**支持的TIFF特性：**
- 8位、16位、32位灰度图像
- 无符号整数和浮点数数据类型
- 未压缩TIFF文件
- 自动字节序检测（大端/小端）
- Strip-based存储格式

**使用示例：**

```c
#include <contourforge/datagen.h>

// 直接加载TIFF文件
cf_heightmap_t* heightmap = NULL;
cf_result_t result = cf_heightmap_load_tiff("terrain.tif", &heightmap);

if (result == CF_SUCCESS) {
    printf("加载成功: %dx%d\n", heightmap->width, heightmap->height);
    cf_heightmap_destroy(heightmap);
}
```

### 2. RAW格式支持

支持原始二进制高度数据的加载，适用于专业地理数据处理。

**支持的RAW数据类型：**
- `CF_RAW_FORMAT_U8` - 8位无符号整数
- `CF_RAW_FORMAT_U16` - 16位无符号整数
- `CF_RAW_FORMAT_I16` - 16位有符号整数
- `CF_RAW_FORMAT_U32` - 32位无符号整数
- `CF_RAW_FORMAT_I32` - 32位有符号整数
- `CF_RAW_FORMAT_F32` - 32位浮点数

**使用示例：**

```c
// 加载512x512的16位RAW文件
cf_heightmap_t* heightmap = NULL;
cf_result_t result = cf_heightmap_load_raw(
    "heightmap.raw",
    512,  // 宽度
    512,  // 高度
    CF_RAW_FORMAT_U16,
    &heightmap
);
```

### 3. 自动格式检测

新增格式检测API，可以自动识别文件格式。

**API函数：**

```c
// 检测文件格式
cf_heightmap_format_t format = cf_heightmap_detect_format("terrain.tif");

// 获取格式名称
const char* name = cf_heightmap_format_name(format);
printf("格式: %s\n", name);  // 输出: "TIFF"
```

**支持的格式：**
- `CF_FORMAT_PNG` - PNG图像
- `CF_FORMAT_JPEG` - JPEG图像
- `CF_FORMAT_BMP` - BMP图像
- `CF_FORMAT_TIFF` - TIFF图像
- `CF_FORMAT_GEOTIFF` - GeoTIFF图像
- `CF_FORMAT_RAW` - 原始二进制数据
- `CF_FORMAT_UNKNOWN` - 未知格式

### 4. 统一加载接口

[`cf_heightmap_load()`](../include/contourforge/datagen.h:95)函数现在支持自动格式检测：

```c
// 自动检测并加载任何支持的格式
cf_heightmap_t* heightmap = NULL;
cf_result_t result = cf_heightmap_load("terrain.tif", &heightmap);
```

## 新增工具

### 格式转换工具

新增[`format_converter`](../examples/format_converter.c:1)示例程序，用于格式转换和信息查看。

**使用方法：**

```bash
# 显示文件信息
./format_converter terrain.tif --info

# 转换TIFF到RAW
./format_converter terrain.tif -o output.raw

# 加载RAW并显示信息
./format_converter heightmap.raw --raw-width 512 --raw-height 512 --raw-format u16 --info
```

## API变更

### 新增函数

```c
// 格式检测
cf_heightmap_format_t cf_heightmap_detect_format(const char* filename);
const char* cf_heightmap_format_name(cf_heightmap_format_t format);

// TIFF加载
cf_result_t cf_heightmap_load_tiff(const char* filepath, cf_heightmap_t** heightmap);

// RAW加载
cf_result_t cf_heightmap_load_raw(
    const char* filepath,
    int width,
    int height,
    cf_raw_format_t format,
    cf_heightmap_t** heightmap
);
```

### 新增类型

```c
// 格式枚举
typedef enum {
    CF_FORMAT_UNKNOWN,
    CF_FORMAT_PNG,
    CF_FORMAT_JPEG,
    CF_FORMAT_BMP,
    CF_FORMAT_TIFF,
    CF_FORMAT_GEOTIFF,
    CF_FORMAT_RAW
} cf_heightmap_format_t;

// RAW数据类型
typedef enum {
    CF_RAW_FORMAT_U8,
    CF_RAW_FORMAT_U16,
    CF_RAW_FORMAT_I16,
    CF_RAW_FORMAT_U32,
    CF_RAW_FORMAT_I32,
    CF_RAW_FORMAT_F32
} cf_raw_format_t;

// 地理元数据（预留）
typedef struct {
    double min_x, max_x;
    double min_y, max_y;
    double pixel_scale_x;
    double pixel_scale_y;
    char projection[256];
    char datum[64];
} cf_geo_metadata_t;
```

## 构建说明

### 依赖项

v0.3.0使用单头文件库，无需额外安装依赖：

- **tinytiffreader.h** - 轻量级TIFF读取器（已包含在`third_party/tinytiff/`）
- **stb_image.h** - PNG/JPEG/BMP支持（已包含）

### 编译

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 运行测试

```bash
# 运行所有测试
ctest

# 运行格式测试
./tests/test_formats
```

## 示例程序更新

### heightmap_loader

[`heightmap_loader`](../examples/heightmap_loader.c:1)现在支持所有格式，并显示检测到的格式：

```bash
# 加载TIFF文件
./heightmap_loader terrain.tif 10.0

# 加载PNG文件（向后兼容）
./heightmap_loader terrain.png 10.0
```

### format_converter

新增的格式转换工具：

```bash
# 查看TIFF信息
./format_converter terrain.tif --info

# 转换格式
./format_converter input.tif -o output.raw
```

## 性能特性

- **TIFF加载**：支持大文件，内存效率高
- **RAW加载**：直接内存映射，速度最快
- **格式检测**：基于文件扩展名和魔数，开销极小

## 限制和注意事项

1. **TIFF压缩**：当前版本仅支持未压缩的TIFF文件
2. **GeoTIFF元数据**：暂不解析地理坐标信息（预留接口）
3. **RAW格式**：需要手动指定尺寸和数据类型
4. **导出功能**：当前仅支持RAW格式导出

## 未来计划

- [ ] LZW和Deflate压缩TIFF支持
- [ ] GeoTIFF地理元数据解析
- [ ] PNG/TIFF格式导出
- [ ] 16位PNG支持
- [ ] HGT（SRTM）格式支持

## 兼容性

- **向后兼容**：所有v0.2.0的代码无需修改即可运行
- **平台支持**：Windows、Linux、macOS
- **编译器**：MSVC、GCC、Clang

## 迁移指南

从v0.2.0升级到v0.3.0无需修改代码。如需使用新功能：

```c
// v0.2.0 代码（仍然有效）
cf_heightmap_t* hm = NULL;
cf_heightmap_load("terrain.png", &hm);

// v0.3.0 新功能
cf_heightmap_format_t fmt = cf_heightmap_detect_format("terrain.tif");
if (fmt == CF_FORMAT_TIFF) {
    cf_heightmap_load_tiff("terrain.tif", &hm);
}
```

## 贡献者

感谢所有为v0.3.0做出贡献的开发者！

## 许可证

MIT License - 详见LICENSE文件
