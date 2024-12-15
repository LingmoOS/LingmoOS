# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-fPIC)

set(libdbusmenu_SRCS
    dbusmenuimporter.cpp
    dbusmenushortcut_p.cpp
    dbusmenutypes_p.cpp
    utils.cpp
    dbusmenuimporter.h
    dbusmenushortcut_p.h
    dbusmenutypes_p.h
)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED Core Gui Widgets DBus)
set_source_files_properties(com.canonical.dbusmenu.xml PROPERTIES
   NO_NAMESPACE true
   INCLUDE "dbusmenutypes_p.h"
   CLASSNAME DBusMenuInterface
)

qt_add_dbus_interface(libdbusmenu_SRCS com.canonical.dbusmenu.xml dbusmenu_interface)

add_library(dbusmenuqt STATIC ${libdbusmenu_SRCS})

target_link_libraries(dbusmenuqt
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_subdirectory(test)
