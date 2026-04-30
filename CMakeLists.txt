cmake_minimum_required(VERSION 3.15)
project(Contourforge VERSION 1.0.0 LANGUAGES C)

# C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 构建选项
option(CF_BUILD_SHARED "Build shared libraries" ON)
option(CF_BUILD_TESTS "Build tests" ON)
option(CF_BUILD_EXAMPLES "Build examples" ON)
option(CF_BUILD_DOCS "Build documentation" OFF)
option(CF_ENABLE_SIMD "Enable SIMD optimizations" ON)
option(CF_ENABLE_OPENMP "Enable OpenMP" ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 平台检测
if(WIN32)
    set(CF_PLATFORM_WINDOWS TRUE)
    add_definitions(-DCF_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(CF_PLATFORM_LINUX TRUE)
    add_definitions(-DCF_PLATFORM_LINUX)
elseif(APPLE)
    set(CF_PLATFORM_MACOS TRUE)
    add_definitions(-DCF_PLATFORM_MACOS)
endif()

# 编译器警告
if(MSVC)
    add_compile_options(/W4)
    add_compile_options(/wd4996)  # 禁用不安全函数警告
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-Wno-unused-parameter)
endif()

# 优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /Ob2 /Oi /Ot)
    else()
        add_compile_options(-O3 -march=native)
    endif()
endif()

# SIMD支持
if(CF_ENABLE_SIMD)
    if(MSVC)
        add_compile_options(/arch:AVX2)
    else()
        add_compile_options(-mavx2 -mfma)
    endif()
    add_definitions(-DCF_ENABLE_SIMD)
endif()

# OpenMP支持
if(CF_ENABLE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_C_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    endif()
endif()

# 依赖管理
# 注意：这里假设第三方库已经安装或通过子模块添加
# 实际使用时需要配置第三方库路径

# 查找OpenGL
find_package(OpenGL REQUIRED)

# 添加GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)

# 添加glad（header-only）
add_library(glad INTERFACE)
target_include_directories(glad INTERFACE third_party/glad/include)

# 单头文件库路径
set(STB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/stb)

# 测试框架
if(CF_BUILD_TESTS)
    # add_subdirectory(third_party/unity)
endif()

# 添加库子目录
add_subdirectory(src/core)
add_subdirectory(src/rendering)
add_subdirectory(src/datagen)
add_subdirectory(src/control)

# 示例程序
if(CF_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 测试
if(CF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 文档
if(CF_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)
        
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# 安装配置
include(GNUInstallDirs)

# 安装头文件
install(DIRECTORY include/contourforge
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# 安装着色器
install(DIRECTORY shaders
    DESTINATION ${CMAKE_INSTALL_DATADIR}/contourforge
    FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag"
)

# CPack配置
set(CPACK_PACKAGE_NAME "Contourforge")
set(CPACK_PACKAGE_VENDOR "Contourforge Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-performance 3D contour rendering library")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Contourforge")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
endif()

if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

# 打印配置信息
message(STATUS "")
message(STATUS "Contourforge Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "  Build shared: ${CF_BUILD_SHARED}")
message(STATUS "  Build tests: ${CF_BUILD_TESTS}")
message(STATUS "  Build examples: ${CF_BUILD_EXAMPLES}")
message(STATUS "  Enable SIMD: ${CF_ENABLE_SIMD}")
message(STATUS "  Enable OpenMP: ${CF_ENABLE_OPENMP}")
message(STATUS "")
