# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (lingmoactivities-imports)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Gui Qml Quick Sql)
find_package (KF6Config     ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6CoreAddons ${KF6_MIN_VERSION} CONFIG REQUIRED)

ecm_add_qml_module(lingmoactivitiesextensionplugin URI "org.kde.activities" VERSION 0.1 GENERATE_PLUGIN_SOURCE DEPENDENCIES QtCore)

target_sources(lingmoactivitiesextensionplugin PRIVATE
   activitymodel.cpp
   activitymodel.h
   activityinfo.cpp
   activityinfo.h

   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/dbusfuture_p.cpp
)

target_link_libraries(
   lingmoactivitiesextensionplugin PRIVATE
   Qt6::Core
   Qt6::DBus
   Qt6::Gui
   Qt6::Qml
   Qt6::Quick
   Qt6::Sql
   Lingmo::Activities
   KF6::ConfigCore
   KF6::CoreAddons
   Boost::headers
)

ecm_finalize_qml_module(lingmoactivitiesextensionplugin DESTINATION ${KDE_INSTALL_QMLDIR})
