# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB XCB_HEADER ${CMAKE_CURRENT_LIST_DIR}/*.h)
file(GLOB XCB_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/3rdparty/*.c)
file(GLOB XCB_RCC_QRCS ${CMAKE_CURRENT_LIST_DIR}/cursors/cursor.qrc)

add_definitions(-DUSE_NEW_IMPLEMENTING)
set(USE_NEW_IMPLEMENTING TRUE)
if (USE_NEW_IMPLEMENTING)
    list(REMOVE_ITEM XCB_HEADER
        ${CMAKE_CURRENT_LIST_DIR}/dplatformbackingstore.h
        ${CMAKE_CURRENT_LIST_DIR}/dplatformwindowhook.h)
    list(REMOVE_ITEM XCB_SOURCE
        ${CMAKE_CURRENT_LIST_DIR}/dplatformbackingstore.cpp
        ${CMAKE_CURRENT_LIST_DIR}/dplatformwindowhook.cpp)
endif()

if(${QT_VERSION_MAJOR} STREQUAL "5")
    qt5_add_dbus_interface(DBUS_INTERFACE_XMLS ${CMAKE_SOURCE_DIR}/misc/com.deepin.im.xml im_interface)
    qt5_add_dbus_interface(DBUS_INTERFACE_XMLS ${CMAKE_SOURCE_DIR}/misc/org.freedesktop.DBus.xml dbus_interface)
else()
    qt6_add_dbus_interface(DBUS_INTERFACE_XMLS ${CMAKE_SOURCE_DIR}/misc/com.deepin.im.xml im_interface)
    qt6_add_dbus_interface(DBUS_INTERFACE_XMLS ${CMAKE_SOURCE_DIR}/misc/org.freedesktop.DBus.xml dbus_interface)
endif()

set(xcb_SRC ${XCB_HEADER} ${XCB_SOURCE} ${XCB_RCC_QRCS} ${DBUS_INTERFACE_XMLS})

include_directories(${CMAKE_CURRENT_LIST_DIR})
