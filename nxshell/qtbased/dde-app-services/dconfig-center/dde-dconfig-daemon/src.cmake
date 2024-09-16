# SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-only

find_package(Qt5DBus REQUIRED)

if(EnableDtk5)
qt5_add_dbus_adaptor(DCONFIG_DBUS_XML ../dde-dconfig-daemon/services/org.desktopspec.ConfigManager.xml
    dconfigserver.h DSGConfigServer
    configmanager_adaptor DSGConfigAdaptor)

qt5_add_dbus_adaptor(DCONFIG_DBUS_XML ../dde-dconfig-daemon/services/org.desktopspec.ConfigManager.Manager.xml
    dconfigconn.h DSGConfigConn
    manager_adaptor DSGConfigManagerAdaptor)
endif()

if(EnableDtk6)
qt_add_dbus_adaptor(DCONFIG_DBUS_XML ../dde-dconfig-daemon/services/org.desktopspec.ConfigManager.xml
    dconfigserver.h DSGConfigServer
    configmanager_adaptor DSGConfigAdaptor)

qt_add_dbus_adaptor(DCONFIG_DBUS_XML ../dde-dconfig-daemon/services/org.desktopspec.ConfigManager.Manager.xml
    dconfigconn.h DSGConfigConn
    manager_adaptor DSGConfigManagerAdaptor)
endif()

include_directories(../common)

set(HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/dconfig_global.h
    ${CMAKE_CURRENT_LIST_DIR}/dconfigserver.h
    ${CMAKE_CURRENT_LIST_DIR}/dconfigresource.h
    ${CMAKE_CURRENT_LIST_DIR}/dconfigconn.h
    ${CMAKE_CURRENT_LIST_DIR}/dconfigrefmanager.h
)
set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/dconfigserver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dconfigresource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dconfigconn.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dconfigrefmanager.cpp
)
