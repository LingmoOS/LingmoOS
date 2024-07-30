add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.weather\")

kcoreaddons_add_plugin(org.kde.lingmo.weather SOURCES weatherapplet.cpp INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.weather
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::UnitConversion
    KF6::I18n
)

install(FILES wind-arrows.svgz DESTINATION ${LINGMO_DATA_INSTALL_DIR}/desktoptheme/default/weather/)
lingmo_install_package(package org.kde.lingmo.weather)

ecm_add_qml_module(weatherplugin URI org.kde.lingmo.private.weather)

target_sources(weatherplugin PRIVATE
    plugin/plugin.cpp
    plugin/util.cpp
    plugin/abstractunitlistmodel.cpp
    plugin/locationlistmodel.cpp
)
target_link_libraries(weatherplugin PRIVATE
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::UnitConversion
    KF6::I18n
    Qt::Qml
)
ecm_finalize_qml_module(weatherplugin)
