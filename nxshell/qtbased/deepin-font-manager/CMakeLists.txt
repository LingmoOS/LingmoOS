#/*****************************************************************************
#* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#*
#* Author:     Xiao Zhiguo <xiaozhiguo@uniontech.com>
#* Date:       2020-09-09
#*
#* Maintainer: Xiao Zhiguo <xiaozhiguo@uniontech.com>
#*
#* This program is free software: you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
#* the Free Software Foundation, either version 3 of the License, or
#* any later version.
#*
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*
#*****************************************************************************/

CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

PROJECT(font-manager)

IF (NOT DEFINED VERSION)
    SET(VERSION 1.0.0)
ENDIF()

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)

FIND_PACKAGE(Qt5 COMPONENTS Core Gui Svg Sql Xml DBus Network REQUIRED)
FIND_PACKAGE(DtkWidget REQUIRED)
FIND_PACKAGE(DtkGui REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)

#社区版上 MOC 时，会缺少 include 路径 /usr/include，
#暂未找到原因，先手动添加 /usr/include 规避
INCLUDE_DIRECTORIES(/usr/include/)

PKG_SEARCH_MODULE(FontConfig REQUIRED fontconfig IMPORTED_TARGET)
#PKG_SEARCH_MODULE(QGsettings REQUIRED gsettings-qt IMPORTED_TARGET)

#CMAKE 3.14版本才有的模块
#INCLUDE(FindFontconfig)
include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ")

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

IF (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
ENDIF()
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
    # Enable Qt builtin debug mode
    ADD_DEFINITIONS("-DQT_MESSAGELOGCONTEXT")
    ADD_SUBDIRECTORY(tests)
ENDIF()
ADD_SUBDIRECTORY(deepin-font-manager)
ADD_SUBDIRECTORY(deepin-font-preview-plugin)

#安装帮助手册需要文件
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deepin-font-manager-assets/deepin-font-manager
   DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application/)

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()


