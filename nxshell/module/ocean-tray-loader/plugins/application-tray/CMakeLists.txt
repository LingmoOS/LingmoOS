# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(PLUGIN_NAME "application-tray")

project(${PLUGIN_NAME})

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED Core Gui Widgets DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED Core Gui Widget)

pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11 xcb xcb-image xcb-composite xcb-xfixes xcb-util xcb-shape xtst xcb-xtest xcb-res xcb-ewmh)

file (GLOB TRAY_SOURCES *.cpp *.h api/types/*.cpp)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.lingmo.ocean.TrayManager1.xml
    PROPERTIES  INCLUDE api/types/traylist.h
                CLASSNAME TrayManager
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.kde.StatusNotifierItem.xml
    PROPERTIES  INCLUDE api/types/dbusimagelist.h
                INCLUDE api/types/dbustooltip.h
                CLASSNAME StatusNotifierItem
)

qt_add_dbus_interfaces(
    DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.lingmo.ocean.TrayManager1.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.kde.StatusNotifierItem.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.kde.StatusNotifierWatcher.xml
)

add_library(${PLUGIN_NAME} SHARED
    ${DBUS_INTERFACES}
    ${TRAY_SOURCES}
    application-tray.qrc
)

target_compile_definitions(${PLUGIN_NAME} PRIVATE "QT_PLUGIN")

target_link_libraries(${PLUGIN_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Dtk${DTK_VERSION_MAJOR}::Core
        Dtk${DTK_VERSION_MAJOR}::Gui
        Dtk${DTK_VERSION_MAJOR}::Widget
        PkgConfig::X11
        dbusmenuqt
        dockpluginmanager-interface
)

target_include_directories(${PLUGIN_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/interfaces/"
    "${CMAKE_SOURCE_DIR}/src/tray-wayland-integration/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../libdbusmenuqt/"
)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/ocean-dock)
# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins)