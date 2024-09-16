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
    set(VERSION 5.5.10.1)
endif ()


#common resource names
set(APP_RES_DIR "assets")
set(APP_BIN_NAME "deepin-devicemanager")
set(APP_TRANS_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/translate_generation.sh")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-devicemanager.desktop")
set(APP_QRC "${APP_RES_DIR}/appimage.qrc")
set(APP_ICON_FILE "${APP_RES_DIR}/icons/deepin/builtin/deepin-devicemanager.svg")
set(APP_AUTHENTICATION "${APP_RES_DIR}/com.deepin.deepin-devicemanager.policy")
set(APP_SERVICE "${APP_RES_DIR}/dbus/com.deepin.Devicemanager.service")
project(${APP_BIN_NAME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed -fPIE -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

option (PERF_ON "Use provided math implementation" ON)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

# 判断系统环境
execute_process(COMMAND cat /etc/os-version OUTPUT_VARIABLE OS_INFO_STR)
string(REGEX MATCHALL "MajorVersion=[0-9]+" MAJOR_STR "${OS_INFO_STR}")
string(REGEX MATCH "[0-9]+" MAJOR_VERSION "${MAJOR_STR}")
if (MAJOR_VERSION MATCHES "23")
    message("--------------------- OS_BUILD_V23 on")
    add_definitions(-DOS_BUILD_V23)
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

#Generate the qm file
execute_process(COMMAND bash ${APP_TRANS_SCRIPT} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
configure_file(src/environments.h.in environments.h @ONLY)
configure_file(src/config.h.in config.h @ONLY)

# include dir
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
SUBDIRLIST(dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(dir ${dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endforeach()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/docx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QtXlsxWriter/include/QtXlsx)

#add cups and zmq
include_directories("/usr/include/cups/")
link_libraries("cups")

include_directories(${CMAKE_CURRENT_LIST_DIR}/src/DDLog/)
file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp ${CMAKE_CURRENT_LIST_DIR}/3rdparty/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h ${CMAKE_CURRENT_LIST_DIR}/3rdparty/*.h)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(PolkitQt5-1 REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Tell CMake to create the executable
add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_H} ${APP_QRC})

target_include_directories(${APP_BIN_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})
target_link_libraries(${APP_BIN_NAME}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Xml
    Qt5::Network
    PolkitQt5-1::Agent
)

# Install files
install(TARGETS deepin-devicemanager DESTINATION ${CMAKE_INSTALL_BINDIR})
file(GLOB APP_QM_FILES "translations/*.qm")
install(FILES ${APP_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-devicemanager/translations)
install(FILES ${APP_DESKTOP} DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${APP_ICON_FILE} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(DIRECTORY ${APP_RES_DIR}/deepin-devicemanager DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application/)
install(FILES ${APP_AUTHENTICATION} DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions)
install(FILES ${APP_SERVICE} DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
install(FILES ${CMAKE_SOURCE_DIR}/deepin-devicemanager/assets/deepin-devicemanager.json DESTINATION /usr/share/deepin-log-viewer/deepin-log.conf.d/)
#hw机型增加DConfig配置
set(APPID org.deepin.devicemanager)
set(configFile ${PROJECT_SOURCE_DIR}/assets/org.deepin.devicemanager.json)
if (DEFINED DSG_DATA_DIR)
    message("-- DConfig is supported by DTK")
    dconfig_meta_files(APPID ${APPID} FILES ${configFile})
endif()

# Test--------deepin-devicemanager
if (CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
   add_subdirectory(./tests)
endif()

