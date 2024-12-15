cmake_minimum_required(VERSION 3.1)

set(MAINCOMPONENT_LIB_NAME ${CMAKE_PROJECT_NAME}-main)

###! maincomponent 中可以添加应用的第三方依赖、注册
###! 应用的c++和qml类型、进行耗时计算和加载等等。
###! 但为了最大程度的启动（创建）组件，请保证在 main.qml
###! 中 AppLoader 结构的正确性，将互相独立的组件独立成
###! 不同的 Component 进行并发加载。

# Set sources
set(MAINCOMPONENNT_SRC
    maincomponentplugin.cpp
)

# Find the QtQuick library
find_package(Qt${QT_VERSION_MAJOR}Quick CONFIG REQUIRED)
if(${QT_VERSION_MAJOR} EQUAL 5)
    find_package(Qt5QuickCompiler REQUIRED)
    qtquick_compiler_add_resources(MAINCOMPONENT_RCC_SOURCES maincomponent.qrc)
else()
    qt_add_resources(MAINCOMPONENT_RCC_SOURCES maincomponent.qrc)
    set(DTK_VERSION_MAJOR ${QT_VERSION_MAJOR})
endif()

# 保证 maincomponentplugin 目录下头文件全局可见
include_directories(src/maincomponentplugin)

# Add LIB
add_library(${MAINCOMPONENT_LIB_NAME} SHARED
    ${MAINCOMPONENNT_SRC}
    ${MAINCOMPONENT_RCC_SOURCES}
)

# 由于 LIB 程序为本项目的功能实现部分，需要用到的第三方库都需要在此处添加。
target_link_libraries(${MAINCOMPONENT_LIB_NAME}
    Qt${QT_VERSION_MAJOR}::Quick
    Dtk${DTK_VERSION_MAJOR}::Declarative
)

# Add install rule
install(TARGETS ${MAINCOMPONENT_LIB_NAME} DESTINATION ${DTK_QML_APP_PLUGIN_PATH})
