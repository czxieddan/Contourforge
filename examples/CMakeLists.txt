# 示例程序

# 简单查看器
add_executable(simple_viewer simple_viewer.c)
target_link_libraries(simple_viewer
    cf_core
    cf_rendering
    cf_datagen
    glfw
)

# 高度图加载器
add_executable(heightmap_loader heightmap_loader.c)
target_link_libraries(heightmap_loader
    cf_core
    cf_datagen
    cf_rendering
    glfw
)

# 交互编辑器
add_executable(interactive_editor interactive_editor.c)
target_link_libraries(interactive_editor
    cf_core
    cf_rendering
    cf_datagen
    cf_control
    glfw
)

# LOD演示
add_executable(lod_demo lod_demo.c)
target_link_libraries(lod_demo
    cf_core
    cf_rendering
    cf_datagen
    glfw
)

# 复制着色器文件到构建目录
add_custom_command(TARGET simple_viewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:simple_viewer>/shaders
    COMMENT "Copying shaders to build directory"
)

add_custom_command(TARGET interactive_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:interactive_editor>/shaders
    COMMENT "Copying shaders to build directory"
)

add_custom_command(TARGET lod_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:lod_demo>/shaders
    COMMENT "Copying shaders to build directory"
)

# 多线程性能测试
add_executable(threading_benchmark threading_benchmark.c)
target_link_libraries(threading_benchmark
    cf_core
    cf_datagen
)
if(NOT WIN32)
    target_link_libraries(threading_benchmark Threads::Threads)
endif()

# 安装示例程序
install(TARGETS simple_viewer heightmap_loader interactive_editor lod_demo threading_benchmark
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)
