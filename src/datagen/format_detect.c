/**
 * @file format_detect.c
 * @brief 高度图格式检测
 */

#include "../../include/contourforge/datagen.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief 获取文件扩展名
 */
static const char* get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

/**
 * @brief 转换为小写
 */
static void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

/**
 * @brief 检测文件格式
 */
cf_heightmap_format_t cf_heightmap_detect_format(const char* filename) {
    if (!filename) {
        return CF_FORMAT_UNKNOWN;
    }

    // 获取扩展名
    const char* ext = get_file_extension(filename);
    if (!ext || !ext[0]) {
        return CF_FORMAT_UNKNOWN;
    }

    // 转换为小写进行比较
    char ext_lower[16];
    strncpy(ext_lower, ext, sizeof(ext_lower) - 1);
    ext_lower[sizeof(ext_lower) - 1] = '\0';
    to_lowercase(ext_lower);

    // 检查扩展名
    if (strcmp(ext_lower, "png") == 0) {
        return CF_FORMAT_PNG;
    } else if (strcmp(ext_lower, "jpg") == 0 || strcmp(ext_lower, "jpeg") == 0) {
        return CF_FORMAT_JPEG;
    } else if (strcmp(ext_lower, "bmp") == 0) {
        return CF_FORMAT_BMP;
    } else if (strcmp(ext_lower, "tif") == 0 || strcmp(ext_lower, "tiff") == 0) {
        // 尝试检测是否为GeoTIFF
        FILE* file = fopen(filename, "rb");
        if (file) {
            uint8_t header[8];
            if (fread(header, 1, 8, file) == 8) {
                // 检查TIFF魔数
                int is_tiff = (header[0] == 0x49 && header[1] == 0x49) ||  // Little-endian
                              (header[0] == 0x4D && header[1] == 0x4D);     // Big-endian
                
                fclose(file);
                
                if (is_tiff) {
                    // 简单检测：假设所有TIFF都可能是GeoTIFF
                    // 实际应用中可以检查GeoTIFF特定标签
                    return CF_FORMAT_GEOTIFF;
                }
            } else {
                fclose(file);
            }
        }
        return CF_FORMAT_TIFF;
    } else if (strcmp(ext_lower, "raw") == 0 || strcmp(ext_lower, "r16") == 0 || 
               strcmp(ext_lower, "r32") == 0) {
        return CF_FORMAT_RAW;
    }

    return CF_FORMAT_UNKNOWN;
}

/**
 * @brief 获取格式名称
 */
const char* cf_heightmap_format_name(cf_heightmap_format_t format) {
    switch (format) {
        case CF_FORMAT_PNG:      return "PNG";
        case CF_FORMAT_JPEG:     return "JPEG";
        case CF_FORMAT_BMP:      return "BMP";
        case CF_FORMAT_TIFF:     return "TIFF";
        case CF_FORMAT_GEOTIFF:  return "GeoTIFF";
        case CF_FORMAT_RAW:      return "RAW";
        default:                 return "Unknown";
    }
}
