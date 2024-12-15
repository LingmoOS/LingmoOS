cmake_minimum_required(VERSION 3.1)

set(PRELOAD_LIB_NAME ${CMAKE_PROJECT_NAME}-preload)

###! 请确保 preloadplugin 下仅依赖最少的三方内容，
###! 且不做额外的复杂操作，将第三方库和自定义类型放在
###! maincomponentplugin 中，保证程序能够快速启动。
# Set sources
set(PRELOAD_SRC
    preloadplugin.cpp
)

# Find the QtQuick library
find_package(Qt${QT_VERSION_MAJOR}Quick CONFIG REQUIRED)
if(${QT_VERSION_MAJOR} EQUAL 5)
    find_package(Qt5QuickCompiler REQUIRED)
    qtquick_compiler_add_resources(PRELOAD_RCC_SOURCES preload.qrc)
else()
    qt_add_resources(PRELOAD_RCC_SOURCES preload.qrc)
    set(DTK_VERSION_MAJOR ${QT_VERSION_MAJOR})
endif()

# 保证 preloadplugin 目录下头文件全局可见
include_directories(src/preloadplugin)

# Add LIB
add_library(${PRELOAD_LIB_NAME} SHARED
    ${PRELOAD_SRC}
    ${PRELOAD_RCC_SOURCES}
)

# 由于 LIB 程序为本项目的功能实现部分，需要用到的第三方库都需要在此处添加。
target_link_libraries(${PRELOAD_LIB_NAME}
    Qt${QT_VERSION_MAJOR}::Quick
    Dtk${DTK_VERSION_MAJOR}::Declarative
)

# Add install rule
install(TARGETS ${PRELOAD_LIB_NAME} DESTINATION ${DTK_QML_APP_PLUGIN_PATH})
