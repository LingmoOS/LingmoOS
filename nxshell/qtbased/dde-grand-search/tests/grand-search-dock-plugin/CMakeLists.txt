# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME test-dde-grand-search-dock-plugin)

# 查找gmock的cmake文件
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../tests/cmake/modules)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(GMock REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    DBus
    Concurrent
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
)

pkg_check_modules(DdeDockInterface REQUIRED dde-dock)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

set(PROJECT_SRC_PATH "${PROJECT_SOURCE_PATH}/grand-search-dock-plugin")
include_directories(${PROJECT_SRC_PATH}
    ${PROJECT_SOURCE_PATH}/grand-search-dock-plugin/gui)

#项目工程文件
FILE(GLOB_RECURSE PRO_SRC "${PROJECT_SRC_PATH}/*.h" "${PROJECT_SRC_PATH}/*.cpp")

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*.cpp" )

# 源文件
set(SRCS
    main.cpp
    ${GLOBAL_SRC}
    ${PRO_SRC}
    ${UT_SRC}
    ${CPP_STUB_SRC}
    "${PROJECT_3RDPARTY_PATH}/interfaces/grandsearchinterface.h"
    "${PROJECT_3RDPARTY_PATH}/interfaces/grandsearchinterface.cpp"
)

add_executable(${BIN_NAME} ${SRCS} ${QRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    ${DdeDockInterface_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    -lpthread
)

