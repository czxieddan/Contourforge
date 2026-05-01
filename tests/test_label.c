/**
 * @file test_label.c
 * @brief 等高线标注系统单元测试
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n  %s\n", message, #condition); \
            return 0; \
        } \
    } while (0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return 1; \
    } while (0)

static cf_label_config_t default_label_config(void) {
    cf_label_config_t config;
    memset(&config, 0, sizeof(config));
    config.spacing = 10.0f;
    config.min_distance = 0.0f;
    config.max_distance = 1000.0f;
    config.lod_levels = 4;
    strcpy(config.unit, "m");
    config.decimal_places = 1;
    config.color = (cf_color_t){1.0f, 1.0f, 1.0f, 1.0f};
    config.size = 16.0f;
    config.show_index = false;
    return config;
}

static cf_model_t* create_label_test_model(void) {
    cf_model_t* model = NULL;
    if (cf_model_create("label_test_model", &model) != CF_SUCCESS) {
        return NULL;
    }

    for (int i = 0; i <= 10; ++i) {
        cf_point3_t point = {(float)i * 10.0f, 0.0f, 100.0f};
        cf_point_set_add(model->points, point);
    }

    for (int i = 0; i < 10; ++i) {
        cf_line_set_add(model->lines, (cf_index_t)i, (cf_index_t)(i + 1));
    }

    cf_model_update_bounds(model);
    return model;
}

static int test_font_loading_validation(void) {
    printf("\n测试: 字体加载参数验证\n");

    cf_font_t* font = NULL;
    TEST_ASSERT(cf_font_load(NULL, 16.0f, &font) == CF_ERROR_INVALID_PARAM, "拒绝空字体路径");
    TEST_ASSERT(cf_font_load("data/fonts/default.ttf", 16.0f, NULL) == CF_ERROR_INVALID_PARAM, "拒绝空输出指针");
    TEST_ASSERT(cf_font_load("missing_font_file.ttf", 16.0f, &font) == CF_ERROR_FILE_NOT_FOUND, "缺失字体文件返回FILE_NOT_FOUND");

    TEST_PASS("字体加载参数验证");
}

static int test_label_manager_generate(void) {
    printf("\n测试: 标注生成\n");

    cf_label_config_t config = default_label_config();
    cf_text_renderer_t* fake_text_renderer = (cf_text_renderer_t*)0x1;
    cf_label_manager_t* manager = NULL;

    TEST_ASSERT(cf_label_manager_create(fake_text_renderer, &config, &manager) == CF_SUCCESS, "标注管理器创建成功");
    TEST_ASSERT(manager != NULL, "标注管理器不为空");

    cf_model_t* model = create_label_test_model();
    TEST_ASSERT(model != NULL, "标注测试模型创建成功");

    TEST_ASSERT(cf_label_manager_generate_labels(manager, model, NULL) == CF_SUCCESS, "无相机生成标注成功");
    TEST_ASSERT(cf_label_manager_update(manager, NULL) == CF_SUCCESS, "无相机更新标注成功");

    float identity[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    TEST_ASSERT(cf_label_manager_render(manager, NULL, identity) == CF_ERROR_INVALID_PARAM, "渲染拒绝空视图矩阵");
    TEST_ASSERT(cf_label_manager_render(manager, identity, NULL) == CF_ERROR_INVALID_PARAM, "渲染拒绝空投影矩阵");

    cf_label_manager_clear(manager);
    cf_model_destroy(model);
    cf_label_manager_destroy(manager);

    TEST_PASS("标注生成");
}

static int test_label_placement_algorithm(void) {
    printf("\n测试: 标注放置算法\n");

    cf_model_t* model = create_label_test_model();
    TEST_ASSERT(model != NULL, "标注测试模型创建成功");

    cf_point3_t positions[16];
    size_t count = 0;
    cf_result_t result = cf_place_labels_on_contour(
        model->lines,
        100.0f,
        20.0f,
        5.0f,
        positions,
        16,
        &count
    );

    TEST_ASSERT(result == CF_SUCCESS, "标注位置生成成功");
    TEST_ASSERT(count > 0, "生成至少一个标注位置");
    TEST_ASSERT(count <= 16, "标注数量不超过容量");

    for (size_t i = 0; i < count; ++i) {
        TEST_ASSERT(fabsf(positions[i].z - 100.0f) < 0.001f, "标注高度保持等高线高度");
        if (i > 0) {
            float dx = positions[i].x - positions[i - 1].x;
            float dy = positions[i].y - positions[i - 1].y;
            float dz = positions[i].z - positions[i - 1].z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);
            TEST_ASSERT(distance >= 5.0f, "标注间距满足碰撞约束");
        }
    }

    cf_model_destroy(model);

    TEST_PASS("标注放置算法");
}

static int test_collision_and_distance_filter(void) {
    printf("\n测试: 碰撞检测和距离过滤\n");

    float near_spacing = cf_calculate_label_spacing_lod(50.0f, 10.0f, 4);
    float mid_spacing = cf_calculate_label_spacing_lod(350.0f, 10.0f, 4);
    float far_spacing = cf_calculate_label_spacing_lod(1200.0f, 10.0f, 4);

    TEST_ASSERT(near_spacing == 10.0f, "近距离保持基础标注间距");
    TEST_ASSERT(mid_spacing == 40.0f, "中远距离提高标注间距");
    TEST_ASSERT(far_spacing == 160.0f, "超远距离使用最大标注间距");

    cf_point3_t positions[3] = {
        {0.0f, 0.0f, 0.0f},
        {10.0f, 0.0f, 0.0f},
        {100.0f, 0.0f, 0.0f}
    };
    bool visible[3] = {false, false, false};
    cf_point3_t camera_pos = {0.0f, 0.0f, 0.0f};

    TEST_ASSERT(cf_filter_labels_by_distance(positions, 3, camera_pos, 5.0f, 50.0f, visible) == CF_SUCCESS, "距离过滤成功");
    TEST_ASSERT(!visible[0], "过近标注不可见");
    TEST_ASSERT(visible[1], "有效距离标注可见");
    TEST_ASSERT(!visible[2], "过远标注不可见");

    TEST_PASS("碰撞检测和距离过滤");
}

int main(void) {
    printf("========================================\n");
    printf("Contourforge 标注系统测试\n");
    printf("版本: %s\n", cf_get_version());
    printf("========================================\n");

    int passed = 0;
    int total = 0;

#define RUN_TEST(test_func) \
    do { \
        total++; \
        if (test_func()) passed++; \
    } while (0)

    RUN_TEST(test_font_loading_validation);
    RUN_TEST(test_label_manager_generate);
    RUN_TEST(test_label_placement_algorithm);
    RUN_TEST(test_collision_and_distance_filter);

    printf("\n========================================\n");
    printf("测试结果: %d/%d 通过\n", passed, total);
    printf("========================================\n");

    return (passed == total) ? 0 : 1;
}
