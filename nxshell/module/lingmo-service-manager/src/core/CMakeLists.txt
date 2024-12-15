# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set(LIB_NAME "core-service")

project(${LIB_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd) # sdbus

file(GLOB_RECURSE LIB_SRCS
  "*.h"
  "*.cpp"
)
add_library(${LIB_NAME} STATIC
  ${LIB_SRCS}
)
target_include_directories(${LIB_NAME} PUBLIC
  Qt5::Core
  Qt5::DBus
  PkgConfig::Systemd # sdbus
)

target_link_libraries(${LIB_NAME} PRIVATE
  Qt5::Core
  Qt5::DBus
  PkgConfig::Systemd # sdbus
)

