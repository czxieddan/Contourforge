# Contourforge着色器说明

本目录包含Contourforge渲染库使用的GLSL着色器文件。

## 着色器列表

### 1. basic.vert / basic.frag
基础顶点和片段着色器，用于简单的3D渲染。

**特性**：
- 支持MVP变换（Model-View-Projection）
- 支持顶点颜色
- 支持uniform颜色覆盖
- 支持透明度控制

**Uniforms**：
- `mat4 model` - 模型矩阵
- `mat4 view` - 视图矩阵
- `mat4 projection` - 投影矩阵
- `vec4 color` - 统一颜色（可选）
- `float alpha` - 透明度

### 2. line.vert / line.frag
专门用于线段渲染的着色器。

**特性**：
- 优化的线段渲染
- 深度淡化效果（远处的线更透明）
- 可调节线宽
- 支持顶点颜色和统一颜色

**Uniforms**：
- `mat4 model` - 模型矩阵
- `mat4 view` - 视图矩阵
- `mat4 projection` - 投影矩阵
- `vec4 lineColor` - 线段颜色（可选）
- `float alpha` - 透明度
- `float lineWidth` - 线宽
- `bool useDepthFade` - 是否启用深度淡化
- `float depthFadeStart` - 淡化开始距离
- `float depthFadeEnd` - 淡化结束距离

## 使用示例

```c
// 加载着色器
cf_shader_t* shader;
cf_shader_load("shaders/line.vert", "shaders/line.frag", &shader);

// 使用着色器
cf_shader_use(shader);

// 设置uniforms
cf_shader_set_mat4(shader, "model", model_matrix);
cf_shader_set_mat4(shader, "view", view_matrix);
cf_shader_set_mat4(shader, "projection", projection_matrix);
cf_shader_set_float(shader, "alpha", 1.0f);
cf_shader_set_int(shader, "useDepthFade", 1);
cf_shader_set_float(shader, "depthFadeStart", 100.0f);
cf_shader_set_float(shader, "depthFadeEnd", 500.0f);

// 渲染
glDrawArrays(GL_LINES, 0, vertex_count);
```

## 版本要求

所有着色器使用OpenGL 3.3 Core Profile（GLSL 330）。

## 扩展着色器

如需添加新的着色器效果，建议：
1. 复制现有着色器作为模板
2. 修改顶点/片段着色器逻辑
3. 更新uniform变量
4. 在渲染代码中设置新的uniforms
