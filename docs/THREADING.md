# Contourforge 多线程系统文档

## 概述

Contourforge v0.2.0引入了全面的多线程支持，通过并行化关键算法显著提升大规模数据处理性能。

## 架构设计

### 线程池

线程池是多线程系统的核心，提供：

- **跨平台支持**：Windows（Win32 API）和POSIX（pthread）
- **任务队列**：FIFO任务调度
- **动态线程数**：自动检测CPU核心数或手动配置
- **优雅关闭**：等待所有任务完成后安全退出

### 并行化算法

1. **等高线提取**：按行分割，每个线程处理一部分行
2. **LOD生成**：多个LOD层级并行生成
3. **线段简化**：分段并行处理Douglas-Peucker算法

## API使用

### 1. 创建线程池

```c
#include <contourforge/threading.h>

// 使用默认配置（自动检测CPU核心数）
cf_thread_config_t config = cf_thread_config_default();

// 或手动配置
cf_thread_config_t config = {
    .num_threads = 4,           // 4个工作线程
    .enable_threading = true,   // 启用多线程
    .task_queue_size = 1024     // 任务队列大小
};

cf_thread_pool_t* pool;
cf_result_t result = cf_thread_pool_create(&config, &pool);
```

### 2. 并行等高线提取

```c
#include <contourforge/datagen.h>

cf_heightmap_t* heightmap;
cf_heightmap_load("terrain.png", &heightmap);

cf_point_set_t* points;
cf_line_set_t* lines;
cf_point_set_create(10000, &points);
cf_line_set_create(points, 10000, &lines);

// 并行提取等高线
cf_contour_extract_parallel(
    heightmap,
    100.0f,        // 等高线高度
    pool,          // 线程池
    points,
    lines
);
```

### 3. 并行LOD生成

```c
#include <contourforge/core.h>

cf_model_t* base_model;
// ... 创建基础模型 ...

cf_lod_config_t lod_config = {
    .level_count = 4,
    .distance_thresholds = (float[]){0.0f, 50.0f, 100.0f, 200.0f},
    .simplification_ratios = (float[]){1.0f, 0.5f, 0.25f, 0.1f},
    .preserve_boundaries = true,
    .use_importance_sampling = true
};

cf_lod_model_t* lod_model;
cf_lod_create_parallel(base_model, &lod_config, pool, &lod_model);
```

### 4. 并行线段简化

```c
cf_point3_t* points = /* ... */;
size_t count = 10000;

cf_point3_t* simplified;
size_t simplified_count;

cf_simplify_douglas_peucker_parallel(
    points,
    count,
    0.5f,          // 容差
    pool,
    &simplified,
    &simplified_count
);
```

### 5. 清理资源

```c
// 等待所有任务完成
cf_thread_pool_wait(pool);

// 销毁线程池
cf_thread_pool_destroy(pool);
```

## 性能优化建议

### 1. 线程数配置

```c
// 对于CPU密集型任务
config.num_threads = CPU核心数;

// 对于I/O密集型任务
config.num_threads = CPU核心数 * 2;

// 对于小数据集
config.num_threads = 2-4;  // 避免线程开销
```

### 2. 任务粒度

- **等高线提取**：每个线程至少处理100行
- **LOD生成**：适合3-8个层级并行
- **线段简化**：点数>1000时使用并行

### 3. 内存管理

```c
// 预分配足够的缓冲区
cf_point_set_create(estimated_size, &points);

// 避免频繁的内存重分配
```

## 性能基准

基于1024x1024高度图的测试结果：

### 等高线提取

| 线程数 | 时间(ms) | 加速比 |
|--------|----------|--------|
| 1      | 450      | 1.0x   |
| 2      | 260      | 1.7x   |
| 4      | 145      | 3.1x   |
| 8      | 95       | 4.7x   |

### LOD生成（4层级）

| 线程数 | 时间(ms) | 加速比 |
|--------|----------|--------|
| 1      | 320      | 1.0x   |
| 2      | 180      | 1.8x   |
| 4      | 105      | 3.0x   |

### 线段简化（10000点）

| 线程数 | 时间(ms) | 加速比 |
|--------|----------|--------|
| 1      | 85       | 1.0x   |
| 2      | 48       | 1.8x   |
| 4      | 28       | 3.0x   |
| 8      | 20       | 4.3x   |

## 线程安全

### 安全的操作

- 读取共享数据（只读访问）
- 使用线程池提交任务
- 使用互斥锁保护的数据结构

### 不安全的操作

- 并发修改同一数据结构
- 跨线程共享非线程安全的对象

### 互斥锁使用

```c
cf_mutex_t* mutex;
cf_mutex_create(&mutex);

cf_mutex_lock(mutex);
// 临界区代码
cf_mutex_unlock(mutex);

cf_mutex_destroy(mutex);
```

## 调试技巧

### 1. 禁用多线程

```c
cf_thread_config_t config = {
    .enable_threading = false  // 单线程模式
};
```

### 2. 获取线程池统计

```c
int active_threads, pending_tasks;
cf_thread_pool_get_stats(pool, &active_threads, &pending_tasks);
printf("活跃线程: %d, 待处理任务: %d\n", active_threads, pending_tasks);
```

### 3. 性能分析

运行性能测试程序：

```bash
./threading_benchmark data/heightmaps/terrain_large.png
```

## 平台差异

### Windows

- 使用Win32线程API（CreateThread, WaitForSingleObject）
- 使用临界区（CRITICAL_SECTION）
- 使用条件变量（CONDITION_VARIABLE）

### Linux/macOS

- 使用POSIX线程（pthread）
- 需要链接pthread库：`-lpthread`
- 使用pthread互斥锁和条件变量

## 编译选项

### CMake配置

```cmake
# 启用多线程支持（默认开启）
option(CF_ENABLE_THREADING "Enable multi-threading support" ON)
```

### 禁用多线程

```bash
cmake -DCF_ENABLE_THREADING=OFF ..
```

## 常见问题

### Q: 为什么多线程反而更慢？

A: 可能原因：
- 数据集太小，线程开销大于收益
- 线程数过多，上下文切换开销大
- 内存带宽瓶颈

### Q: 如何选择最佳线程数？

A: 建议：
- 从CPU核心数开始
- 运行性能测试找到最佳值
- 考虑其他并发任务

### Q: 线程池可以重用吗？

A: 可以。创建一次线程池，多次提交任务，最后统一销毁。

## 示例代码

完整示例请参考：
- `examples/threading_benchmark.c` - 性能测试
- `examples/lod_demo.c` - LOD演示（可选多线程）

## 未来改进

- [ ] 工作窃取算法
- [ ] 任务优先级
- [ ] 更细粒度的并行化
- [ ] GPU加速支持
- [ ] 自适应线程数调整

## 参考资料

- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [Windows Threading](https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads)
- [C11 Threads](https://en.cppreference.com/w/c/thread)
