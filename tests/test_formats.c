/**
 * @file test_formats.c
 * @brief 多格式支持单元测试
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
    } while(0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return 1; \
    } while(0)

/**
 * @brief 测试格式检测
 */
int test_format_detection(void) {
    printf("\n测试: 格式检测\n");
    
    // 测试PNG
    cf_heightmap_format_t format = cf_heightmap_detect_format("test.png");
    TEST_ASSERT(format == CF_FORMAT_PNG, "PNG格式检测");
    
    // 测试JPEG
    format = cf_heightmap_detect_format("test.jpg");
    TEST_ASSERT(format == CF_FORMAT_JPEG, "JPEG格式检测");
    
    format = cf_heightmap_detect_format("test.jpeg");
    TEST_ASSERT(format == CF_FORMAT_JPEG, "JPEG格式检测（.jpeg扩展名）");
    
    // 测试BMP
    format = cf_heightmap_detect_format("test.bmp");
    TEST_ASSERT(format == CF_FORMAT_BMP, "BMP格式检测");
    
    // 测试TIFF
    format = cf_heightmap_detect_format("test.tif");
    TEST_ASSERT(format == CF_FORMAT_TIFF || format == CF_FORMAT_GEOTIFF, "TIFF格式检测");
    
    format = cf_heightmap_detect_format("test.tiff");
    TEST_ASSERT(format == CF_FORMAT_TIFF || format == CF_FORMAT_GEOTIFF, "TIFF格式检测（.tiff扩展名）");
    
    // 测试RAW
    format = cf_heightmap_detect_format("test.raw");
    TEST_ASSERT(format == CF_FORMAT_RAW, "RAW格式检测");
    
    // 测试未知格式
    format = cf_heightmap_detect_format("test.xyz");
    TEST_ASSERT(format == CF_FORMAT_UNKNOWN, "未知格式检测");
    
    TEST_PASS("格式检测");
}

/**
 * @brief 测试格式名称
 */
int test_format_names(void) {
    printf("\n测试: 格式名称\n");
    
    TEST_ASSERT(strcmp(cf_heightmap_format_name(CF_FORMAT_PNG), "PNG") == 0, "PNG名称");
    TEST_ASSERT(strcmp(cf_heightmap_format_name(CF_FORMAT_JPEG), "JPEG") == 0, "JPEG名称");
    TEST_ASSERT(strcmp(cf_heightmap_format_name(CF_FORMAT_BMP), "BMP") == 0, "BMP名称");
    TEST_ASSERT(strcmp(cf_heightmap_format_name(CF_FORMAT_TIFF), "TIFF") == 0, "TIFF名称");
    TEST_ASSERT(strcmp(cf_heightmap_format_name(CF_FORMAT_GEOTIFF), "GeoTIFF") == 0, "GeoTIFF名称");
    TEST_ASSERT(strcmp(cf_heightmap_format_name(CF_FORMAT_RAW), "RAW") == 0, "RAW名称");
    TEST_ASSERT(strcmp(cf_heightmap_format_name(CF_FORMAT_UNKNOWN), "Unknown") == 0, "未知格式名称");
    
    TEST_PASS("格式名称");
}

/**
 * @brief 创建测试RAW文件
 */
int create_test_raw_file(const char* filename, int width, int height, cf_raw_format_t format) {
    FILE* file = fopen(filename, "wb");
    if (!file) return 0;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float value = (float)y / (float)(height - 1);
            
            switch (format) {
                case CF_RAW_FORMAT_U8: {
                    uint8_t val = (uint8_t)(value * 255.0f);
                    fwrite(&val, sizeof(uint8_t), 1, file);
                    break;
                }
                case CF_RAW_FORMAT_U16: {
                    uint16_t val = (uint16_t)(value * 65535.0f);
                    fwrite(&val, sizeof(uint16_t), 1, file);
                    break;
                }
                case CF_RAW_FORMAT_F32: {
                    fwrite(&value, sizeof(float), 1, file);
                    break;
                }
                default:
                    break;
            }
        }
    }
    
    fclose(file);
    return 1;
}

/**
 * @brief 测试RAW格式加载
 */
int test_raw_loading(void) {
    printf("\n测试: RAW格式加载\n");
    
    const char* test_file = "test_heightmap.raw";
    const int width = 64;
    const int height = 64;
    
    // 创建测试文件（U8格式）
    if (!create_test_raw_file(test_file, width, height, CF_RAW_FORMAT_U8)) {
        fprintf(stderr, "无法创建测试文件\n");
        return 0;
    }
    
    // 加载RAW文件
    cf_heightmap_t* heightmap = NULL;
    cf_result_t result = cf_heightmap_load_raw(test_file, width, height, CF_RAW_FORMAT_U8, &heightmap);
    
    // 删除测试文件
    remove(test_file);
    
    TEST_ASSERT(result == CF_SUCCESS, "RAW文件加载成功");
    TEST_ASSERT(heightmap != NULL, "高度图不为空");
    TEST_ASSERT(heightmap->width == width, "宽度正确");
    TEST_ASSERT(heightmap->height == height, "高度正确");
    TEST_ASSERT(heightmap->data != NULL, "数据不为空");
    
    // 验证数据（应该是垂直渐变）
    float first_row_avg = 0.0f;
    float last_row_avg = 0.0f;
    
    for (int x = 0; x < width; x++) {
        first_row_avg += heightmap->data[x];
        last_row_avg += heightmap->data[(height - 1) * width + x];
    }
    
    first_row_avg /= width;
    last_row_avg /= width;
    
    TEST_ASSERT(first_row_avg < 0.1f, "第一行接近0");
    TEST_ASSERT(last_row_avg > 0.9f, "最后一行接近1");
    
    cf_heightmap_destroy(heightmap);
    
    TEST_PASS("RAW格式加载");
}

/**
 * @brief 测试RAW格式参数验证
 */
int test_raw_parameter_validation(void) {
    printf("\n测试: RAW格式参数验证\n");
    
    cf_heightmap_t* heightmap = NULL;
    cf_result_t result;
    
    // 测试空文件名
    result = cf_heightmap_load_raw(NULL, 64, 64, CF_RAW_FORMAT_U8, &heightmap);
    TEST_ASSERT(result == CF_ERROR_INVALID_PARAM, "空文件名应返回错误");
    
    // 测试空输出指针
    result = cf_heightmap_load_raw("test.raw", 64, 64, CF_RAW_FORMAT_U8, NULL);
    TEST_ASSERT(result == CF_ERROR_INVALID_PARAM, "空输出指针应返回错误");
    
    // 测试无效宽度
    result = cf_heightmap_load_raw("test.raw", 0, 64, CF_RAW_FORMAT_U8, &heightmap);
    TEST_ASSERT(result == CF_ERROR_INVALID_PARAM, "无效宽度应返回错误");
    
    // 测试无效高度
    result = cf_heightmap_load_raw("test.raw", 64, 0, CF_RAW_FORMAT_U8, &heightmap);
    TEST_ASSERT(result == CF_ERROR_INVALID_PARAM, "无效高度应返回错误");
    
    // 测试不存在的文件
    result = cf_heightmap_load_raw("nonexistent_file.raw", 64, 64, CF_RAW_FORMAT_U8, &heightmap);
    TEST_ASSERT(result == CF_ERROR_FILE_NOT_FOUND, "不存在的文件应返回错误");
    
    TEST_PASS("RAW格式参数验证");
}

/**
 * @brief 测试不同RAW数据类型
 */
int test_raw_data_types(void) {
    printf("\n测试: RAW数据类型\n");
    
    const int width = 32;
    const int height = 32;
    
    // 测试U16格式
    const char* test_file_u16 = "test_u16.raw";
    if (create_test_raw_file(test_file_u16, width, height, CF_RAW_FORMAT_U16)) {
        cf_heightmap_t* heightmap = NULL;
        cf_result_t result = cf_heightmap_load_raw(test_file_u16, width, height, CF_RAW_FORMAT_U16, &heightmap);
        remove(test_file_u16);
        
        TEST_ASSERT(result == CF_SUCCESS, "U16格式加载成功");
        TEST_ASSERT(heightmap != NULL, "U16高度图不为空");
        cf_heightmap_destroy(heightmap);
    }
    
    // 测试F32格式
    const char* test_file_f32 = "test_f32.raw";
    if (create_test_raw_file(test_file_f32, width, height, CF_RAW_FORMAT_F32)) {
        cf_heightmap_t* heightmap = NULL;
        cf_result_t result = cf_heightmap_load_raw(test_file_f32, width, height, CF_RAW_FORMAT_F32, &heightmap);
        remove(test_file_f32);
        
        TEST_ASSERT(result == CF_SUCCESS, "F32格式加载成功");
        TEST_ASSERT(heightmap != NULL, "F32高度图不为空");
        cf_heightmap_destroy(heightmap);
    }
    
    TEST_PASS("RAW数据类型");
}

/**
 * @brief 运行所有测试
 */
int main(void) {
    printf("========================================\n");
    printf("Contourforge 多格式支持测试\n");
    printf("版本: %s\n", cf_get_version());
    printf("========================================\n");
    
    int passed = 0;
    int total = 0;
    
    #define RUN_TEST(test_func) \
        do { \
            total++; \
            if (test_func()) passed++; \
        } while(0)
    
    RUN_TEST(test_format_detection);
    RUN_TEST(test_format_names);
    RUN_TEST(test_raw_loading);
    RUN_TEST(test_raw_parameter_validation);
    RUN_TEST(test_raw_data_types);
    
    printf("\n========================================\n");
    printf("测试结果: %d/%d 通过\n", passed, total);
    printf("========================================\n");
    
    return (passed == total) ? 0 : 1;
}
