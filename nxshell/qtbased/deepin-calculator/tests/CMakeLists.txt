# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#源程序cmakelists再添加上ut相关环境搭建
#ut相关环境搭建详情见
#https://wikidev.uniontech.com/index.php?title=UT_%E6%B5%8B%E8%AF%95%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA#.E7.94.A8.E4.BE.8B.E7.BC.96.E5.86.99_.7C_UT_Case

cmake_minimum_required(VERSION 3.9.5)

if (NOT DEFINED VERSION)
    set(VERSION 1.2.2)
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-needed -fPIE")
set(QT_MINIMUM_VERSION "5.7.1")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

#add_definitions(-DQT_NO_DEBUG_OUTPUT)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    message(STATUS "this system is mips64=========")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-needed")
endif()

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

    execute_process(COMMAND bash "${CMAKE_CURRENT_LIST_DIR}/../translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../)
endif ()

configure_file(${CMAKE_CURRENT_LIST_DIR}/../src/environments.h.in environments.h @ONLY)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Test REQUIRED)

find_package(DtkWidget REQUIRED dtkwidget)
find_package(DtkGui REQUIRED dtkgui)
find_package(DtkCore REQUIRED dtkcore)

include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})

pkg_check_modules(GTEST REQUIRED
    gtest
    gmock
    )

set (EXE_NAME deepin-calculator)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::Xml
    Qt5::Svg
    Qt5::Test

    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
)

#CMAKE_CURRENT_LIST_DIR缓存文件的目录，头文件找不到问题解决方案1,文件中未使用该头文件变量　2,CMAKE_CURRENT_LIST_DIR改为CMAKE_SOURCE_DIR
SET(HEAD ${CMAKE_CURRENT_LIST_DIR}/../src
         ${CMAKE_CURRENT_LIST_DIR}/../src/bak
         ${CMAKE_CURRENT_LIST_DIR}/../src/control
         ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/core
         ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/math
         ${CMAKE_CURRENT_LIST_DIR}/../src/views
         ${CMAKE_CURRENT_LIST_DIR}/../src/widgets
         )

file(GLOB_RECURSE CAL_H ${CMAKE_CURRENT_LIST_DIR}/../src/*.h)
file(GLOB_RECURSE CAL_C ${CMAKE_CURRENT_LIST_DIR}/../src/*.c)
file(GLOB_RECURSE CAL_CPP ${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp)
file(GLOB_RECURSE CAL_3rdH ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/*.h)
file(GLOB_RECURSE CAL_3rdC ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/*.c)
file(GLOB_RECURSE CAL_3rdCPP ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/*.cpp)
list(REMOVE_ITEM CAL_CPP "${CMAKE_CURRENT_LIST_DIR}/../src/main.cpp")

file(GLOB_RECURSE CAL_TEST ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE CAL_TESTH ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

#find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set (DC_QRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-calculator.qrc
)

set(PROJECT_NAME_TEST
    ${PROJECT_NAME}-test)

# 生成测试可执行程序
add_executable(${PROJECT_NAME_TEST} ${CAL_H} ${CAL_C} ${CAL_CPP} ${CAL_3rdH} ${CAL_3rdC} ${CAL_3rdCPP} ${CAL_TEST} ${CAL_TESTH})

# 生成测试可执行程序的依赖库
target_link_libraries(${PROJECT_NAME_TEST} ${GTEST_LIBRARYS} ${GTEST_MAIN_LIBRARYS} gmock gmock_main gtest gtest_main pthread)

target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${Qt5Widgets_LIBRARIES}
                                              ${Qt5DBus_LIBRARIES}
                                              ${Qt5TestLib_LIBRARIES}
                                              ${PROJECT_BINARY_DIR}
                                              ${DtkWidget_INCLUDE_DIRS}
                                              ${DtkCore_LIBRARIES}
                                              ${DtkGUI_INCLUDE_DIRS}
                                              ${DFrameworkDBus_INCLUDE_DIRS}
                                              ${GTEST_INCLUDE_DIRS}
                                              ${HEAD})
target_link_libraries (${PROJECT_NAME_TEST} ${LINK_LIBS})

#set(CMAKE_INSTALL_PREFIX /usr)
if(DEFINED ENV{PREFIX})
    set(CMAKE_INSTALL_PREFIX $ENV{PREFIX})
else()
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

install(TARGETS deepin-calculator DESTINATION bin)
file(GLOB QM_FILES "${CMAKE_CURRENT_LIST_DIR}/../translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/deepin-calculator/translations)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/../deepin-calculator.desktop
        DESTINATION share/applications/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/../src/assets/images/deepin-calculator.svg
            DESTINATION share/icons/hicolor/scalable/apps/)


#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
#    COMMAND mkdir -p tests/coverageResult
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET test
    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "

    #1.清理原先的gcov信息
#    COMMAND lcov --directory ./tests/CMakeFiles/${PROJECT_NAME_TEST}.dir --zerocounters
#    COMMAND lcov --directory ./tests/CMakeFiles/${PROJECT_NAME}.dir --zerocounters
    COMMAND ${CMAKE_BINARY_DIR}/tests/${PROJECT_NAME_TEST}


    #2.收集gcov信息到.info文件中
#    COMMAND lcov --directory . --capture --output-file ./tests/coverageResult/${PROJECT_NAME}_Coverage.info

    #3.过滤一些我们不感兴趣的文件的覆盖率信息
#    COMMAND echo " =================== do filter begin ==================== "
#    COMMAND lcov --remove ./tests/coverageResult/${PROJECT_NAME}_Coverage.info
#    '*/${PROJECT_NAME_TEST}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/tests/*' '*/googletest/*' '*/usr/include/*' '*/src/bak/*' '*/src/core/*' '*/src/math/*'
#    -o ./tests/coverageResult/${PROJECT_NAME}_Coverage.info
#    COMMAND echo " =================== do filter end ==================== "

    #3.将.info信息生成报告到reprot文件夹中
#    COMMAND genhtml -o ./tests/coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info

    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

#    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "

#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

# 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
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

