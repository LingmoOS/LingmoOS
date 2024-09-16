# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#
# Author:     V4fr3e <V4fr3e@deepin.io>
#
# Maintainer: V4fr3e <liujinli@uniontech.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.7)
set(PROJECT_VERSION "5.9.0.53")

if (NOT DEFINED VERSION)
    set(VERSION 1.2.2)
endif ()

include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

ADD_COMPILE_OPTIONS(-fno-access-control)

#set different ISO types
#1:professional 2:community 3:server
#set(ISO_TYPE "professional")
set(ISO_TYPE "community")
#set(ISO_TYPE "server")

#common resource names
set(APP_RES_DIR "${CMAKE_SOURCE_DIR}/resources")
set(APP_BIN_NAME "dde-introduction")
set(APP_TRANS_SCRIPT "${CMAKE_SOURCE_DIR}/translate_generation.sh")
set(APP_DESKTOP "${CMAKE_SOURCE_DIR}/dde-introduction.desktop")
set(APP_QRC "${CMAKE_SOURCE_DIR}/dde-introduction.qrc")
set(APP_LOGO "${APP_RES_DIR}/dde-introduction.svg")

#Check if APP_TS_UPDATE var isn't set, don't need update ts file
if (NOT DEFINED APP_TS_UPDATE)
   set(APP_TS_UPDATE false)
endif()

#Only update ts file when APP_TS_UPDATE=true
if (${APP_TS_UPDATE} STREQUAL "true")
    set(APP_TS_UPDATE true)
endif()

project(${APP_BIN_NAME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "-Wl,--as-needed -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "loongarch64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DISLOONGARCH64")
endif ()

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

execute_process(COMMAND bash
                    ${APP_TRANS_SCRIPT}
                    translations
                    ${APP_TS_UPDATE}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${APP_BIN_NAME}
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

configure_file(${CMAKE_CURRENT_LIST_DIR}/src/environments.h.in ${CMAKE_CURRENT_LIST_DIR}/src/environments.h @ONLY)

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

#Include all app own subdirectorys
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()

file(GLOB_RECURSE CPP_SRC ${CMAKE_SOURCE_DIR}/src/*.cpp)


# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus Sql Multimedia REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DFrameworkdbus REQUIRED)

include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    pkg_check_modules(LIBDMR REQUIRED libdmr)
    include_directories(${LIBDMR_INCLUDE_DIRS})
else()
    ADD_DEFINITIONS("-D DISABLE_VIDEO")
    LIST(REMOVE_ITEM CPP_SRC ${CMAKE_CURRENT_LIST_DIR}/src/modules/videowidget.cpp)
endif()


# Tell CMake to create the executable
add_executable(${PROJECT_NAME} ${CPP_SRC} ${APP_QRC})

target_include_directories(${APP_BIN_NAME}
    PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})

target_link_libraries(${APP_BIN_NAME}
    ${DtkCore_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Multimedia
)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    target_link_libraries(${APP_BIN_NAME} ${LIBDMR_LIBRARIES})
endif()


# Install files
install(TARGETS dde-introduction DESTINATION ${CMAKE_INSTALL_BINDIR})

file(GLOB APP_QM_FILES "${CMAKE_SOURCE_DIR}/translations/*.qm")
file(GLOB_RECURSE ASS_SRC ${CMAKE_SOURCE_DIR}/resources/video/*.ass)

#Choose different videos for different ISO types
if(ISO_TYPE MATCHES "server")
    set(VIDEO_PATH ${CMAKE_SOURCE_DIR}/resources/video/server/demo.mp4)
elseif(ISO_TYPE MATCHES "community")
    set(VIDEO_PATH ${CMAKE_SOURCE_DIR}/resources/video/community/demo.mp4)
else()
    set(VIDEO_PATH ${CMAKE_SOURCE_DIR}/resources/video/professional/demo.mp4)
endif()

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()

file(GLOB_RECURSE DEMO_VIDEO_SRC ${VIDEO_PATH})

install(FILES ${APP_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-introduction/translations)
install(FILES ${APP_DESKTOP} DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${APP_LOGO} DESTINATION  ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/)
install(FILES ${ASS_SRC} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-introduction/)
install(FILES ${DEMO_VIDEO_SRC} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-introduction/)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    #add ut test
    add_subdirectory(tests)
endif()

