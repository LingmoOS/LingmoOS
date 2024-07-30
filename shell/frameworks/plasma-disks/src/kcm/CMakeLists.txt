# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

file(GLOB QML_SRCS package/contents/*.qml)
add_custom_target(QmlFiles ALL echo SOURCES ${QML_SRCS})

set(kcm_SRCS
    module.cpp
    devicemodel.cpp devicemodel.h
    servicerunner.cpp servicerunner.h
)

# Something wonky is going on with the dbus macros. When the commands are set in the parent scope
# we'll not have access to them here as cmake claims
#   No rule to make target 'src/org.kde.smart.Device.xml'
# Instead generate the xmls (again) here...
qt_generate_dbus_interface(../device.h org.kde.kded.smart.Device.xml)
qt_add_dbus_interface(kcm_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded.smart.Device.xml org.kde.kded.smart.Device)

qt_add_dbus_interface(kcm_SRCS org.freedesktop.DBus.Properties.xml org.freedesktop.DBus.Properties)
set_source_files_properties(org.freedesktop.DBus.ObjectManager.xml
    PROPERTIES
        INCLUDE dbusobjectmanagerserver.h
)
qt_add_dbus_interface(kcm_SRCS org.freedesktop.DBus.ObjectManager.xml org.freedesktop.DBus.ObjectManager)

kcmutils_add_qml_kcm(kcm_disks SOURCES ${kcm_SRCS} INSTALL_NAMESPACE "plasma/kcms/kinfocenter")

target_link_libraries(kcm_disks PRIVATE
    statickdedsmart
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    Qt6::DBus
    KF6::Solid
)
