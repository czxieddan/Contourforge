/**
 * @file heightmap.c
 * @brief 高度图加载模块实现
 */

#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb/stb_image.h"

#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief 从文件加载高度图
 */
cf_result_t cf_heightmap_load(
    const char* filepath,
    cf_heightmap_t** heightmap
) {
    if (!filepath || !heightmap) {
        return CF_ERROR_INVALID_PARAM;
    }

    // 使用stb_image加载图像
    int width, height, channels;
    unsigned char* image_data = stbi_load(filepath, &width, &height, &channels, 1);
    
    if (!image_data) {
        return CF_ERROR_FILE_NOT_FOUND;
    }

    // 分配高度图结构
    cf_heightmap_t* hm = (cf_heightmap_t*)malloc(sizeof(cf_heightmap_t));
    if (!hm) {
        stbi_image_free(image_data);
        return CF_ERROR_OUT_OF_MEMORY;
    }

    // 分配高度数据数组
    hm->width = width;
    hm->height = height;
    hm->data = (float*)malloc(sizeof(float) * width * height);
    if (!hm->data) {
        free(hm);
        stbi_image_free(image_data);
        return CF_ERROR_OUT_OF_MEMORY;
    }

    // 将像素值转换为高度值（0-255 -> 0.0-1.0）
    float min_h = INFINITY;
    float max_h = -INFINITY;
    
    for (int i = 0; i < width * height; i++) {
        float h = (float)image_data[i] / 255.0f;
        hm->data[i] = h;
        
        if (h < min_h) min_h = h;
        if (h > max_h) max_h = h;
    }

    hm->min_height = min_h;
    hm->max_height = max_h;

    // 释放图像数据
    stbi_image_free(image_data);

    *heightmap = hm;
    return CF_SUCCESS;
}

/**
 * @brief 从高度图采样高度（双线性插值）
 */
float cf_heightmap_sample(
    const cf_heightmap_t* heightmap,
    float x,
    float y
) {
    if (!heightmap || !heightmap->data) {
        return 0.0f;
    }

    // 将归一化坐标转换为像素坐标
    float px = x * (heightmap->width - 1);
    float py = y * (heightmap->height - 1);

    // 边界检查
    if (px < 0.0f) px = 0.0f;
    if (py < 0.0f) py = 0.0f;
    if (px >= heightmap->width - 1) px = heightmap->width - 1.001f;
    if (py >= heightmap->height - 1) py = heightmap->height - 1.001f;

    // 双线性插值
    int x0 = (int)px;
    int y0 = (int)py;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float fx = px - x0;
    float fy = py - y0;

    // 获取四个角的高度值
    float h00 = heightmap->data[y0 * heightmap->width + x0];
    float h10 = heightmap->data[y0 * heightmap->width + x1];
    float h01 = heightmap->data[y1 * heightmap->width + x0];
    float h11 = heightmap->data[y1 * heightmap->width + x1];

    // 双线性插值
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    float h = h0 * (1.0f - fy) + h1 * fy;

    return h;
}

/**
 * @brief 获取指定位置的高度值（最近邻）
 */
static float heightmap_get(const cf_heightmap_t* hm, int x, int y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= hm->width) x = hm->width - 1;
    if (y >= hm->height) y = hm->height - 1;
    
    return hm->data[y * hm->width + x];
}

/**
 * @brief 销毁高度图
 */
void cf_heightmap_destroy(cf_heightmap_t* heightmap) {
    if (heightmap) {
        if (heightmap->data) {
            free(heightmap->data);
        }
        free(heightmap);
    }
}
