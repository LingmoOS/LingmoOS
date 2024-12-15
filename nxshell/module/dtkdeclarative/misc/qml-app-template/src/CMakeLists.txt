cmake_minimum_required(VERSION 3.1)

# 同时生成 一个 exe 和 两个 lib 三个文件
# lib 文件包含 Preload 插件和 MainComponent 插件
# (1) Preload 插件保证程序启动的入口。
# (2) MainCompnent 会在加载完毕自动添加到 Preload 中

# Application
if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
    add_definitions(-DPLUGINPATH="${LIBRARY_OUTPUT_PATH}")
endif()

add_subdirectory(main)
add_subdirectory(preloadplugin)
add_subdirectory(maincomponentplugin)
