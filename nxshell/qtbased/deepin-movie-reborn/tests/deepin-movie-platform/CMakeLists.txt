# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#ADD_DEFINITIONS( -LITE_DIV)
execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

option (USE_TEST "Use TESTS" ON)
add_definitions( -DUSE_TEST )

# 设置Qt模块
set(QtModule Core Gui Widgets Network X11Extras PrintSupport DBus Sql Svg Multimedia MultimediaWidgets Concurrent LinguistTools Test)

# 设置工程名字
project(deepin-movie-platform-test)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})
find_package(gui-private)


# 包含源码文件夹
include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(../../src)
include_directories(../../src/common)
include_directories(../../src/common/platform)
include_directories(../../src/widgets)
include_directories(../../src/widgets/platform)
include_directories(../../src/libdmr)
include_directories(../../src/vendor)
include_directories(../../src/backends)
include_directories(../../src/backends/mpv)
include_directories(../../src/backends/mediaplayer)

#add_subdirectory(googletest)

# 定义 source 文件
FILE (GLOB allSource
    ../../src/common/*.cpp
    ../../src/common/platform/*.cpp
    ../../src/widgets/*.cpp
    ../../src/widgets/platform/*.cpp
    ../../src/libdmr/*.cpp
    ../../src/vendor/*.cpp
    ../../src/backends/mpv/*.cpp
    ../../src/backends/mediaplayer/*.cpp
    ../../src/backends/*.cpp
    )

FILE (GLOB allTestSource
    *.cpp
    *.sh
    common/*.cpp
    widgets/*.cpp
    libdmr/*.cpp
    backends/*.cpp
    vendor/*.cpp
    *.sh
    )

# 定义资源文件
set(AllQRC
    ../../assets/resources.qrc
    ../../assets/icons/theme-icons.qrc
)

# 设置不删除生成的文件夹内容文件（设置此项是为了翻译后的文件不被删除）
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

# 添加需要安装的模块
add_executable(${PROJECT_NAME} ${allSource} ${allTestSource}
    ${AllQRC}
    )

# 添加gocv覆盖率文件的输出
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")    #mips平台下不进行内存检测
else()
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/objects)

add_custom_target(platform_test
    COMMAND mkdir -p build-ut
    COMMAND mkdir -p build-ut/report
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_command(TARGET platform_test
    COMMAND echo " ===================CREAT LCOV REPROT==================== "
    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
    COMMAND ./tests/${PROJECT_NAME} #-o ./build-ut/report/report_${PROJECT_NAME}.xml,xml
    COMMAND lcov --directory . --capture --output-file ./build-ut/${PROJECT_NAME}_Coverage.info

    COMMAND echo " =================== do filter begin ==================== "
    COMMAND lcov --remove ./build-ut/${PROJECT_NAME}_Coverage.info 'CMakeFiles/${PROJECT_NAME}.dir/${PROJECT_NAME}_autogen/*/*'
    '${PROJECT_NAME}_autogen/*/*/*.cpp' 'googletest/*/*' '*/usr/include/*' '*/tests/*' '/usr/local/*' '*/src/common/filter.*'
    '*/src/common/settings_translation.cpp' '*/src/common/event_monitor.cpp' '*/src/widgets/videoboxbutton.cpp' '*/src/common/event_relayer.cpp'
    -o ./build-ut/${PROJECT_NAME}_Coverage_fileter.info
    COMMAND echo " =================== do filter end ==================== "

    COMMAND genhtml -o ./build-ut/report ./build-ut/${PROJECT_NAME}_Coverage_fileter.info
    COMMAND echo " -- Coverage files have been output to ${CMAKE_BINARY_DIR}/build-ut/report "
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
add_dependencies(platform_test ${PROJECT_NAME})

#------------------------------ 添加第三方库  DTK -------------------------------
# 使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)
#set(TARGET_LIBS X11 Xext Xtst PkgConfig::Xcb Qt5::Widgets Qt5::X11Extras Qt5::Network
#    Qt5::Concurrent Qt5::DBus Qt5::Sql PkgConfig::Dtk
#    PkgConfig::Mpv PkgConfig::MPRIS PkgConfig::DBusextended
#    PkgConfig::AV pthread GL)
# 检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkgui
        libpulse dvdnav gsettings-qt x11 xext xtst xcb gl
        xcb-aux xcb-proto xcb-ewmh xcb-shape mpris-qt5 dbusextended-qt5 libva libva-x11
        gstreamer-1.0 glib-2.0 gstreamer-pbutils-1.0
        )
    #libpulse-simple xcb-aux gtest
# 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})

# 将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES}  Qt5::GuiPrivate)
#------------------------------添加第三方库end-------------------------------------

target_link_libraries(${PROJECT_NAME} gmock gmock_main gtest gtest_main)

# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})
