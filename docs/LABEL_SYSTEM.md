# 等高线标注系统

Contourforge v0.3.0引入了等高线标注功能，可以在3D场景中显示等高线的高度值。

## 功能特性

### 文字渲染
- **TrueType字体支持**：使用stb_truetype库渲染高质量文字
- **Billboard效果**：文字始终面向相机，保持可读性
- **字形纹理图集**：优化渲染性能
- **可配置样式**：字体大小、颜色、透明度

### 标注放置
- **自动放置算法**：沿等高线均匀分布标注
- **碰撞检测**：避免标注重叠
- **LOD系统**：根据相机距离动态调整标注密度
- **距离过滤**：只显示指定距离范围内的标注

### 配置选项
- **间距控制**：设置标注之间的距离
- **单位显示**：米、英尺等单位
- **小数精度**：控制数值显示精度
- **颜色和大小**：自定义标注外观

## API使用

### 1. 加载字体

```c
cf_font_t* font;
cf_result_t result = cf_font_load("data/fonts/default.ttf", 24.0f, &font);
```

### 2. 创建文字渲染器

```c
cf_text_renderer_t* text_renderer;
result = cf_text_renderer_create(font, text_shader, &text_renderer);
```

### 3. 配置标注

```c
cf_label_config_t label_config = {
    .spacing = 50.0f,           // 标注间距（米）
    .min_distance = 10.0f,      // 最小显示距离
    .max_distance = 1000.0f,    // 最大显示距离
    .lod_levels = 4,            // LOD层级数
    .unit = "m",                // 单位
    .decimal_places = 1,        // 小数位数
    .color = {1.0f, 1.0f, 0.0f, 1.0f},  // 黄色
    .size = 24.0f,              // 字体大小
    .show_index = false         // 不显示索引
};
```

### 4. 创建标注管理器

```c
cf_label_manager_t* label_manager;
result = cf_label_manager_create(text_renderer, &label_config, &label_manager);
```

### 5. 生成标注

```c
// 为等高线模型生成标注
result = cf_label_manager_generate_labels(label_manager, model, camera);
```

### 6. 渲染标注

```c
// 在渲染循环中
const float* view = cf_camera_get_view_matrix(camera);
const float* proj = cf_camera_get_projection_matrix(camera);
cf_label_manager_render(label_manager, view, proj);
```

### 7. 更新标注（相机变化时）

```c
// 相机移动后更新标注可见性和LOD
cf_label_manager_update(label_manager, camera);
```

### 8. 清理资源

```c
cf_label_manager_destroy(label_manager);
cf_text_renderer_destroy(text_renderer);
cf_font_destroy(font);
```

## 完整示例

参见 `examples/label_demo.c` 获取完整的标注系统使用示例。

### 运行演示

```bash
cd build
./examples/label_demo [heightmap_path]
```

### 控制

- **L** - 切换标注显示/隐藏
- **R** - 重新生成标注
- **鼠标拖拽** - 旋转相机
- **鼠标滚轮** - 缩放
- **ESC** - 退出

## 着色器

标注系统使用专用的文字着色器：

- `shaders/text.vert` - 顶点着色器（Billboard变换）
- `shaders/text.frag` - 片段着色器（纹理采样）

## 字体资源

默认字体位于 `data/fonts/default.ttf`（Roboto Regular）。

### 使用自定义字体

将TrueType字体文件放入 `data/fonts/` 目录，然后加载：

```c
cf_font_load("data/fonts/your_font.ttf", size, &font);
```

## LOD系统

标注密度根据相机距离自动调整：

| 距离范围 | 间距倍数 |
|---------|---------|
| < 100m  | 1x      |
| 100-300m | 2x     |
| 300-600m | 4x     |
| 600-1000m | 8x    |
| > 1000m  | 16x    |

## 性能优化

- **字形缓存**：字形纹理预先烘焙到图集
- **批量渲染**：所有标注在一次绘制调用中渲染
- **视锥剔除**：只渲染可见范围内的标注
- **LOD**：远距离减少标注数量

## 技术细节

### Billboard变换

标注使用Billboard技术始终面向相机：

```glsl
vec3 camera_right = vec3(view[0][0], view[1][0], view[2][0]);
vec3 camera_up = vec3(view[0][1], view[1][1], view[2][1]);

vec3 world_pos = label_position + 
                 camera_right * vertex_pos.x * scale +
                 camera_up * vertex_pos.y * scale;
```

### 字形纹理图集

所有字符预先渲染到512x512的纹理图集，支持ASCII 32-127字符。

### 标注放置算法

1. 遍历等高线的所有线段
2. 累积线段长度
3. 当累积长度超过间距时放置标注
4. 检查与已有标注的碰撞
5. 重置累积长度

## 限制

- 当前仅支持ASCII字符（32-127）
- 字形纹理图集大小固定（512x512）
- 不支持多行文字
- 不支持文字旋转（始终水平）

## 未来改进

- Unicode支持
- 动态字形图集
- 文字阴影/描边
- 多行文字
- 文字旋转选项
- 更智能的标注放置（考虑地形坡度）
