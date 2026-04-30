/**
 * @file version.c
 * @brief 版本信息实现
 */

#include <contourforge/contourforge.h>

const char* cf_get_version(void)
{
    return CONTOURFORGE_VERSION_STRING;
}

void cf_get_version_number(int* major, int* minor, int* patch)
{
    if (major) *major = CONTOURFORGE_VERSION_MAJOR;
    if (minor) *minor = CONTOURFORGE_VERSION_MINOR;
    if (patch) *patch = CONTOURFORGE_VERSION_PATCH;
}
