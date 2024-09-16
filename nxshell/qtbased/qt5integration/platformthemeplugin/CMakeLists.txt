# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus Widgets)
set(QT_LIBS Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Widgets)
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5X11Extras REQUIRED)
    list(APPEND QT_LIBS Qt5::X11Extras)
    if(QT_VERSION_MINOR GREATER_EQUAL 8) # Qt5.8
        find_package(Qt5ThemeSupport REQUIRED)
        list(APPEND QT_LIBS Qt5::ThemeSupportPrivate)
    else()
        list(APPEND QT_LIBS Qt5::PlatformSupportPrivate) # TODO to be verified
    endif()
    set(3RD_PARTY_SOURCES
        ${CMAKE_SOURCE_DIR}/3rdparty/qdbustrayicon.cpp
        ${CMAKE_SOURCE_DIR}/3rdparty/qstatusnotifieritemadaptor.cpp
        ${CMAKE_SOURCE_DIR}/3rdparty/qdbusmenuconnection.cpp
    )
    set(3RD_PARTY_HEADERS
        ${CMAKE_SOURCE_DIR}/3rdparty/qdbustrayicon_p.h
        ${CMAKE_SOURCE_DIR}/3rdparty/qstatusnotifieritemadaptor_p.h
        ${CMAKE_SOURCE_DIR}/3rdparty/qdbusmenuconnection_p.h
    )
else()
    list(APPEND QT_LIBS Qt6::GuiPrivate)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(Deps REQUIRED IMPORTED_TARGET mtdev x11)
if(ENABLE_QT_XDG_ICON_LOADER)
    set(DEFS -DXDG_ICON_VERSION_MAR=${XDG_ICON_VERSION_MAJOR})
    list(APPEND QT_LIBS ${QT_XDG_ICON_DEPS})
endif()
if(QT_VERSION_MAJOR EQUAL 5)
    qt5_add_dbus_interface(DBUS_INTERFACES ${CMAKE_CURRENT_LIST_DIR}/xmls/com.deepin.filemanager.filedialog.xml filedialog_interface)
    qt5_add_dbus_interface(DBUS_INTERFACES ${CMAKE_CURRENT_LIST_DIR}/xmls/com.deepin.filemanager.filedialogmanager.xml filedialogmanager_interface)
else()
    qt6_add_dbus_interface(DBUS_INTERFACES ${CMAKE_CURRENT_LIST_DIR}/xmls/com.deepin.filemanager.filedialog.xml filedialog_interface)
    qt6_add_dbus_interface(DBUS_INTERFACES ${CMAKE_CURRENT_LIST_DIR}/xmls/com.deepin.filemanager.filedialogmanager.xml filedialogmanager_interface)
endif()
dtk_add_plugin(
    NAME
        qdeepin
    OUTPUT_DIR
        ${PLUGIN_OUTPUT_BASE_DIR}/platformthemes
    INSTALL_DIR
        ${PLUGIN_INSTALL_BASE_DIR}/platformthemes
    SOURCES
        dthemesettings.cpp
        qdeepinfiledialoghelper.cpp
        qdeepintheme.cpp
        ${3RD_PARTY_SOURCES}
        main.cpp
        ${DBUS_INTERFACES}
    HEADERS
        dthemesettings.h
        qdeepinfiledialoghelper.h
        qdeepintheme.h
        ${3RD_PARTY_HEADERS}
    RESOURCES
        icons/deepin-theme-plugin-icons.qrc
        deepin-theme-plugin.qrc
    DEPENDENCIES
        Dtk${VERSION_SUFFIX}::Gui
        ${QT_LIBS}
        PkgConfig::Deps
    DEFINITIONS
        ${DEFS}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}
)
