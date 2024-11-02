cmake_minimum_required(VERSION 3.16)

project(test-widget VERSION 4.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Widgets REQUIRED)

set(PLUGIN_SOURCE
        test-plugin.cpp
        test-plugin.h
)
add_library(test-plugin MODULE ${PLUGIN_SOURCE} )

target_link_libraries(test-plugin
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        lingmo-quick::framework
)