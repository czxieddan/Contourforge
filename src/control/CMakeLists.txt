# 控制库源文件
set(CF_CONTROL_SOURCES
    input.c
    selection.c
    editor.c
)

set(CF_CONTROL_HEADERS
    ${CMAKE_SOURCE_DIR}/include/contourforge/control.h
)

# 创建控制库
if(CF_BUILD_SHARED)
    add_library(cf_control SHARED ${CF_CONTROL_SOURCES} ${CF_CONTROL_HEADERS})
else()
    add_library(cf_control STATIC ${CF_CONTROL_SOURCES} ${CF_CONTROL_HEADERS})
endif()

# 链接库
target_link_libraries(cf_control
    PUBLIC
        cf_core
        cf_rendering
)

# 包含目录
target_include_directories(cf_control
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置库属性
set_target_properties(cf_control PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "contourforge_control"
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/contourforge/control.h"
)

# Windows特定设置
if(WIN32)
    target_compile_definitions(cf_control PRIVATE CF_CONTROL_EXPORTS)
endif()

# 安装
install(TARGETS cf_control
    EXPORT ContourforgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/contourforge
)
