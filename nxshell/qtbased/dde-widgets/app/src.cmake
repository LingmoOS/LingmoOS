# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

include_directories(../interface)

qt5_add_dbus_adaptor(DBUS_XML ${CMAKE_CURRENT_LIST_DIR}/services/org.deepin.dde.Widgets1.xml
    widgetsserver.h WidgetsServer
    dbusserver_adaptor DBusServer)

set(HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/mainview.h
    ${CMAKE_CURRENT_LIST_DIR}/displaymodepanel.h
    ${CMAKE_CURRENT_LIST_DIR}/global.h
    ${CMAKE_CURRENT_LIST_DIR}/pluginspec.h
    ${CMAKE_CURRENT_LIST_DIR}/widgethandler.h
    ${CMAKE_CURRENT_LIST_DIR}/widgetmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/editmodepanel.h
    ${CMAKE_CURRENT_LIST_DIR}/widgetstore.h
    ${CMAKE_CURRENT_LIST_DIR}/widgetsserver.h
    ${CMAKE_CURRENT_LIST_DIR}/instanceproxy.h
    ${CMAKE_CURRENT_LIST_DIR}/instancemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/instancepanel.h
    ${CMAKE_CURRENT_LIST_DIR}/aboutdialog.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/utils.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/geometryhandler.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/animationviewcontainer.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/appearancehandler.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/button.h
    ${CMAKE_CURRENT_LIST_DIR}/accessible/accessible.h
)
set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/mainview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/displaymodepanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pluginspec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/widgethandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/widgetmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/editmodepanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/widgetstore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/widgetsserver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instanceproxy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instancemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instancepanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/aboutdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/geometryhandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/animationviewcontainer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/appearancehandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/button.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/utils)
