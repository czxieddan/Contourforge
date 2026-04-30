/**
 * @file version.c
 * @brief Contourforge版本信息实现
 */

#include "contourforge/types.h"
#include <stdio.h>
#include <stdbool.h>

/* 版本信息 */
#define CF_VERSION_MAJOR 1
#define CF_VERSION_MINOR 0
#define CF_VERSION_PATCH 0

/* 初始化状态 */
static bool g_initialized = false;

/**
 * @brief 获取版本字符串
 */
const char* cf_get_version_string(void) {
    static char version_str[32];
    snprintf(version_str, sizeof(version_str), "%d.%d.%d",
             CF_VERSION_MAJOR, CF_VERSION_MINOR, CF_VERSION_PATCH);
    return version_str;
}

/**
 * @brief 获取版本字符串（别名）
 */
const char* cf_get_version(void) {
    return cf_get_version_string();
}

/**
 * @brief 获取主版本号
 */
int cf_get_version_major(void) {
    return CF_VERSION_MAJOR;
}

/**
 * @brief 获取次版本号
 */
int cf_get_version_minor(void) {
    return CF_VERSION_MINOR;
}

/**
 * @brief 获取补丁版本号
 */
int cf_get_version_patch(void) {
    return CF_VERSION_PATCH;
}

/**
 * @brief 初始化Contourforge库
 */
cf_result_t cf_init(void) {
    if (g_initialized) {
        return CF_ERROR_ALREADY_INITIALIZED;
    }
    
    g_initialized = true;
    return CF_SUCCESS;
}

/**
 * @brief 清理Contourforge库
 */
void cf_cleanup(void) {
    g_initialized = false;
}

/**
 * @brief 检查是否已初始化
 */
bool cf_is_initialized(void) {
    return g_initialized;
}
