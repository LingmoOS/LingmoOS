# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(APP_QRC "../dde-introduction.qrc")

ADD_COMPILE_OPTIONS(-fno-access-control)

# 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")

find_package(Qt5Test REQUIRED)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(src)

set(PROJECT_NAME_TEST
    ${PROJECT_NAME}-test)

#Include all app own subdirectorys

file(GLOB_RECURSE INTRODUCTION_SRC_TEST ${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp)
file(GLOB_RECURSE INTRODUCTION_SRC_TEST1 ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

list(REMOVE_ITEM INTRODUCTION_SRC_TEST "${CMAKE_CURRENT_LIST_DIR}/../src/main.cpp")
#ADD_DEFINITIONS("-D DISABLE_VIDEO")
#LIST(REMOVE_ITEM INTRODUCTION_SRC_TEST "${CMAKE_CURRENT_LIST_DIR}/../src/modules/videowidget.cpp")
#LIST(REMOVE_ITEM INTRODUCTION_SRC_TEST1 "${CMAKE_CURRENT_LIST_DIR}/src/modules/ut_videowidget.cpp")


if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    ADD_DEFINITIONS("-D DISABLE_VIDEO")
    LIST(REMOVE_ITEM INTRODUCTION_SRC_TEST ${CMAKE_CURRENT_LIST_DIR}/../src/modules/videowidget.cpp)
    LIST(REMOVE_ITEM INTRODUCTION_SRC_TEST1 ${CMAKE_CURRENT_LIST_DIR}/src/modules/ut_videowidget.cpp)
endif()

# 生成测试可执行程序
add_executable(${PROJECT_NAME_TEST}
    ${INTRODUCTION_SRC_TEST}
    ${INTRODUCTION_SRC_TEST1}
    ${APP_QRC}
    )

# 生成测试可执行程序的依赖库QApplication
target_link_libraries(${PROJECT_NAME_TEST} 
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${LIBDMR_LIBRARIES}
    ${GTEST_LIBRARYS}
    ${GTEST_MAIN_LIBRARYS}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Multimedia
    gtest
    gtest_main
    -pthread
)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    pkg_check_modules(LIBDMR REQUIRED libdmr)
    include_directories(${LIBDMR_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME_TEST} ${LIBDMR_LIBRARIES})
endif()

target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${DtkWidget_INCLUDE_DIRS}
                                                       ${DtkCore_INCLUDE_DIRS}
                                                       ${DtkGui_INCLUDE_DIRS})

# ------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test)
add_custom_command(TARGET test
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND ${PROJECT_NAME_TEST}
    )
add_dependencies(test ${PROJECT_NAME_TEST})

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")

#ASAN安全性检测标志
set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")
if(CMAKE_SAFETYTEST STREQUAL "")
    set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")
if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  #安全测试选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
endif()
