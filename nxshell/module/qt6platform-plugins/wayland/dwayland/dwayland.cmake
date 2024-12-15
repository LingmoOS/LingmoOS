# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB WAYLAND_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/*.h
  ${CMAKE_SOURCE_DIR}/src/global.h
  ${CMAKE_SOURCE_DIR}/src/vtablehook.h
  ${CMAKE_SOURCE_DIR}/src/dxcbxsettings.h
  ${CMAKE_SOURCE_DIR}/src/dplatformsettings.h
  ${CMAKE_SOURCE_DIR}/src/dnativesettings.h
)
file(GLOB WAYLAND_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
  ${CMAKE_SOURCE_DIR}/src/global.cpp
  ${CMAKE_SOURCE_DIR}/src/vtablehook.cpp
  ${CMAKE_SOURCE_DIR}/src/dxcbxsettings.cpp
  ${CMAKE_SOURCE_DIR}/src/dplatformsettings.cpp
  ${CMAKE_SOURCE_DIR}/src/dnativesettings.cpp
)

set(wayland_SRC
  ${WAYLAND_HEADER}
  ${WAYLAND_SOURCE}
)
