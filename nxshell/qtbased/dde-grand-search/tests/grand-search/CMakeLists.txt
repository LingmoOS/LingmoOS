# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME test-dde-grand-search)

# 应用库目录
set(LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${BIN_NAME}")

# 插件目录
set(PLUGIN_BASE_DIR "${LIB_BASE_DIR}/plugins")

# DBus接口与服务
set(3RD_PARTY_PLUGIN
    "${PROJECT_3RDPARTY_PATH}/interfaces/daemongrandsearchinterface.h"
    "${PROJECT_3RDPARTY_PATH}/interfaces/daemongrandsearchinterface.cpp"
    "${PROJECT_3RDPARTY_PATH}/services/grandsearchserviceadaptor.h"
    "${PROJECT_3RDPARTY_PATH}/services/grandsearchserviceadaptor.cpp")

# 扩展搜索插件目录
set(PLUGIN_SEARCHER_DIR "${PLUGIN_BASE_DIR}/searcher")
add_definitions(-DPLUGIN_SEARCHER_DIR="${PLUGIN_SEARCHER_DIR}")

# 预览插件目录
set(PLUGIN_PREVIEW_DIR "${PLUGIN_BASE_DIR}/preview")
add_definitions(-DPLUGIN_PREVIEW_DIR="${PLUGIN_PREVIEW_DIR}")

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
REQUIRED)

pkg_check_modules(GIO_LIB REQUIRED
        gio-unix-2.0
        glib-2.0
)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
)

set(PROJECT_SRC_PATH "${PROJECT_SOURCE_PATH}/grand-search")
include_directories(${PROJECT_SRC_PATH})

#项目工程文件
FILE(GLOB_RECURSE PRO_SRC "${PROJECT_SRC_PATH}/*/*.h" "${PROJECT_SRC_PATH}/*/*.cpp")

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*/*.cpp")

# 源文件
set(SRCS
    main.cpp
    ${GLOBAL_SRC}
    ${PRO_SRC}
    ${UT_SRC}
    ${CPP_STUB_SRC}
    ${3RD_PARTY_PLUGIN}
)

add_executable(${BIN_NAME} ${SRCS})
target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${GIO_LIB_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${GIO_LIB_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    -lpthread
)


