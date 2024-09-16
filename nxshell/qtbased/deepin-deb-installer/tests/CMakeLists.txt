# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)
if (NOT DEFINED VERSION)
    message(WARNING "Not defined version ,about dialog version set 5.3.9")
    set(VERSION 5.3.9)
endif ()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
#common resource names
set(APP_RES_DIR "${CMAKE_SOURCE_DIR}/assets")
set(APP_BIN_NAME_TEST "deepin-deb-installer-test")
set(APP_TRANS_SCRIPT "${APP_RES_DIR}/translate_generation.sh")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-deb-installer.desktop")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")

project(${APP_BIN_NAME_TEST})


ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

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

#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")

else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage")

#Generate the qm file
execute_process(COMMAND bash ${APP_TRANS_SCRIPT}
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

configure_file(${APP_RES_DIR}/environments.h.in environments.h @ONLY)

add_definitions(-DUSE_POLKIT -DENABLE_INACTIVE_DISPLAY)
# Find the library

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(PolkitQt5-1 REQUIRED)


set(LINK_LIBS
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    QApt
    Qt5::Concurrent
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)

macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/deb-installer)

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/../src/deb-installer/)

#Include all app own subdirectorys
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}${subdir})
endforeach()

file(GLOB_RECURSE APP_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/singleInstallerApplication.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/manager/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/model/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/utils/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/process/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/view/pages/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/view/widgets/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/uab/*.cpp
    )
FILE(GLOB allTestSource
  src/*.cpp
  src/manager/*.cpp
  src/model/*.cpp
  src/utils/*.cpp
  src/view/pages/*.cpp
  src/view/widgets/*.cpp
  src/uab/*.cpp
)
include_directories(src)
include_directories( ${CMAKE_CURRENT_LIST_DIR}/../src/deb-installer/)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Tell CMake to create the executable
add_executable(${APP_BIN_NAME_TEST} ${APP_SRCS} ${APP_QRC} ${TEST_ACCESSIBLE_HEADERS_FILES}  ${allTestSource} )

target_include_directories(${APP_BIN_NAME_TEST}
    PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})


target_link_libraries(${APP_BIN_NAME_TEST}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    QApt
    PolkitQt5-1::Agent
    Qt5::Test
    Qt5::Concurrent
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

#add_custom_target(test
#    COMMAND echo " =================== TEST BEGIN ==================== "
#    COMMAND ${CMAKE_BINARY_DIR}/tests/${APP_BIN_NAME_TEST}
#    COMMAND echo " =================== TEST END ==================== "
#)
