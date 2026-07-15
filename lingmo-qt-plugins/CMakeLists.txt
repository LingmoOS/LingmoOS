cmake_minimum_required(VERSION 3.16)

project(lingmo-qt-plugins)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(BUILD_PLATFORMTHEME "Build the Lingmo Qt platform theme plugin" ON)
option(BUILD_WIDGETSTYLE "Build the legacy Lingmo Qt widget style" OFF)

if(BUILD_PLATFORMTHEME)
    add_subdirectory(platformtheme)
endif()

if(BUILD_WIDGETSTYLE)
    add_subdirectory(widgetstyle)
endif()
