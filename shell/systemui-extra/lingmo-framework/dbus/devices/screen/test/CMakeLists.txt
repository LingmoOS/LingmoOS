project(my_qt_app)



find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(../src)
cmake_minimum_required(VERSION 3.16)
# 查找Qt5及其模块
find_package(Qt5 COMPONENTS Core Widgets DBus REQUIRED)  
  
# 接下来，你可以使用Qt5::Core, Qt5::Widgets, Qt5::DBus来链接你的可执行文件或库  
set(CMAKE_AUTOMOC ON)

# 示例：添加可执行文件  
add_executable(my_qt_app
    test.cpp
    ../src/screen.cpp
    ../src/QGSettings/qgsettings.cpp
    ../src/QGSettings/qconftype.cpp)



target_include_directories(my_qt_app PRIVATE)

# 链接Qt模块  
target_link_libraries(my_qt_app
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    glib-2.0
    gio-2.0
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GOBJECT_LIBRARIES})
