# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

if (BUILD_WITH_X11)
pkg_check_modules(AppRunTimeXcb REQUIRED IMPORTED_TARGET xcb xcb-res xcb-ewmh xcb-icccm)

add_library(dock-appruntimeitem SHARED
    appruntimeitem.cpp
    appruntimeitem.h
    windowmanager.cpp
    windowmanager.h
    xcbgetinfo.h
    xcbgetinfo.cpp
    qmlappruntime.qrc
    ${CMAKE_SOURCE_DIR}/panels/dock/dockiteminfo.cpp
    ${CMAKE_SOURCE_DIR}/panels/dock/dockiteminfo.h
    ${CMAKE_SOURCE_DIR}/panels/dock/constants.h
    ${CMAKE_SOURCE_DIR}/panels/dock/taskmanager/x11utils.h
    ${CMAKE_SOURCE_DIR}/panels/dock/taskmanager/x11utils.cpp
    ${CMAKE_SOURCE_DIR}/panels/dock/taskmanager/abstractwindow.h
    ${CMAKE_SOURCE_DIR}/panels/dock/taskmanager/x11window.h
    ${CMAKE_SOURCE_DIR}/panels/dock/taskmanager/x11window.cpp
)

target_include_directories(dock-appruntimeitem PRIVATE
     ${CMAKE_SOURCE_DIR}/panels/dock
     ${CMAKE_SOURCE_DIR}/panels/dock/taskmanager
    "${CMAKE_CURRENT_BINARY_DIR}/../"
)
target_link_libraries(dock-appruntimeitem PRIVATE
    ocean-shell-frame
    PkgConfig::AppRunTimeXcb
)

ds_install_package(PACKAGE org.lingmo.ds.dock.appruntimeitem TARGET dock-appruntimeitem)
ds_handle_package_translation(PACKAGE org.lingmo.ds.dock.appruntimeitem)

install(FILES "package/icons/appruntime.svg" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ocean-dock/icons/oceanui-setting)
endif(BUILD_WITH_X11)
