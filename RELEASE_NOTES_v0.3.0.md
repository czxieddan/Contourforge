# Contourforge v0.3.0 Release Notes

发布日期：2026-05-01

## 版本亮点

- 📁 多格式支持：TIFF、GeoTIFF、RAW等专业地理数据格式
- 🏷️ 等高线标注：3D场景中显示高度值，支持TrueType字体和Billboard效果
- 📊 完善的测试：新增LOD、线程池、标注、格式加载单元测试

## 新功能

### 1. 多格式支持

- TIFF格式（8/16/32位灰度，高度数据自动归一化）
- GeoTIFF格式识别（地理元数据接口预留）
- RAW格式（U8、U16、I16、U32、I32、F32六种数据类型）
- 自动格式检测（扩展名和文件魔数）
- 格式转换工具 `format_converter`
- 统一加载接口 `cf_heightmap_load()` 可自动分派PNG/JPEG/BMP/TIFF加载器

### 2. 等高线标注

- TrueType字体渲染（基于 `stb_truetype`）
- 3D文字Billboard效果
- 自动标注放置算法
- 碰撞检测与最小距离约束
- 基于相机距离的LOD标注密度
- 可配置单位、小数位、颜色、字号和显示距离

### 3. 测试完善

- LOD系统测试：创建/销毁、层级选择、统计信息、并行LOD生成
- 线程池测试：创建/销毁、任务提交执行、并发安全性、轻量性能验证
- 标注系统测试：字体加载参数验证、标注生成、放置算法、距离过滤
- 格式加载测试：格式检测、RAW加载、参数验证、多数据类型加载

## API变更

### 新增API

#### 格式支持

- `cf_heightmap_detect_format()`
- `cf_heightmap_format_name()`
- `cf_heightmap_load_tiff()`
- `cf_heightmap_load_raw()`
- `cf_heightmap_format_t`
- `cf_raw_format_t`
- `cf_geo_metadata_t`

#### 标注系统

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
- `cf_label_config_t`

### 废弃API

- 无

## 使用示例

### 自动加载TIFF/PNG/JPEG/BMP

```c
cf_heightmap_t* heightmap = NULL;
cf_result_t result = cf_heightmap_load("terrain.tif", &heightmap);
if (result != CF_SUCCESS) {
    return result;
}

printf("size=%dx%d, range=[%.3f, %.3f]\n",
       heightmap->width,
       heightmap->height,
       heightmap->min_height,
       heightmap->max_height);

cf_heightmap_destroy(heightmap);
```

### 加载RAW高度图

```c
cf_heightmap_t* heightmap = NULL;
cf_result_t result = cf_heightmap_load_raw(
    "terrain_u16.raw",
    1024,
    1024,
    CF_RAW_FORMAT_U16,
    &heightmap
);
```

### 启用等高线标注

```c
cf_font_t* font = NULL;
cf_font_load("data/fonts/default.ttf", 18.0f, &font);

cf_shader_t* text_shader = NULL;
cf_shader_load("shaders/text.vert", "shaders/text.frag", &text_shader);

cf_text_renderer_t* text_renderer = NULL;
cf_text_renderer_create(font, text_shader, &text_renderer);

cf_label_config_t label_config = {
    .spacing = 50.0f,
    .min_distance = 5.0f,
    .max_distance = 1000.0f,
    .lod_levels = 4,
    .unit = "m",
    .decimal_places = 1,
    .color = {1.0f, 1.0f, 1.0f, 1.0f},
    .size = 18.0f,
    .show_index = false
};

cf_label_manager_t* labels = NULL;
cf_label_manager_create(text_renderer, &label_config, &labels);
cf_label_manager_generate_labels(labels, contour_model, camera);
```

## 升级指南

从v0.2.0升级到v0.3.0：

1. 更新库文件并重新编译项目。
2. 保持现有PNG/JPEG/BMP高度图加载代码不变。
3. 可选：使用新的格式支持API加载TIFF/RAW数据。
4. 可选：在渲染初始化后创建文字渲染器和标注管理器，启用等高线标注。
5. 可选：运行新增测试验证集成：`ctest -R "test_lod|test_threading|test_label|test_formats" -V`。

## 已知问题

- 标注在极近距离或高密度线段场景中可能重叠。
- TIFF压缩格式支持有限，推荐使用未压缩灰度TIFF。
- GeoTIFF当前仅识别格式，暂不解析完整投影和坐标系统。
- OpenGL相关字体和文字渲染API必须在有效OpenGL上下文中调用。

## 下载

- 源代码：https://github.com/czxieddan/contourforge/archive/v0.3.0.zip
- 发布页：https://github.com/czxieddan/contourforge/releases/tag/v0.3.0

## 校验状态

- Windows MSVC Debug构建：通过
- 示例程序编译：通过
- 单元测试套件：新增LOD、线程池、标注和格式测试
- 文档：README、CHANGELOG、API、发布说明和项目完成报告已更新
