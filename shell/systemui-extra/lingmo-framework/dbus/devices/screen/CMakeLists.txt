cmake_minimum_required(VERSION 3.5)
project(screen VERSION 1.0)  

find_package(PkgConfig)


pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})

cmake_minimum_required(VERSION 3.16)
# 查找Qt5及其模块
find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Core DBus Widgets)
  

set(DEVICES_SCREEN_LINK
    glib-2.0
    gio-2.0
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GOBJECT_LIBRARIES}
)


# 索引动态库的头文件地址
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)

file(GLOB LIBSCREEN_SOURCES src/*.cpp src/QGSettings/*.cpp)

add_library(screen STATIC ${LIBSCREEN_SOURCES})  

target_link_libraries(screen
    PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    ${DEVICES_SCREEN_LINK})
