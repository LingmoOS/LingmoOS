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

if (NOT DEFINED VERSION)
    set(VERSION 1.2.2)
endif ()

#common resource names
set(APP_RES_DIR "assets")
set(APP_BIN_NAME "deepin-voice-note")
set(APP_TRANS_SCRIPT "${APP_RES_DIR}/translate_generation.sh")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-voice-note.desktop")
set(APP_QRC "${APP_RES_DIR}/images.qrc")
set(APP_CONFIG "${APP_RES_DIR}/deepin-voice-note.conf")
set(APP_LOGO "${APP_RES_DIR}/icons/deepin/builtin/deepin-voice-note.svg")

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
set(CMAKE_CXX_FLAGS "-Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")
set(CMAKE_CXX_FLAGS "-g")

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-need")
endif ()

if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()

#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  add_definitions(-DWEB_PATH="${CMAKE_SOURCE_DIR}/assets/web")
  # Enable Qt builtin debug mode
  # add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O3 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -Wl,-O1 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -Wl,-O1 -Wl,--gc-sections")
  add_definitions(-DWEB_PATH="${CMAKE_INSTALL_PREFIX}/share/deepin-voice-note/web")
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

#安全加固参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")

#Generate the qm file
#bash translate.sh
#       transdir
#       needupdatets  <Only -DUPDATE_TS when need update ts file>
#       sourcedir
#       <projectname>.ts

# 判断系统环境
#file(STRINGS "/etc/os-version" MAJOR_STR REGEX "MajorVersion")
execute_process(COMMAND cat /etc/os-version OUTPUT_VARIABLE OS_INFO_STR)
string(REGEX MATCHALL "MajorVersion=[0-9]+" MAJOR_STR "${OS_INFO_STR}")
string(REGEX MATCH "[0-9]+" MAJOR_VERSION "${MAJOR_STR}")
if (MAJOR_VERSION MATCHES "23")
    message("--------------------- OS_BUILD_V23 on")
    add_definitions(-DOS_BUILD_V23)
endif()

execute_process(COMMAND bash
                    ${APP_TRANS_SCRIPT}
                    translations
                    ${APP_TS_UPDATE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${APP_BIN_NAME}
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

configure_file(src/environments.h.in environments.h @ONLY)

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

file(GLOB_RECURSE VNOTE_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus Sql Multimedia WebEngineWidgets WebChannel REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(DFrameworkdbus REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(LIBVLC REQUIRED libvlc)
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${LIBVLC_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Svg_INCLUDE_DIRS})
include_directories(${Qt5Xml_INCLUDE_DIRS})
pkg_search_module(libdmr REQUIRED libdmr)

# Tell CMake to create the executable
add_executable(${PROJECT_NAME} ${VNOTE_SRC} ${APP_QRC})

target_include_directories(${APP_BIN_NAME}
    PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR} ${libdmr_INCLUDE_DIRS})

target_link_libraries(${APP_BIN_NAME}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${LIBVLC_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Sql
    Qt5::Multimedia
    Qt5::WebChannel
    Qt5::WebEngineWidgets
    ${Qt5Svg_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${libdmr_LIBRARIES}
)

set(CMAKE_INSTALL_PREFIX /usr)

# Install files
install(TARGETS deepin-voice-note DESTINATION bin)

file(GLOB APP_QM_FILES "translations/*.qm")
install(FILES ${APP_QM_FILES} DESTINATION share/deepin-voice-note/translations)
install(FILES ${APP_DESKTOP} DESTINATION share/applications)
install(FILES ${APP_CONFIG} DESTINATION share/deepin-voice-note)
install(FILES ${APP_LOGO} DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/)
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/sw/deepin-voice-note     DESTINATION /usr/share/deepin-manual/manual-assets/application/)
else()
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/deepin-voice-note     DESTINATION /usr/share/deepin-manual/manual-assets/application/)
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/web    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-voice-note)
#if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
if (CMAKE_BUILD_TYPE MATCHES Debug)
#add_subdirectory(tests)
endif()
#endif()
