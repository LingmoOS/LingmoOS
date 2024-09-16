# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)

set(BIN_NAME dde_launcher_unit_test)

# 自动生成moc文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D SANITIZER_CHECK)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O2")
endif()

# 源文件
file(GLOB_RECURSE SRCS "*.h" "*.cpp" "/widget_unit_test/*.cpp" "/view_unit_test/*.cpp" "/worker_unit_test/*.cpp" "/global_util_unit_test/*cpp" "/boxfram_unit_test/*.cpp" "/dbusinterface_unit_test/*.cpp" test_res.qrc ${CMAKE_CURRENT_SOURCE_DIR}/../src/skin.qrc)
# 查找依赖库
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5X11Extras REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(Qt5Svg REQUIRED)
#QTest 依赖库
find_package(Qt5 COMPONENTS Test REQUIRED)
find_package(GTest REQUIRED)
find_package(DtkCore REQUIRED)

pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(XCB_EWMH REQUIRED xcb-ewmh)

include_directories(
    ../src
    ../src/boxframe
    ../src/dbusinterface
    ../src/dbusinterface/dbusvariant
    ../src/dbusservices
    ../src/delegate
    ../src/global_util
    ../src/model
    ../src/skin
    ../src/view
    ../src/widgets
    ../src/worker
)

aux_source_directory(../src SRC)
aux_source_directory(../src/boxframe BOXFRAME)
aux_source_directory(../src/dbusinterface DBUSINTERFACE)
aux_source_directory(../src/dbusinterface/dbusvariant DBUSVARIANT)
aux_source_directory(../src/dbusservices DBUSSERVICES)
aux_source_directory(../src/delegate DELEGATE)
aux_source_directory(../src/global_util GLOBAL_UTIL)
aux_source_directory(../src/model MODEL)
aux_source_directory(../src/skin SKIN)
aux_source_directory(../src/view VIEW)
aux_source_directory(../src/widgets WIDGETS)
aux_source_directory(../src/worker WORKER)

file(GLOB SRC_PATH
    ${SRC}
    ${BOXFRAME}
    ${DBUSINTERFACE}
    ${DBUSVARIANT}
    ${DBUSSERVICES}
    ${DELEGATE}
    ${GLOBAL_UTIL}
    ${MODEL}
    ${SKIN}
    ${VIEW}
    ${WIDGETS}
    ${WORKER}
)
list(REMOVE_ITEM SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp)

# 添加执行文件信息
#${LAUNCHER}
add_executable(${BIN_NAME} ${SRCS} ${SRC_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../src/widgets/images.qrc)

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt5Test_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${GTEST_LIBRARIES}
    -lpthread
    -lm
)

add_custom_target(check)

add_custom_command(TARGET check
    COMMAND ./${BIN_NAME}
    )

add_dependencies(check ${BIN_NAME})
