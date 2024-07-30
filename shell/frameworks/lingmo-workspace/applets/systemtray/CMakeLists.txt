add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.private.systemtray\")

lingmo_install_package(package org.kde.lingmo.private.systemtray)

include_directories(${lingmo-workspace_SOURCE_DIR}/statusnotifierwatcher)

set(systemtray_SRCS
    dbusserviceobserver.cpp
    plasmoidregistry.cpp
    statusnotifieritemjob.cpp
    statusnotifieritemhost.cpp
    statusnotifieritemservice.cpp
    statusnotifieritemsource.cpp
    systemtraysettings.cpp
    systemtraytypes.cpp
)

qt_add_dbus_interface(systemtray_SRCS ${KSTATUSNOTIFIERITEM_DBUS_INTERFACES_DIR}/kf6_org.kde.StatusNotifierWatcher.xml statusnotifierwatcher_interface)
qt_add_dbus_interface(systemtray_SRCS ${lingmo-workspace_SOURCE_DIR}/libkmpris/dbus/org.freedesktop.DBus.Properties.xml dbusproperties)

set(statusnotifieritem_xml ${KSTATUSNOTIFIERITEM_DBUS_INTERFACES_DIR}/kf6_org.kde.StatusNotifierItem.xml)
set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "systemtraytypes.h"
   CLASSNAME OrgKdeStatusNotifierItem
)
qt_add_dbus_interface(systemtray_SRCS ${statusnotifieritem_xml} statusnotifieritem_interface)

ecm_qt_declare_logging_category(systemtray_SRCS HEADER debug.h
                                            IDENTIFIER SYSTEM_TRAY
                                            CATEGORY_NAME kde.systemtray
                                            DEFAULT_SEVERITY Info
                                            DESCRIPTION "lingmo applet systemtray"
                                            EXPORT LINGMOWORKSPACE
                                        )
add_library(systemtray_static STATIC ${systemtray_SRCS})
target_link_libraries(systemtray_static
    PUBLIC
        Qt::DBus
        Qt::Gui
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        Qt::Quick
        KF6::ConfigCore
        KF6::IconThemes
        KF6::I18n
        KF6::ItemModels
        KF6::WindowSystem
        KF6::XmlGui
        Lingmo::Lingmo
        Lingmo::Lingmo5Support
        Lingmo::LingmoQuick
        dbusmenuqt
)

set_property(TARGET systemtray_static PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(systemtraymodel_static STATIC systemtraymodel.cpp sortedsystemtraymodel.cpp)
target_link_libraries(systemtraymodel_static PUBLIC systemtray_static)
set_property(TARGET systemtraymodel_static PROPERTY POSITION_INDEPENDENT_CODE ON)

kcoreaddons_add_plugin(org.kde.lingmo.private.systemtray SOURCES systemtray.cpp INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.private.systemtray systemtraymodel_static)

ecm_add_qml_module(systemtrayplugin URI org.kde.lingmo.private.systemtray GENERATE_PLUGIN_SOURCE)
target_sources(systemtrayplugin PRIVATE systemtraymodel.cpp)
target_link_libraries(systemtrayplugin PRIVATE systemtray_static)
ecm_finalize_qml_module(systemtrayplugin)

install(FILES statusnotifieritem.operations DESTINATION ${LINGMO5SUPPORT_DATA_INSTALL_DIR}/services)

add_subdirectory(container)
if(BUILD_TESTING)
  add_subdirectory(autotests)
  add_subdirectory(tests)
endif()
