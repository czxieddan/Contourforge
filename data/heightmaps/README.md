# 测试高度图数据

本目录包含用于测试Contourforge的高度图数据。

## 测试数据说明

### 1. 简单渐变图（用于基础测试）
- **gradient_simple.png**: 简单的线性渐变，用于验证基本的等高线提取功能
- **gradient_radial.png**: 径向渐变，用于测试圆形等高线

### 2. 真实地形数据（用于性能测试）
- 可以从以下来源获取真实的地形高度图：
  - [USGS Earth Explorer](https://earthexplorer.usgs.gov/)
  - [OpenTopography](https://opentopography.org/)
  - [Terrain.party](http://terrain.party/)

## 创建测试数据

### 使用Python生成简单测试图像

```python
import numpy as np
from PIL import Image

# 1. 线性渐变
width, height = 256, 256
gradient = np.linspace(0, 255, height, dtype=np.uint8)
gradient = np.tile(gradient.reshape(-1, 1), (1, width))
Image.fromarray(gradient).save('gradient_simple.png')

# 2. 径向渐变
x = np.linspace(-1, 1, width)
y = np.linspace(-1, 1, height)
X, Y = np.meshgrid(x, y)
R = np.sqrt(X**2 + Y**2)
R = np.clip(R, 0, 1)
radial = ((1 - R) * 255).astype(np.uint8)
Image.fromarray(radial).save('gradient_radial.png')

# 3. 山峰地形
peaks = np.zeros((height, width))
for _ in range(5):
    cx, cy = np.random.randint(0, width), np.random.randint(0, height)
    for i in range(height):
        for j in range(width):
            dist = np.sqrt((i - cy)**2 + (j - cx)**2)
            peaks[i, j] += np.exp(-dist / 30)
peaks = (peaks / peaks.max() * 255).astype(np.uint8)
Image.fromarray(peaks).save('terrain_peaks.png')
```

### 使用GIMP创建测试图像

1. 打开GIMP
2. 创建新图像（256x256像素，灰度模式）
3. 使用渐变工具创建渐变
4. 导出为PNG格式

## 数据格式要求

- **格式**: PNG、JPG、BMP等stb_image支持的格式
- **颜色模式**: 灰度图（单通道）或RGB（将自动转换为灰度）
- **分辨率**: 建议256x256到2048x2048
- **像素值**: 0-255，其中0表示最低高度，255表示最高高度

## 使用示例

```c
// 加载高度图
cf_heightmap_t* heightmap;
cf_heightmap_load("data/heightmaps/gradient_simple.png", &heightmap);

// 生成等高线
cf_contour_config_t config = {
    .interval = 10.0f,
    .min_height = 0.0f,
    .max_height = 1.0f,
    .simplify_tolerance = 0.5f,
    .build_topology = true
};

cf_model_t* model;
cf_contour_generate(heightmap, &config, &model);
```
