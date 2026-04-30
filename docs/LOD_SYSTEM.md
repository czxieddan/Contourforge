# Contourforge LOD系统文档

**版本**: 0.2.0  
**日期**: 2026-04-30

## 概述

LOD（Level of Detail，细节层次）系统是Contourforge v0.2.0引入的核心功能，用于优化大规模数据的渲染性能。通过根据相机距离自动调整模型细节，LOD系统可以显著提升帧率，同时保持视觉质量。

### 主要特性

- **多级LOD层次**: 支持任意数量的LOD层级
- **自动距离选择**: 根据相机距离自动切换LOD层级
- **智能采样算法**: 
  - 均匀采样：简单快速
  - 重要性采样：基于曲率保留关键特征
- **边界保护**: 确保模型轮廓在简化后保持清晰
- **内存优化**: 使用索引共享原始点数据
- **性能统计**: 提供详细的内存和简化率统计

---

## API参考

### 数据结构

#### cf_lod_level_t
```c
typedef struct {
    cf_index_t* point_indices;      // 点索引数组
    size_t point_count;             // 点数量
    cf_index_t* line_indices;       // 线索引数组（成对）
    size_t line_count;              // 线段数量
    float distance_threshold;       // 距离阈值
    float simplification_ratio;     // 简化率
} cf_lod_level_t;
```

单个LOD层级的数据。

#### cf_lod_model_t
```c
typedef struct {
    cf_model_t* base_model;         // 基础模型
    cf_lod_level_t* levels;         // LOD层级数组
    size_t level_count;             // 层级数量
    int current_level;              // 当前使用的层级
} cf_lod_model_t;
```

完整的LOD模型，包含多个层级。

#### cf_lod_config_t
```c
typedef struct {
    size_t level_count;             // LOD层级数量
    float* distance_thresholds;     // 距离阈值数组
    float* simplification_ratios;   // 简化率数组
    bool preserve_boundaries;       // 是否保护边界
    bool use_importance_sampling;   // 是否使用重要性采样
} cf_lod_config_t;
```

LOD配置参数。

#### cf_lod_stats_t
```c
typedef struct {
    size_t original_point_count;    // 原始点数量
    size_t original_line_count;     // 原始线段数量
    size_t* level_point_counts;     // 各层级点数量
    size_t* level_line_counts;      // 各层级线段数量
    float* reduction_ratios;        // 各层级简化率
    size_t total_memory_bytes;      // 总内存占用
} cf_lod_stats_t;
```

LOD统计信息。

### 核心函数

#### cf_lod_create
```c
cf_result_t cf_lod_create(
    cf_model_t* base_model,
    const cf_lod_config_t* config,
    cf_lod_model_t** lod_model
);
```

创建LOD模型。

**参数**:
- `base_model`: 基础模型（完整细节）
- `config`: LOD配置
- `lod_model`: 输出LOD模型指针

**返回**: 成功返回`CF_SUCCESS`

**示例**:
```c
// 配置5个LOD层级
float distances[] = {20.0f, 40.0f, 60.0f, 80.0f, 100.0f};
float ratios[] = {1.0f, 0.5f, 0.25f, 0.125f, 0.0625f};

cf_lod_config_t config = {
    .level_count = 5,
    .distance_thresholds = distances,
    .simplification_ratios = ratios,
    .preserve_boundaries = true,
    .use_importance_sampling = true
};

cf_lod_model_t* lod_model;
cf_lod_create(base_model, &config, &lod_model);
```

#### cf_lod_select_level
```c
int cf_lod_select_level(
    const cf_lod_model_t* lod_model,
    float distance
);
```

根据距离选择合适的LOD层级。

**参数**:
- `lod_model`: LOD模型
- `distance`: 相机到模型的距离

**返回**: LOD层级索引（0为最高细节）

#### cf_lod_set_level
```c
cf_result_t cf_lod_set_level(
    cf_lod_model_t* lod_model,
    int level
);
```

手动设置当前LOD层级。

**参数**:
- `lod_model`: LOD模型
- `level`: 层级索引

**返回**: 成功返回`CF_SUCCESS`

#### cf_lod_get_current_level
```c
int cf_lod_get_current_level(const cf_lod_model_t* lod_model);
```

获取当前LOD层级。

**返回**: 当前层级索引

#### cf_lod_get_stats
```c
cf_result_t cf_lod_get_stats(
    const cf_lod_model_t* lod_model,
    cf_lod_stats_t* stats
);
```

获取LOD统计信息。

**参数**:
- `lod_model`: LOD模型
- `stats`: 输出统计信息

**返回**: 成功返回`CF_SUCCESS`

**注意**: 使用完毕后需调用`cf_lod_stats_destroy`释放统计信息。

#### cf_lod_stats_destroy
```c
void cf_lod_stats_destroy(cf_lod_stats_t* stats);
```

销毁LOD统计信息。

#### cf_lod_destroy
```c
void cf_lod_destroy(cf_lod_model_t* lod_model);
```

销毁LOD模型。

**注意**: 不会销毁基础模型，需要单独销毁。

### 渲染器集成

#### cf_renderer_set_lod_model
```c
cf_result_t cf_renderer_set_lod_model(
    cf_renderer_t* renderer,
    cf_lod_model_t* lod_model
);
```

设置LOD模型到渲染器。

#### cf_renderer_set_auto_lod
```c
void cf_renderer_set_auto_lod(
    cf_renderer_t* renderer,
    bool enable
);
```

启用/禁用自动LOD选择。

**参数**:
- `renderer`: 渲染器
- `enable`: true=自动选择，false=手动控制

#### cf_renderer_set_lod_debug
```c
void cf_renderer_set_lod_debug(
    cf_renderer_t* renderer,
    bool enable
);
```

启用/禁用LOD调试模式（显示当前层级信息）。

---

## 使用指南

### 基本工作流程

1. **创建基础模型**
```c
cf_model_t* model = create_your_model();
```

2. **配置LOD参数**
```c
float distances[] = {20.0f, 40.0f, 60.0f, 80.0f};
float ratios[] = {1.0f, 0.5f, 0.25f, 0.125f};

cf_lod_config_t config = {
    .level_count = 4,
    .distance_thresholds = distances,
    .simplification_ratios = ratios,
    .preserve_boundaries = true,
    .use_importance_sampling = true
};
```

3. **生成LOD模型**
```c
cf_lod_model_t* lod_model;
cf_lod_create(model, &config, &lod_model);
```

4. **设置到渲染器**
```c
cf_renderer_set_lod_model(renderer, lod_model);
cf_renderer_set_auto_lod(renderer, true);
```

5. **渲染循环**
```c
while (!cf_renderer_should_close(renderer)) {
    cf_renderer_begin_frame(renderer);
    cf_renderer_render(renderer);  // 自动选择LOD
    cf_renderer_end_frame(renderer);
}
```

6. **清理**
```c
cf_lod_destroy(lod_model);
cf_model_destroy(model);
```

### 配置建议

#### 距离阈值

距离阈值决定何时切换LOD层级。建议：

- **近距离** (< 20单位): LOD 0 (100%细节)
- **中距离** (20-50单位): LOD 1-2 (50-25%细节)
- **远距离** (> 50单位): LOD 3+ (< 25%细节)

根据场景大小调整这些值。

#### 简化率

简化率控制每个层级保留多少点：

- **LOD 0**: 1.0 (100% - 原始模型)
- **LOD 1**: 0.5 (50% - 每2个点保留1个)
- **LOD 2**: 0.25 (25% - 每4个点保留1个)
- **LOD 3**: 0.125 (12.5% - 每8个点保留1个)
- **LOD 4**: 0.0625 (6.25% - 每16个点保留1个)

#### 采样策略

- **均匀采样**: 
  - 优点：快速，可预测
  - 缺点：可能丢失重要特征
  - 适用：规则网格，性能优先

- **重要性采样**:
  - 优点：保留高曲率区域，视觉质量更好
  - 缺点：计算稍慢
  - 适用：复杂地形，质量优先

#### 边界保护

启用`preserve_boundaries`可确保：
- 始终保留首尾点
- 模型轮廓清晰
- 避免边界退化

建议始终启用。

---

## 性能优化

### 内存占用

LOD系统使用索引共享原始点数据，内存开销公式：

```
总内存 = 基础模型内存 + Σ(层级索引内存)
       ≈ 基础模型内存 × (1 + 0.5 × 层级数)
```

例如，5个LOD层级约增加150%内存。

### 渲染性能

性能提升取决于：
- **场景复杂度**: 点/线越多，提升越明显
- **相机距离**: 远距离物体使用低LOD
- **LOD层级数**: 更多层级=更平滑过渡

典型提升：
- 简单场景 (< 10K点): 1.5-2x FPS
- 中等场景 (10K-100K点): 2-3x FPS
- 复杂场景 (> 100K点): 3-5x FPS

### 最佳实践

1. **预计算LOD**: 在加载时生成，避免运行时开销
2. **缓存网格**: 渲染器自动缓存LOD网格
3. **合理层级数**: 3-5个层级通常足够
4. **距离阈值**: 根据视野和场景大小调整
5. **批量渲染**: 相同LOD的对象一起渲染

---

## 示例程序

### lod_demo.c

完整的LOD演示程序，展示：
- 自动LOD选择
- 手动LOD控制
- 实时统计信息
- 性能对比

**运行**:
```bash
./build/bin/Release/lod_demo.exe
```

**控制**:
- 鼠标左键拖拽: 旋转视角
- 鼠标滚轮: 缩放（触发LOD切换）
- A: 切换自动/手动LOD
- 1-5: 手动选择LOD层级
- S: 切换统计信息显示
- D: 切换LOD调试模式
- ESC: 退出

---

## 故障排除

### 问题：LOD切换有明显跳变

**原因**: 距离阈值设置不当或简化率变化太大

**解决**:
- 增加LOD层级数
- 使用更平滑的简化率梯度
- 调整距离阈值使过渡更渐进

### 问题：远距离模型消失

**原因**: 最低LOD层级简化率过低

**解决**:
- 增加最低LOD的简化率（如从0.0625提升到0.125）
- 确保至少保留轮廓点

### 问题：性能提升不明显

**原因**: 
- 场景太简单
- 相机距离始终很近
- LOD层级配置不合理

**解决**:
- 检查场景复杂度（点/线数量）
- 调整距离阈值
- 使用更激进的简化率

### 问题：内存占用过高

**原因**: LOD层级过多或基础模型过大

**解决**:
- 减少LOD层级数
- 优化基础模型
- 考虑流式加载

---

## 技术细节

### LOD选择算法

```c
int select_lod_level(float distance, cf_lod_config_t* config) {
    for (size_t i = 0; i < config->level_count; i++) {
        if (distance < config->distance_thresholds[i]) {
            return i;
        }
    }
    return config->level_count - 1;  // 最低细节
}
```

### 重要性采样

基于曲率的重要性度量：

```c
float curvature = acos(dot(v1, v2));  // v1, v2为相邻向量
```

曲率越大的点越重要，优先保留。

### 内存布局

```
LOD Model
├── Base Model (原始数据)
│   ├── Points [N个点]
│   └── Lines [M条线]
└── LOD Levels
    ├── Level 0: indices[N] (100%)
    ├── Level 1: indices[N/2] (50%)
    ├── Level 2: indices[N/4] (25%)
    └── Level 3: indices[N/8] (12.5%)
```

所有层级共享基础模型的点数据，仅存储索引。

---

## 未来改进

v0.3.0计划：
- [ ] 平滑LOD过渡（alpha混合）
- [ ] 基于屏幕空间的LOD选择
- [ ] 几何着色器LOD
- [ ] LOD数据序列化/反序列化
- [ ] 多线程LOD生成
- [ ] 自适应LOD（基于帧率）

---

## 参考资料

- [Real-Time Rendering, 4th Edition](http://www.realtimerendering.com/)
- [Douglas-Peucker Algorithm](https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm)
- [Visvalingam-Whyatt Algorithm](https://en.wikipedia.org/wiki/Visvalingam%E2%80%93Whyatt_algorithm)

---

**版权所有 © 2026 Contourforge项目**
