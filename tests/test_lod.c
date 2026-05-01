/**
 * @file test_lod.c
 * @brief LOD系统单元测试
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>
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

static cf_model_t* create_test_model(size_t point_count) {
    cf_model_t* model = NULL;
    if (cf_model_create("lod_test_model", &model) != CF_SUCCESS) {
        return NULL;
    }

    for (size_t i = 0; i < point_count; ++i) {
        float t = (float)i / (float)(point_count - 1);
        cf_point3_t point = {
            (float)i,
            sinf(t * 6.28318530718f) * 10.0f,
            cosf(t * 6.28318530718f) * 5.0f
        };
        cf_point_set_add(model->points, point);
    }

    for (size_t i = 0; i + 1 < point_count; ++i) {
        cf_line_set_add(model->lines, (cf_index_t)i, (cf_index_t)(i + 1));
    }

    cf_model_update_bounds(model);
    return model;
}

static cf_lod_config_t create_lod_config(float* distances, float* ratios) {
    distances[0] = 50.0f;
    distances[1] = 150.0f;
    distances[2] = 300.0f;

    ratios[0] = 1.0f;
    ratios[1] = 0.5f;
    ratios[2] = 0.25f;

    cf_lod_config_t config;
    config.level_count = 3;
    config.distance_thresholds = distances;
    config.simplification_ratios = ratios;
    config.preserve_boundaries = true;
    config.use_importance_sampling = false;
    return config;
}

static int test_lod_create_destroy(void) {
    printf("\n测试: LOD创建和销毁\n");

    cf_model_t* model = create_test_model(64);
    TEST_ASSERT(model != NULL, "测试模型创建成功");

    float distances[3];
    float ratios[3];
    cf_lod_config_t config = create_lod_config(distances, ratios);

    cf_lod_model_t* lod = NULL;
    cf_result_t result = cf_lod_create(model, &config, &lod);

    TEST_ASSERT(result == CF_SUCCESS, "LOD创建返回成功");
    TEST_ASSERT(lod != NULL, "LOD模型不为空");
    TEST_ASSERT(lod->level_count == 3, "LOD层级数量正确");
    TEST_ASSERT(lod->levels[0].point_count == 64, "第0层保留全部点");
    TEST_ASSERT(lod->levels[1].point_count == 32, "第1层按比例简化");
    TEST_ASSERT(lod->levels[2].point_count == 16, "第2层按比例简化");

    cf_lod_destroy(lod);
    cf_model_destroy(model);

    TEST_PASS("LOD创建和销毁");
}

static int test_lod_level_selection(void) {
    printf("\n测试: LOD层级选择\n");

    cf_model_t* model = create_test_model(64);
    TEST_ASSERT(model != NULL, "测试模型创建成功");

    float distances[3];
    float ratios[3];
    cf_lod_config_t config = create_lod_config(distances, ratios);

    cf_lod_model_t* lod = NULL;
    TEST_ASSERT(cf_lod_create(model, &config, &lod) == CF_SUCCESS, "LOD创建成功");

    TEST_ASSERT(cf_lod_select_level(lod, 10.0f) == 0, "近距离选择第0层");
    TEST_ASSERT(cf_lod_select_level(lod, 75.0f) == 1, "中距离选择第1层");
    TEST_ASSERT(cf_lod_select_level(lod, 250.0f) == 2, "远距离选择第2层");
    TEST_ASSERT(cf_lod_select_level(lod, 1000.0f) == 2, "超远距离选择最低细节层");

    TEST_ASSERT(cf_lod_set_level(lod, 1) == CF_SUCCESS, "设置LOD层级成功");
    TEST_ASSERT(cf_lod_get_current_level(lod) == 1, "当前LOD层级正确");
    TEST_ASSERT(cf_lod_set_level(lod, -1) == CF_ERROR_INVALID_PARAM, "拒绝负层级");
    TEST_ASSERT(cf_lod_set_level(lod, 3) == CF_ERROR_INVALID_PARAM, "拒绝越界层级");

    cf_lod_destroy(lod);
    cf_model_destroy(model);

    TEST_PASS("LOD层级选择");
}

static int test_lod_stats(void) {
    printf("\n测试: LOD统计信息\n");

    cf_model_t* model = create_test_model(80);
    TEST_ASSERT(model != NULL, "测试模型创建成功");

    float distances[3];
    float ratios[3];
    cf_lod_config_t config = create_lod_config(distances, ratios);

    cf_lod_model_t* lod = NULL;
    TEST_ASSERT(cf_lod_create(model, &config, &lod) == CF_SUCCESS, "LOD创建成功");

    cf_lod_stats_t stats;
    TEST_ASSERT(cf_lod_get_stats(lod, &stats) == CF_SUCCESS, "获取LOD统计成功");
    TEST_ASSERT(stats.original_point_count == 80, "原始点数量正确");
    TEST_ASSERT(stats.original_line_count == 79, "原始线段数量正确");
    TEST_ASSERT(stats.level_point_counts != NULL, "层级点统计不为空");
    TEST_ASSERT(stats.level_line_counts != NULL, "层级线统计不为空");
    TEST_ASSERT(stats.reduction_ratios != NULL, "简化率统计不为空");
    TEST_ASSERT(stats.level_point_counts[0] == 80, "第0层点统计正确");
    TEST_ASSERT(stats.level_point_counts[1] == 40, "第1层点统计正确");
    TEST_ASSERT(stats.total_memory_bytes > 0, "内存统计大于0");

    cf_lod_stats_destroy(&stats);
    TEST_ASSERT(stats.level_point_counts == NULL, "统计销毁后指针清空");

    cf_lod_destroy(lod);
    cf_model_destroy(model);

    TEST_PASS("LOD统计信息");
}

static int test_parallel_lod_generation(void) {
    printf("\n测试: 并行LOD生成\n");

    cf_model_t* model = create_test_model(96);
    TEST_ASSERT(model != NULL, "测试模型创建成功");

    cf_thread_config_t thread_config = cf_thread_config_default();
    thread_config.num_threads = 2;
    thread_config.task_queue_size = 16;

    cf_thread_pool_t* pool = NULL;
    TEST_ASSERT(cf_thread_pool_create(&thread_config, &pool) == CF_SUCCESS, "线程池创建成功");
    TEST_ASSERT(pool != NULL, "线程池不为空");

    float distances[3];
    float ratios[3];
    cf_lod_config_t config = create_lod_config(distances, ratios);
    config.use_importance_sampling = true;

    cf_lod_model_t* lod = NULL;
    cf_result_t result = cf_lod_create_parallel(model, &config, pool, &lod);
    TEST_ASSERT(result == CF_SUCCESS, "并行LOD创建返回成功");
    TEST_ASSERT(lod != NULL, "并行LOD模型不为空");
    TEST_ASSERT(lod->level_count == 3, "并行LOD层级数量正确");
    TEST_ASSERT(lod->levels[0].point_count == 96, "并行LOD第0层点数正确");
    TEST_ASSERT(lod->levels[1].point_count == 48, "并行LOD第1层点数正确");
    TEST_ASSERT(lod->levels[2].point_count == 24, "并行LOD第2层点数正确");

    cf_lod_destroy(lod);
    cf_thread_pool_destroy(pool);
    cf_model_destroy(model);

    TEST_PASS("并行LOD生成");
}

int main(void) {
    printf("========================================\n");
    printf("Contourforge LOD系统测试\n");
    printf("版本: %s\n", cf_get_version());
    printf("========================================\n");

    int passed = 0;
    int total = 0;

#define RUN_TEST(test_func) \
    do { \
        total++; \
        if (test_func()) passed++; \
    } while (0)

    RUN_TEST(test_lod_create_destroy);
    RUN_TEST(test_lod_level_selection);
    RUN_TEST(test_lod_stats);
    RUN_TEST(test_parallel_lod_generation);

    printf("\n========================================\n");
    printf("测试结果: %d/%d 通过\n", passed, total);
    printf("========================================\n");

    return (passed == total) ? 0 : 1;
}
