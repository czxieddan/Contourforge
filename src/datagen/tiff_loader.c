/**
 * @file tiff_loader.c
 * @brief TIFF格式高度图加载器
 */

#include "../../third_party/tinytiff/tinytiffreader.h"
#include "../../include/contourforge/datagen.h"
#include "../../include/contourforge/core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief 从TIFF文件加载高度图
 */
cf_result_t cf_heightmap_load_tiff(
    const char* filepath,
    cf_heightmap_t** heightmap
) {
    if (!filepath || !heightmap) {
        return CF_ERROR_INVALID_PARAM;
    }

    // 打开TIFF文件
    TinyTIFFReader* tiff = TinyTIFFReader_open(filepath);
    if (!tiff) {
        return CF_ERROR_FILE_NOT_FOUND;
    }

    // 读取IFD
    if (!TinyTIFFReader_readIFD(tiff)) {
        TinyTIFFReader_close(tiff);
        return CF_ERROR_INVALID_FORMAT;
    }

    // 获取图像尺寸
    uint32_t width, height;
    TinyTIFFReader_getDimensions(tiff, &width, &height);

    // 读取图像数据为浮点数组
    float* data = TinyTIFFReader_readImageAsFloat(tiff);
    if (!data) {
        TinyTIFFReader_close(tiff);
        return CF_ERROR_INVALID_FORMAT;
    }

    // 分配高度图结构
    cf_heightmap_t* hm = (cf_heightmap_t*)malloc(sizeof(cf_heightmap_t));
    if (!hm) {
        free(data);
        TinyTIFFReader_close(tiff);
        return CF_ERROR_OUT_OF_MEMORY;
    }

    // 计算最小最大高度
    float min_h = INFINITY;
    float max_h = -INFINITY;
    
    size_t pixel_count = (size_t)width * height;
    for (size_t i = 0; i < pixel_count; i++) {
        float h = data[i];
        if (h < min_h) min_h = h;
        if (h > max_h) max_h = h;
    }

    // 填充高度图结构
    hm->width = (int)width;
    hm->height = (int)height;
    hm->data = data;
    hm->min_height = min_h;
    hm->max_height = max_h;

    TinyTIFFReader_close(tiff);

    *heightmap = hm;
    return CF_SUCCESS;
}
