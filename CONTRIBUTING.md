# 贡献指南

感谢您对Contourforge项目的关注！我们欢迎各种形式的贡献。

---

## 目录

1. [行为准则](#行为准则)
2. [如何贡献](#如何贡献)
3. [开发流程](#开发流程)
4. [代码规范](#代码规范)
5. [提交规范](#提交规范)
6. [测试要求](#测试要求)
7. [文档规范](#文档规范)

---

## 行为准则

### 我们的承诺

为了营造开放和友好的环境，我们承诺：

- 尊重不同的观点和经验
- 优雅地接受建设性批评
- 关注对社区最有利的事情
- 对其他社区成员表示同理心

### 不可接受的行为

- 使用性化的语言或图像
- 人身攻击或侮辱性评论
- 公开或私下骚扰
- 未经许可发布他人的私人信息
- 其他不道德或不专业的行为

---

## 如何贡献

### 报告Bug

发现Bug？请帮助我们改进！

1. **检查是否已报告**: 搜索 [Issues](https://github.com/czxieddan/contourforge/issues)
2. **创建新Issue**: 使用Bug报告模板
3. **提供详细信息**:
   - 操作系统和版本
   - 编译器和版本
   - 重现步骤
   - 预期行为
   - 实际行为
   - 错误日志或截图

**Bug报告模板**:
```markdown
**环境**
- OS: Windows 11
- 编译器: MSVC 2022
- Contourforge版本: 0.1.0

**重现步骤**
1. 加载高度图 'terrain.png'
2. 设置等高线间隔为10
3. 调用 cf_contour_generate()

**预期行为**
生成等高线模型

**实际行为**
程序崩溃，错误码-2

**日志**
```
错误: 内存不足
```

### 提出新功能

有好主意？我们很乐意听取！

1. **检查路线图**: 查看 [ARCHITECTURE.md](ARCHITECTURE.md#9-开发路线图)
2. **创建Feature Request**: 使用功能请求模板
3. **描述清楚**:
   - 功能描述
   - 使用场景
   - 预期API设计
   - 可能的实现方案

### 改进文档

文档永远可以更好！

- 修正拼写错误
- 改进示例代码
- 添加使用教程
- 翻译文档

### 贡献代码

准备好编码了？太棒了！

1. **选择Issue**: 查找标记为 `good first issue` 的Issue
2. **讨论方案**: 在Issue中讨论实现方案
3. **Fork仓库**: Fork到您的账户
4. **创建分支**: 从 `main` 创建功能分支
5. **编写代码**: 遵循代码规范
6. **编写测试**: 确保测试覆盖
7. **提交PR**: 创建Pull Request

---

## 开发流程

### 1. 设置开发环境

```bash
# 克隆您的Fork
git clone https://github.com/YOUR_USERNAME/contourforge.git
cd contourforge

# 添加上游仓库
git remote add upstream https://github.com/czxieddan/contourforge.git

# 安装依赖
git submodule update --init --recursive

# 构建项目
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCF_BUILD_TESTS=ON
cmake --build .

# 运行测试
ctest
```

### 2. 创建功能分支

```bash
# 更新main分支
git checkout main
git pull upstream main

# 创建功能分支
git checkout -b feature/amazing-feature
```

分支命名规范：
- `feature/` - 新功能
- `fix/` - Bug修复
- `docs/` - 文档更新
- `refactor/` - 代码重构
- `test/` - 测试相关

### 3. 开发和测试

```bash
# 编写代码
# ...

# 编译
cmake --build build

# 运行测试
cd build
ctest -V

# 运行特定测试
./bin/test_memory
```

### 4. 提交更改

```bash
# 添加更改
git add .

# 提交（遵循提交规范）
git commit -m "feat: add amazing feature"

# 推送到您的Fork
git push origin feature/amazing-feature
```

### 5. 创建Pull Request

1. 访问您的Fork页面
2. 点击 "New Pull Request"
3. 填写PR模板
4. 等待代码审查

---

## 代码规范

### C语言规范

#### 命名约定

```c
// 函数: cf_<module>_<action>
cf_result_t cf_model_create(const char* name, cf_model_t** model);

// 类型: cf_<name>_t
typedef struct cf_model cf_model_t;

// 枚举: CF_<NAME>
typedef enum {
    CF_SUCCESS = 0,
    CF_ERROR_INVALID_PARAM = -1
} cf_result_t;

// 宏: CF_<NAME>
#define CF_MAX_PATH 260
```

#### 代码风格

```c
// 缩进: 4个空格
// 大括号: K&R风格
if (condition) {
    // 代码
} else {
    // 代码
}

// 指针: 星号靠近类型
cf_model_t* model;

// 函数参数: 每行一个（如果太长）
cf_result_t cf_contour_generate(
    const cf_heightmap_t* heightmap,
    const cf_contour_config_t* config,
    cf_model_t** model
);
```

#### 注释规范

```c
/**
 * @brief 创建模型
 * 
 * 分配内存并初始化模型结构。模型包含点集和线集。
 * 
 * @param name 模型名称（可以为NULL）
 * @param model 输出模型指针
 * @return CF_SUCCESS 或错误码
 * 
 * @note 使用完毕后必须调用 cf_model_destroy() 释放
 * 
 * @example
 * ```c
 * cf_model_t* model;
 * cf_model_create("Terrain", &model);
 * // 使用模型...
 * cf_model_destroy(model);
 * ```
 */
cf_result_t cf_model_create(const char* name, cf_model_t** model);
```

#### 错误处理

```c
// 总是检查返回值
cf_result_t result = cf_model_create("Model", &model);
if (result != CF_SUCCESS) {
    fprintf(stderr, "Error: Failed to create model\n");
    return result;
}

// 资源清理
cf_model_t* model = NULL;
cf_result_t result = cf_model_create("Model", &model);
if (result != CF_SUCCESS) {
    goto cleanup;
}

// ... 使用模型 ...

cleanup:
    if (model) {
        cf_model_destroy(model);
    }
    return result;
```

### 性能考虑

- 避免不必要的内存分配
- 使用内存池管理小对象
- 缓存计算结果
- 使用SIMD优化（如果适用）
- 考虑缓存局部性

---

## 提交规范

### Conventional Commits

使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范：

```
<type>(<scope>): <subject>

<body>

<footer>
```

#### Type（类型）

- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档更新
- `style`: 代码格式（不影响功能）
- `refactor`: 重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建/工具相关

#### Scope（范围）

- `core`: 核心模块
- `rendering`: 渲染模块
- `datagen`: 数据生成模块
- `control`: 控制模块
- `build`: 构建系统
- `docs`: 文档

#### 示例

```bash
# 新功能
git commit -m "feat(datagen): add Visvalingam simplification algorithm"

# Bug修复
git commit -m "fix(rendering): fix memory leak in shader loading"

# 文档
git commit -m "docs: update API documentation for camera module"

# 性能优化
git commit -m "perf(core): optimize octree query with SIMD"
```

---

## 测试要求

### 单元测试

每个新功能都应该有对应的单元测试：

```c
// tests/test_new_feature.c
#include "unity.h"
#include <contourforge/core.h>

void setUp(void) {
    // 测试前准备
}

void tearDown(void) {
    // 测试后清理
}

void test_new_feature_basic(void) {
    // 测试基本功能
    cf_result_t result = cf_new_feature();
    TEST_ASSERT_EQUAL(CF_SUCCESS, result);
}

void test_new_feature_edge_cases(void) {
    // 测试边界情况
    cf_result_t result = cf_new_feature_with_null();
    TEST_ASSERT_EQUAL(CF_ERROR_INVALID_PARAM, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_new_feature_basic);
    RUN_TEST(test_new_feature_edge_cases);
    return UNITY_END();
}
```

### 测试覆盖

- 正常情况
- 边界情况
- 错误情况
- 性能测试（如果适用）

### 运行测试

```bash
# 运行所有测试
cd build
ctest

# 运行特定测试
ctest -R test_memory

# 详细输出
ctest -V

# 并行运行
ctest -j8
```

---

## 文档规范

### API文档

使用Doxygen风格注释：

```c
/**
 * @brief 简短描述
 * 
 * 详细描述，可以多行。
 * 
 * @param param1 参数1描述
 * @param param2 参数2描述
 * @return 返回值描述
 * 
 * @note 注意事项
 * @warning 警告信息
 * @see 相关函数
 * 
 * @example
 * ```c
 * // 示例代码
 * ```
 */
```

### Markdown文档

- 使用清晰的标题层级
- 添加目录（长文档）
- 使用代码块和语法高亮
- 添加示例和截图
- 保持简洁明了

---

## 代码审查

### 审查清单

提交PR前，请自我审查：

- [ ] 代码遵循项目规范
- [ ] 添加了必要的注释
- [ ] 编写了单元测试
- [ ] 所有测试通过
- [ ] 更新了相关文档
- [ ] 没有编译警告
- [ ] 没有内存泄漏
- [ ] 性能影响可接受

### 审查流程

1. **自动检查**: CI会自动运行测试
2. **代码审查**: 维护者会审查代码
3. **讨论修改**: 根据反馈修改
4. **合并**: 审查通过后合并

---

## 发布流程

（仅限维护者）

1. 更新版本号
2. 更新CHANGELOG.md
3. 创建Git标签
4. 构建发布包
5. 发布到GitHub Releases
6. 更新文档网站

---

## 获取帮助

遇到问题？

- 📖 查看 [文档](docs/)
- 💬 在 [Discussions](https://github.com/czxieddan/contourforge/discussions) 提问
- 🐛 在 [Issues](https://github.com/czxieddan/contourforge/issues) 报告问题
- 📧 发送邮件到 czxieddan@gmail.com

---

## 致谢

感谢所有贡献者！

您的贡献将被记录在：
- [CHANGELOG.md](CHANGELOG.md)
- [贡献者列表](https://github.com/czxieddan/contourforge/graphs/contributors)

---

**感谢您的贡献！** 🎉

---

**最后更新**: 2026-04-30
