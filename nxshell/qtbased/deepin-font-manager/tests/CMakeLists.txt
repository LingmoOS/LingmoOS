# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

ADD_COMPILE_OPTIONS(-fno-access-control)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage -fno-inline")

#使用cmake -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" ..
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


FIND_PACKAGE(Qt5Test REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

SET(PROJECT_NAME_TEST ${PROJECT_NAME}-test)
#ADD_EXECUTABLE(${PROJECT_NAME_TEST} "")

#libdeepin-font-manager

SET(QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager-assets/deepin-font-manager.qrc)
SET(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libdeepin-font-manager)


#include_directories(third-party/googletest)                           #一下三行添加包含以便测试用例代码的编写
#include_directories(third-party/googletest/googlemock/include)
#include_directories(third-party/googletest/googletest/include)
#add_subdirectory(third-party/googletest)

FILE (GLOB allSource
     ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/views/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/interfaces/*.cpp
    )

LIST(REMOVE_ITEM allSource "${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/main.cpp")

FILE (GLOB allHeaders
     ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/views/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/interfaces/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    )

SET(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libdeepin-font-manager)

FILE(GLOB LIB_SRC_FILES ${LIB_DIR}/*.cpp ${LIB_DIR}/*.h)

#FILE(GLOB_RECURSE FONT_SRC_TEST1 ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

FILE(GLOB_RECURSE FONT_SRC_TEST_INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/deepin-font-manager-test/interfaces/*.cpp)
FILE(GLOB_RECURSE FONT_SRC_TEST_VIEWS ${CMAKE_CURRENT_LIST_DIR}/src/deepin-font-manager-test/views/*.cpp)
FILE(GLOB_RECURSE FONT_SRC_TEST_A ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)


ADD_EXECUTABLE(${PROJECT_NAME_TEST} ${FONT_SRC_TEST_INTERFACE} ${FONT_SRC_TEST_VIEWS} ${FONT_SRC_TEST_A} ${allHeaders} ${allSource} ${LIB_SRC_FILES} ${QRC_FILES})

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME_TEST} PUBLIC ${LIB_DIR})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/views)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager/interfaces)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 生成测试可执行程序的依赖库
TARGET_LINK_LIBRARIES(${PROJECT_NAME_TEST} gtest gtest_main -pthread)


TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME_TEST} PUBLIC ${DtkWidget_INCLUDE_DIRS}
                           ${DtkCore_INCLUDE_DIRS}
                            ${DtkGui_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME_TEST} Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Sql Qt5::Xml Qt5::Svg Qt5::DBus Qt5::Network
            Qt5::Test ${DtkCore_LIBRARIES} ${DtkWidget_LIBRARIES} ${FREETYPE_LIBRARIES}
            ${FREETYPE_LIBRARIES} PkgConfig::FontConfig  ${GTEST_LIBRARYS} ${GTEST_MAIN_LIBRARYS})

ADD_CUSTOM_TARGET( test
#    #执行mkdir -p 创建 coverageResult
#    COMMAND mkdir -p test/coverageResult
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(TARGET test
        COMMAND echo " =================== TEST BEGIN ===================="
        COMMAND ${CMAKE_BINARY_DIR}/tests/${PROJECT_NAME_TEST}
        COMMAND echo " =================== TEST END ===================="
)

##'make test'命令依赖与我们的测试程序
ADD_DEPENDENCIES(test ${PROJECT_NAME_TEST})

