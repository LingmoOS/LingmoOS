cmake_minimum_required(VERSION 3.16)

project(lingmo-qt-plugins)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(platformtheme)
add_subdirectory(widgetstyle)
