/**
 * @file raw_loader.c
 * @brief RAW格式高度图加载器
 */

#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/**
 * @brief 从RAW文件加载高度图
 */
cf_result_t cf_heightmap_load_raw(
    const char* filepath,
    int width,
    int height,
    cf_raw_format_t format,
    cf_heightmap_t** heightmap
) {
    if (!filepath || !heightmap || width <= 0 || height <= 0) {
        return CF_ERROR_INVALID_PARAM;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return CF_ERROR_FILE_NOT_FOUND;
    }

    // 计算数据大小
    size_t pixel_count = (size_t)width * height;
    size_t bytes_per_pixel = 0;
    
    switch (format) {
        case CF_RAW_FORMAT_U8:
            bytes_per_pixel = 1;
            break;
        case CF_RAW_FORMAT_U16:
        case CF_RAW_FORMAT_I16:
            bytes_per_pixel = 2;
            break;
        case CF_RAW_FORMAT_U32:
        case CF_RAW_FORMAT_I32:
        case CF_RAW_FORMAT_F32:
            bytes_per_pixel = 4;
            break;
        default:
            fclose(file);
            return CF_ERROR_INVALID_PARAM;
    }

    size_t data_size = pixel_count * bytes_per_pixel;

    // 读取原始数据
    uint8_t* raw_data = (uint8_t*)malloc(data_size);
    if (!raw_data) {
        fclose(file);
        return CF_ERROR_OUT_OF_MEMORY;
    }

    if (fread(raw_data, 1, data_size, file) != data_size) {
        free(raw_data);
        fclose(file);
        return CF_ERROR_INVALID_FORMAT;
    }

    fclose(file);

    // 分配高度图结构
    cf_heightmap_t* hm = (cf_heightmap_t*)malloc(sizeof(cf_heightmap_t));
    if (!hm) {
        free(raw_data);
        return CF_ERROR_OUT_OF_MEMORY;
    }

    // 分配浮点数据数组
    hm->data = (float*)malloc(pixel_count * sizeof(float));
    if (!hm->data) {
        free(hm);
        free(raw_data);
        return CF_ERROR_OUT_OF_MEMORY;
    }

    // 转换为浮点数并计算最小最大值
    float min_h = INFINITY;
    float max_h = -INFINITY;

    for (size_t i = 0; i < pixel_count; i++) {
        float value = 0.0f;
        
        switch (format) {
            case CF_RAW_FORMAT_U8:
                value = (float)raw_data[i] / 255.0f;
                break;
            case CF_RAW_FORMAT_U16: {
                uint16_t val16;
                memcpy(&val16, &raw_data[i * 2], sizeof(uint16_t));
                value = (float)val16 / 65535.0f;
                break;
            }
            case CF_RAW_FORMAT_I16: {
                int16_t val16;
                memcpy(&val16, &raw_data[i * 2], sizeof(int16_t));
                value = ((float)val16 + 32768.0f) / 65535.0f;
                break;
            }
            case CF_RAW_FORMAT_U32: {
                uint32_t val32;
                memcpy(&val32, &raw_data[i * 4], sizeof(uint32_t));
                value = (float)((double)val32 / 4294967295.0);
                break;
            }
            case CF_RAW_FORMAT_I32: {
                int32_t val32;
                memcpy(&val32, &raw_data[i * 4], sizeof(int32_t));
                value = (float)(((double)val32 + 2147483648.0) / 4294967295.0);
                break;
            }
            case CF_RAW_FORMAT_F32:
                memcpy(&value, &raw_data[i * 4], sizeof(float));
                break;
        }
        
        hm->data[i] = value;
        if (value < min_h) min_h = value;
        if (value > max_h) max_h = value;
    }

    free(raw_data);

    hm->width = width;
    hm->height = height;
    hm->min_height = min_h;
    hm->max_height = max_h;

    *heightmap = hm;
    return CF_SUCCESS;
}
