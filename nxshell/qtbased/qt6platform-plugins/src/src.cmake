# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

list(APPEND GLOBAL_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/dbackingstoreproxy.h
    ${CMAKE_CURRENT_LIST_DIR}/dnativesettings.h
    ${CMAKE_CURRENT_LIST_DIR}/dopenglpaintdevice.h
    ${CMAKE_CURRENT_LIST_DIR}/dxcbxsettings.h
    ${CMAKE_CURRENT_LIST_DIR}/global.h
    ${CMAKE_CURRENT_LIST_DIR}/vtablehook.h
    ${CMAKE_CURRENT_LIST_DIR}/dplatformsettings.h
    ${CMAKE_CURRENT_LIST_DIR}/ddesktopinputselectioncontrol.h
    ${CMAKE_CURRENT_LIST_DIR}/dinputselectionhandle.h
    ${CMAKE_CURRENT_LIST_DIR}/dapplicationeventmonitor.h
    ${CMAKE_CURRENT_LIST_DIR}/dselectedtexttooltip.h
)
list(APPEND GLOBAL_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/dbackingstoreproxy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dnativesettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dopenglpaintdevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dxcbxsettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/global.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vtablehook.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dplatformsettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddesktopinputselectioncontrol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dinputselectionhandle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dapplicationeventmonitor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dselectedtexttooltip.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
