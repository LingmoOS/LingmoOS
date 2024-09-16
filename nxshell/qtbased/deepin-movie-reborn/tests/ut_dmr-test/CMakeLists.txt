# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 设置工程名字
project(ut_dmr-test)

# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

option (USE_TEST "Use TESTS" ON)
add_definitions( -DUSE_TEST )

# 设置Qt模块
set(QtModule Core Gui Widgets Network X11Extras PrintSupport DBus Sql Svg Concurrent LinguistTools Test)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})

pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkwidget)

include_directories(../../examples/test)
include_directories(../../src)
include_directories(../../src/common)
include_directories(../../src/widgets)
include_directories(../../src/libdmr)
include_directories(../../src/vendor)
include_directories(../../src/backends)
include_directories(../../src/backends/mpv)

# 定义 source 文件
FILE (GLOB allSource
    ../../examples/test/window.cpp
    )

FILE (GLOB allTestSource
    *.cpp
    *.sh
    )

# 定义资源文件
set(AllQRC
    ../../assets/resources.qrc
    ../../assets/icons/theme-icons.qrc
)

add_executable(${PROJECT_NAME} ${allSource} ${allTestSource} ${AllQRC})

#添加内存泄露检测参数
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")    #mips平台下不进行内存检测
else()
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/objects)

# 使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkgui
        libpulse dvdnav gsettings-qt x11 xext xtst xcb gl
        xcb-aux xcb-proto xcb-ewmh xcb-shape mpris-qt5 dbusextended-qt5 libva libva-x11
        )
    #libpulse-simple xcb-aux gtest
# 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})

# 将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} )
#------------------------------添加第三方库end-------------------------------------

target_link_libraries(${PROJECT_NAME} gmock gmock_main gtest gtest_main dmr)

# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})

