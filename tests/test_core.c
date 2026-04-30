/**
 * @file test_core.c
 * @brief 核心模块测试
 */

#include <contourforge/core.h>
#include <stdio.h>
#include <assert.h>

void test_point_set(void)
{
    printf("Testing point set...\n");
    
    // 创建点集
    cf_point_set_t* ps = NULL;
    cf_result_t result = cf_point_set_create(10, &ps);
    assert(result == CF_SUCCESS);
    assert(ps != NULL);
    
    // 添加点
    cf_point3_t p1 = {1.0f, 2.0f, 3.0f};
    cf_index_t idx1 = cf_point_set_add(ps, p1);
    assert(idx1 == 0);
    assert(ps->count == 1);
    
    cf_point3_t p2 = {4.0f, 5.0f, 6.0f};
    cf_index_t idx2 = cf_point_set_add(ps, p2);
    assert(idx2 == 1);
    assert(ps->count == 2);
    
    // 获取点
    const cf_point3_t* retrieved = cf_point_set_get(ps, idx1);
    assert(retrieved != NULL);
    assert(retrieved->x == 1.0f);
    assert(retrieved->y == 2.0f);
    assert(retrieved->z == 3.0f);
    
    // 清理
    cf_point_set_destroy(ps);
    
    printf("  Point set tests passed!\n");
}

void test_line_set(void)
{
    printf("Testing line set...\n");
    
    // 创建点集
    cf_point_set_t* ps = NULL;
    cf_point_set_create(10, &ps);
    
    cf_point3_t p1 = {0.0f, 0.0f, 0.0f};
    cf_point3_t p2 = {1.0f, 1.0f, 1.0f};
    cf_index_t idx1 = cf_point_set_add(ps, p1);
    cf_index_t idx2 = cf_point_set_add(ps, p2);
    
    // 创建线集
    cf_line_set_t* ls = NULL;
    cf_result_t result = cf_line_set_create(ps, 10, &ls);
    assert(result == CF_SUCCESS);
    assert(ls != NULL);
    
    // 添加线段
    cf_index_t line_idx = cf_line_set_add(ls, idx1, idx2);
    assert(line_idx == 0);
    assert(ls->count == 1);
    
    // 清理
    cf_line_set_destroy(ls);
    cf_point_set_destroy(ps);
    
    printf("  Line set tests passed!\n");
}

void test_model(void)
{
    printf("Testing model...\n");
    
    // 创建模型
    cf_model_t* model = NULL;
    cf_result_t result = cf_model_create("test_model", &model);
    assert(result == CF_SUCCESS);
    assert(model != NULL);
    assert(model->name != NULL);
    
    // 清理
    cf_model_destroy(model);
    
    printf("  Model tests passed!\n");
}

int main(void)
{
    printf("Running core module tests...\n\n");
    
    test_point_set();
    test_line_set();
    test_model();
    
    printf("\nAll core tests passed!\n");
    return 0;
}
