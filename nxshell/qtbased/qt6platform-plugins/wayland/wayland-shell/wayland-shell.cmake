# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

find_package(ECM REQUIRED 1.0.0)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

find_package(DWayland)
if(DWayland_FOUND)
    add_definitions(-DD_DEEPIN_IS_DWAYLAND)
    set(COMMON_LIBS Deepin::WaylandClient)
    message("Using the DWayland module")
else()
    find_package(KF5Wayland REQUIRED)
    set(COMMON_LIBS KF5::WaylandClient)
    message("Using the KF5Wayland module")
endif()
