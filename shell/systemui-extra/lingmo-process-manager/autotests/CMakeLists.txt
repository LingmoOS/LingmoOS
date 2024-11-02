cmake_minimum_required(VERSION 3.5)

project(AutoTests LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
add_definitions(-DQT_NO_KEYWORDS)
find_package(PkgConfig REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

pkg_check_modules(LINGMOSDKWAYLANDHELPER_PKG lingmosdk-waylandhelper)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)

include_directories(${GIO_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../base)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../utils)

add_executable(test_appinfomanager
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/desktopfilemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/selection/selectionchecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/appinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_appinfomanager.cpp
)

add_executable(test_configmanager
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    tst_configmanager.cpp
)

add_executable(test_processinfohelper
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    tst_processinfohelper.cpp
)

add_executable(test_exceptiongroupprocesswatcher
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/exceptiongroupprocesswatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/processinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/desktopfilemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/selection/selectionchecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/appinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_exceptiongroupprocesswatcher.cpp
)

add_executable(test_timewheel
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.cpp
    tst_timewheel.cpp
)

add_executable(test_misc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_misc.cpp
)

add_executable(test_appinfohelper
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    tst_appinfohelper.cpp
)

add_executable(test_appchooser
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appchooser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appchooser.cpp
    tst_appchooser.cpp
)


add_test(NAME AutoTests COMMAND AutoTests)
target_include_directories(test_appinfomanager PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_INCLUDE_DIRS})
target_link_directories(test_appinfomanager PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARY_DIRS})

target_include_directories(test_exceptiongroupprocesswatcher PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_INCLUDE_DIRS})
target_link_directories(test_exceptiongroupprocesswatcher PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARY_DIRS})

target_include_directories(test_appinfohelper PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_INCLUDE_DIRS})
target_link_directories(test_appinfohelper PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARY_DIRS})

target_link_libraries(
    test_appinfomanager PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    KF5::KIOCore
    KF5::WindowSystem
    ${GSETTINGQT_LIBRARIES}
    ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
    pthread
    procps
    X11
)

target_link_libraries(
    test_configmanager PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GSETTINGQT_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
)

target_link_libraries(
    test_processinfohelper PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GIO_LIBRARIES}
    procps
)

target_link_libraries(
    test_exceptiongroupprocesswatcher PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    KF5::KIOCore
    KF5::WindowSystem
    ${GSETTINGQT_LIBRARIES}
    ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
    pthread
    procps
    X11
)

target_link_libraries(
    test_timewheel PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(
    test_misc PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(
    test_appinfohelper PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${GIO_LIBRARIES}
    KF5::WindowSystem
    KF5::KIOCore
    ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARIES}
)

target_link_libraries(
    test_appchooser PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GIO_LIBRARIES}
)
