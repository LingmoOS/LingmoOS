# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS DBus)
find_package(OCEANApplicationManager REQUIRED)
find_package(yaml-cpp REQUIRED)

set_source_files_properties(
    ${OCEAN_APPLICATION_MANAGER_DBUS_API_DIR}/org.desktopspec.ApplicationManager1.Application.xml
    PROPERTIES  INCLUDE api/types/am.h
                CLASSNAME Application
)

set_source_files_properties(
    ${OCEAN_APPLICATION_MANAGER_DBUS_API_DIR}/org.desktopspec.ApplicationManager1.xml
    PROPERTIES  INCLUDE api/types/am.h
                CLASSNAME ApplicationManager
)

set_source_files_properties(
    ${OCEAN_APPLICATION_MANAGER_DBUS_API_DIR}/org.desktopspec.ObjectManager1.xml
    PROPERTIES  INCLUDE api/types/am.h
                CLASSNAME ObjectManager
)

qt_add_dbus_interfaces(
    DBUS_INTERFACES
    ${OCEAN_APPLICATION_MANAGER_DBUS_API_DIR}/org.desktopspec.ApplicationManager1.Application.xml
    ${OCEAN_APPLICATION_MANAGER_DBUS_API_DIR}/org.desktopspec.ApplicationManager1.xml
    ${OCEAN_APPLICATION_MANAGER_DBUS_API_DIR}/org.desktopspec.ObjectManager1.xml
)


add_library(ocean-apps SHARED ${DBUS_INTERFACES}
    amappitem.cpp
    amappitem.h
    amappitemmodel.cpp
    amappitemmodel.h
    appgroup.cpp
    appgroup.h
    appgroupmanager.cpp
    appgroupmanager.h
    appitem.cpp
    appitem.h
    appitemmodel.cpp
    appitemmodel.h
    appsapplet.cpp
    appsapplet.h
    appsdockedhelper.cpp
    appsdockedhelper.h
    appslaunchtimes.cpp
    appslaunchtimes.h
    categoryutils.cpp
    categoryutils.h
    itemspage.cpp
    itemspage.h
)

target_link_libraries(ocean-apps PRIVATE
    ocean-shell-frame
    Qt${QT_VERSION_MAJOR}::Concurrent
    yaml-cpp
)

ds_install_package(PACKAGE org.lingmo.ds.ocean-apps TARGET ocean-apps)
dtk_add_config_meta_files(APPID org.lingmo.ocean.shell FILES configs/org.lingmo.ds.ocean-apps.json)
