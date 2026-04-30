# 核心库源文件
set(CF_CORE_SOURCES
    memory.c
    data_structures.c
    octree.c
    version.c
)

set(CF_CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/contourforge/core.h
    ${CMAKE_SOURCE_DIR}/include/contourforge/types.h
)

# 创建核心库
if(CF_BUILD_SHARED)
    add_library(cf_core SHARED ${CF_CORE_SOURCES} ${CF_CORE_HEADERS})
else()
    add_library(cf_core STATIC ${CF_CORE_SOURCES} ${CF_CORE_HEADERS})
endif()

# 包含目录
target_include_directories(cf_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置库属性
set_target_properties(cf_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "contourforge_core"
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/contourforge/core.h;${CMAKE_SOURCE_DIR}/include/contourforge/types.h"
)

# Windows特定设置
if(WIN32)
    target_compile_definitions(cf_core PRIVATE CF_CORE_EXPORTS)
endif()

# 安装
install(TARGETS cf_core
    EXPORT ContourforgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/contourforge
)
