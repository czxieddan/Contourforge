# 渲染库源文件
set(CF_RENDERING_SOURCES
    renderer.c
    shader.c
    camera.c
    mesh.c
    buffer.c
)

set(CF_RENDERING_HEADERS
    ${CMAKE_SOURCE_DIR}/include/contourforge/rendering.h
)

# 创建渲染库
if(CF_BUILD_SHARED)
    add_library(cf_rendering SHARED ${CF_RENDERING_SOURCES} ${CF_RENDERING_HEADERS})
else()
    add_library(cf_rendering STATIC ${CF_RENDERING_SOURCES} ${CF_RENDERING_HEADERS})
endif()

# 链接库
target_link_libraries(cf_rendering
    PUBLIC
        cf_core
        OpenGL::GL
    # TODO: 添加GLFW和cglm链接
    # glfw
    # cglm
)

# 包含目录
target_include_directories(cf_rendering
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    # TODO: 添加第三方库包含目录
)

# 设置库属性
set_target_properties(cf_rendering PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "contourforge_rendering"
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/contourforge/rendering.h"
)

# Windows特定设置
if(WIN32)
    target_compile_definitions(cf_rendering PRIVATE CF_RENDERING_EXPORTS)
endif()

# 安装
install(TARGETS cf_rendering
    EXPORT ContourforgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/contourforge
)
