/**
 * @file format_converter.c
 * @brief 高度图格式转换工具
 * 
 * 功能：
 * 1. 支持多种格式之间的转换
 * 2. 显示格式信息和元数据
 * 3. 支持RAW格式的导入导出
 */

#include <contourforge/contourforge.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 打印使用说明
 */
void print_usage(const char* program_name) {
    printf("Contourforge 格式转换工具 v%s\n\n", cf_get_version());
    printf("用法:\n");
    printf("  %s <input_file> [options]\n\n", program_name);
    printf("选项:\n");
    printf("  -i, --info              显示文件信息（不转换）\n");
    printf("  -o <output>             输出文件路径\n");
    printf("  -f <format>             输出格式 (png, tiff, raw)\n");
    printf("  --raw-width <width>     RAW输入宽度\n");
    printf("  --raw-height <height>   RAW输入高度\n");
    printf("  --raw-format <format>   RAW数据格式 (u8, u16, i16, u32, i32, f32)\n\n");
    printf("示例:\n");
    printf("  # 显示TIFF文件信息\n");
    printf("  %s terrain.tif --info\n\n", program_name);
    printf("  # 转换TIFF到PNG\n");
    printf("  %s terrain.tif -o terrain.png\n\n", program_name);
    printf("  # 加载RAW文件并转换为PNG\n");
    printf("  %s heightmap.raw --raw-width 512 --raw-height 512 --raw-format u16 -o output.png\n\n", program_name);
    printf("支持的格式:\n");
    printf("  输入: PNG, JPEG, BMP, TIFF, GeoTIFF, RAW\n");
    printf("  输出: PNG, TIFF, RAW\n");
}

/**
 * @brief 打印高度图信息
 */
void print_info(const char* filename, const cf_heightmap_t* heightmap) {
    cf_heightmap_format_t format = cf_heightmap_detect_format(filename);
    
    printf("\n========================================\n");
    printf("文件信息\n");
    printf("========================================\n");
    printf("文件名: %s\n", filename);
    printf("格式: %s\n", cf_heightmap_format_name(format));
    printf("尺寸: %d x %d\n", heightmap->width, heightmap->height);
    printf("总像素: %d\n", heightmap->width * heightmap->height);
    printf("高度范围: %.6f - %.6f\n", heightmap->min_height, heightmap->max_height);
    printf("高度差: %.6f\n", heightmap->max_height - heightmap->min_height);
    
    // 计算统计信息
    double sum = 0.0;
    double sum_sq = 0.0;
    size_t count = (size_t)heightmap->width * heightmap->height;
    
    for (size_t i = 0; i < count; i++) {
        double val = heightmap->data[i];
        sum += val;
        sum_sq += val * val;
    }
    
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double std_dev = sqrt(variance);
    
    printf("平均高度: %.6f\n", mean);
    printf("标准差: %.6f\n", std_dev);
    printf("========================================\n\n");
}

/**
 * @brief 解析RAW格式字符串
 */
cf_raw_format_t parse_raw_format(const char* str) {
    if (strcmp(str, "u8") == 0) return CF_RAW_FORMAT_U8;
    if (strcmp(str, "u16") == 0) return CF_RAW_FORMAT_U16;
    if (strcmp(str, "i16") == 0) return CF_RAW_FORMAT_I16;
    if (strcmp(str, "u32") == 0) return CF_RAW_FORMAT_U32;
    if (strcmp(str, "i32") == 0) return CF_RAW_FORMAT_I32;
    if (strcmp(str, "f32") == 0) return CF_RAW_FORMAT_F32;
    return CF_RAW_FORMAT_U8;
}

/**
 * @brief 导出为RAW格式
 */
int export_raw(const cf_heightmap_t* heightmap, const char* output_path, cf_raw_format_t format) {
    FILE* file = fopen(output_path, "wb");
    if (!file) {
        fprintf(stderr, "错误: 无法创建输出文件: %s\n", output_path);
        return 0;
    }
    
    size_t count = (size_t)heightmap->width * heightmap->height;
    
    for (size_t i = 0; i < count; i++) {
        float value = heightmap->data[i];
        
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
            case CF_RAW_FORMAT_I16: {
                int16_t val = (int16_t)(value * 65535.0f - 32768.0f);
                fwrite(&val, sizeof(int16_t), 1, file);
                break;
            }
            case CF_RAW_FORMAT_U32: {
                uint32_t val = (uint32_t)(value * 4294967295.0);
                fwrite(&val, sizeof(uint32_t), 1, file);
                break;
            }
            case CF_RAW_FORMAT_I32: {
                int32_t val = (int32_t)(value * 4294967295.0 - 2147483648.0);
                fwrite(&val, sizeof(int32_t), 1, file);
                break;
            }
            case CF_RAW_FORMAT_F32: {
                fwrite(&value, sizeof(float), 1, file);
                break;
            }
        }
    }
    
    fclose(file);
    return 1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = NULL;
    const char* output_format = NULL;
    int info_only = 0;
    
    // RAW格式参数
    int raw_width = 0;
    int raw_height = 0;
    cf_raw_format_t raw_format = CF_RAW_FORMAT_U8;
    int is_raw_input = 0;
    
    // 解析命令行参数
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--info") == 0) {
            info_only = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            output_format = argv[++i];
        } else if (strcmp(argv[i], "--raw-width") == 0 && i + 1 < argc) {
            raw_width = atoi(argv[++i]);
            is_raw_input = 1;
        } else if (strcmp(argv[i], "--raw-height") == 0 && i + 1 < argc) {
            raw_height = atoi(argv[++i]);
            is_raw_input = 1;
        } else if (strcmp(argv[i], "--raw-format") == 0 && i + 1 < argc) {
            raw_format = parse_raw_format(argv[++i]);
            is_raw_input = 1;
        }
    }
    
    // 加载高度图
    cf_heightmap_t* heightmap = NULL;
    cf_result_t result;
    
    printf("加载高度图: %s\n", input_file);
    
    if (is_raw_input) {
        if (raw_width <= 0 || raw_height <= 0) {
            fprintf(stderr, "错误: RAW格式需要指定宽度和高度\n");
            return 1;
        }
        printf("  RAW格式: %dx%d\n", raw_width, raw_height);
        result = cf_heightmap_load_raw(input_file, raw_width, raw_height, raw_format, &heightmap);
    } else {
        result = cf_heightmap_load(input_file, &heightmap);
    }
    
    if (result != CF_SUCCESS) {
        fprintf(stderr, "错误: 加载高度图失败 (code: %d)\n", result);
        return 1;
    }
    
    printf("加载成功!\n");
    
    // 显示信息
    if (info_only) {
        print_info(input_file, heightmap);
        cf_heightmap_destroy(heightmap);
        return 0;
    }
    
    // 转换格式
    if (output_file) {
        printf("\n转换格式...\n");
        printf("  输出文件: %s\n", output_file);
        
        // 检测输出格式
        if (!output_format) {
            cf_heightmap_format_t detected = cf_heightmap_detect_format(output_file);
            output_format = cf_heightmap_format_name(detected);
        }
        
        printf("  输出格式: %s\n", output_format);
        
        // 目前只支持RAW导出（PNG/TIFF导出需要额外的编码库）
        if (strcmp(output_format, "RAW") == 0 || strcmp(output_format, "Unknown") == 0) {
            if (export_raw(heightmap, output_file, CF_RAW_FORMAT_F32)) {
                printf("转换成功!\n");
            } else {
                fprintf(stderr, "错误: 导出失败\n");
                cf_heightmap_destroy(heightmap);
                return 1;
            }
        } else {
            fprintf(stderr, "错误: 暂不支持导出为 %s 格式\n", output_format);
            fprintf(stderr, "提示: 当前版本仅支持导出为RAW格式\n");
            cf_heightmap_destroy(heightmap);
            return 1;
        }
    } else {
        // 没有指定输出文件，只显示信息
        print_info(input_file, heightmap);
    }
    
    cf_heightmap_destroy(heightmap);
    return 0;
}
