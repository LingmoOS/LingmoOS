cmake_minimum_required(VERSION 3.7)

set(VERSION "6.0.0" CACHE STRING "Application version")

project(deepin-editor
    VERSION ${VERSION}
    DESCRIPTION "Text editor for DDE"
    HOMEPAGE_URL "https://github.com/linuxdeepin/deepin-editor"
    LANGUAGES CXX C
)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

# 设置软件版本
include_directories(${PROJECT_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/src/environments.h.in ${PROJECT_BINARY_DIR}/environments.h @ONLY)

# 玲珑路径配置
add_definitions(-DLINGLONG_PREFIX=\"${CMAKE_INSTALL_PREFIX}/\")
# 高亮文件路径配置
set(KF5_HIGHLIGHT_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/share/deepin-editor/org.kde.syntax-highlighing)
add_definitions(-DKF5_HIGHLIGHT_PATH=\"${KF5_HIGHLIGHT_INSTALL_PATH}\")

add_subdirectory(src)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
endif()
