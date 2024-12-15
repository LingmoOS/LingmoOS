# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(DBUS_SOURCE)

set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/../../xml/org.desktopspec.ObjectManager1.xml
    PROPERTIES
    INCLUDE
       ${CMAKE_CURRENT_LIST_DIR}/types/objectmanager_type.h
    CLASSNAME
        ApplicationObjectManager1
)

qt_add_dbus_interface(
    DBUS_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/../../xml/org.desktopspec.ObjectManager1.xml
    objectmanager_interface
)


dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Audio1.xml
    org_lingmo_ocean_audio1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Audio1.Sink.xml
    org_lingmo_ocean_audio1_sink
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Accounts1.xml
    org_lingmo_ocean_accounts1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Accounts1.User.xml
    org_lingmo_ocean_accounts1_user
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/com.lingmo.chromeExtension.TabsLimit.xml
    com_lingmo_chromeextension_tabslimit
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Display1.xml
    org_lingmo_ocean_display1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Display1.Monitor.xml
    org_lingmo_ocean_display1_monitor
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Appearance1.xml
    org_lingmo_ocean_appearance1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Bluetooth1.xml
    org_lingmo_ocean_bluetooth1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.daemon.Dock1.xml
    org_lingmo_ocean_daemon_dock1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.Gesture1.xml
    org_lingmo_ocean_gesture1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.ImageBlur1.xml
    org_lingmo_ocean_imageblur1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.InputDevice1.Keyboard.xml
    org_lingmo_ocean_inputdevice1_keyboard
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.SessionManager1.xml
    org_lingmo_ocean_sessionmanager1
)

dtk_add_dbus_interface(
    DBUS_SOURCE
    ../../xml/org.lingmo.ocean.SoundEffect1.xml
    org_lingmo_ocean_soundeffect1
)

file(GLOB_RECURSE TYPES_SRCS
    "types/*.h"
    "types/*.cpp"
)

set(SESSION_UI_DBUS_INTERFACE_SRCS
    ${DBUS_SOURCE}
    ${TYPES_SRCS}
)

add_library(session-ui-dbus-shared OBJECT
    ${SESSION_UI_DBUS_INTERFACE_SRCS}
)

target_include_directories(session-ui-dbus-shared
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(session-ui-dbus-shared
PRIVATE
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Core
)
