# 测试程序

# 核心模块测试
add_executable(test_core test_core.c)
target_link_libraries(test_core cf_core)
add_test(NAME test_core COMMAND test_core)

# 内存管理测试
add_executable(test_memory test_memory.c)
target_link_libraries(test_memory cf_core)
add_test(NAME test_memory COMMAND test_memory)

# 八叉树测试
add_executable(test_octree test_octree.c)
target_link_libraries(test_octree cf_core)
add_test(NAME test_octree COMMAND test_octree)

# 数据生成测试
add_executable(test_datagen test_datagen.c)
target_link_libraries(test_datagen cf_core cf_datagen)
add_test(NAME test_datagen COMMAND test_datagen)

# 等高线提取测试
add_executable(test_contour test_contour.c)
target_link_libraries(test_contour cf_core cf_datagen)
add_test(NAME test_contour COMMAND test_contour)

# 简化算法测试
add_executable(test_simplify test_simplify.c)
target_link_libraries(test_simplify cf_core cf_datagen)
add_test(NAME test_simplify COMMAND test_simplify)

# 渲染器测试（需要OpenGL上下文）
# add_executable(test_renderer test_renderer.c)
# target_link_libraries(test_renderer cf_core cf_rendering)
# add_test(NAME test_renderer COMMAND test_renderer)

message(STATUS "Tests configured. Run 'ctest' to execute all tests.")
