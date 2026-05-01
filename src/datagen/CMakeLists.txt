# 数据生成库源文件
set(CF_DATAGEN_SOURCES
    heightmap.c
    contour.c
    simplify.c
    topology.c
    contour_parallel.c
    simplify_parallel.c
    tiff_loader.c
    raw_loader.c
    format_detect.c
)

set(CF_DATAGEN_HEADERS
    ${CMAKE_SOURCE_DIR}/include/contourforge/datagen.h
)

# 创建数据生成库
if(CF_BUILD_SHARED)
    add_library(cf_datagen SHARED ${CF_DATAGEN_SOURCES} ${CF_DATAGEN_HEADERS})
else()
    add_library(cf_datagen STATIC ${CF_DATAGEN_SOURCES} ${CF_DATAGEN_HEADERS})
endif()

# 链接库
target_link_libraries(cf_datagen
    PUBLIC
        cf_core
)

# 包含目录
target_include_directories(cf_datagen
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${STB_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/tinytiff
)

# 设置库属性
set_target_properties(cf_datagen PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "contourforge_datagen"
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/contourforge/datagen.h"
)

# Windows特定设置
if(WIN32)
    target_compile_definitions(cf_datagen PRIVATE CF_DATAGEN_EXPORTS)
endif()

# 安装
install(TARGETS cf_datagen
    EXPORT ContourforgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/contourforge
)
