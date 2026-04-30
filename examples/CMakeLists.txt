# 示例程序

# 简单查看器
add_executable(simple_viewer simple_viewer.c)
target_link_libraries(simple_viewer
    cf_core
    cf_rendering
    cf_datagen
)

# 高度图加载器
add_executable(heightmap_loader heightmap_loader.c)
target_link_libraries(heightmap_loader
    cf_core
    cf_datagen
)

# 交互编辑器
add_executable(interactive_editor interactive_editor.c)
target_link_libraries(interactive_editor
    cf_core
    cf_rendering
    cf_datagen
    cf_control
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

# 安装示例程序
install(TARGETS simple_viewer heightmap_loader interactive_editor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)
