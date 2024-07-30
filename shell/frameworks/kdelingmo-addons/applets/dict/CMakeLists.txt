find_package(Qt6WebEngineQuick ${QT_MIN_VERSION} CONFIG)

set_package_properties(Qt6WebEngineQuick PROPERTIES
    PURPOSE "Needed by the dict applet"
    URL "https://doc.qt.io/qt-6/qtwebengine-index.html"
    TYPE OPTIONAL
)

if (NOT Qt6WebEngineQuick_FOUND)
    return()
endif()

lingmo_install_package(package org.kde.lingmo_applet_dict)
# TODO: fix pluginid "org.kde.lingmo_applet_dict" into properly namespaced id "org.kde.lingmo.dict"

ecm_install_icons(
    ICONS sc-apps-accessories-dictionary.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

#######################################################################################
add_definitions(-DTRANSLATION_DOMAIN="lingmo_applet_org.kde.lingmo_applet_dict")

ecm_add_qml_module(dictplugin URI org.kde.lingmo.private.dict)
target_sources(dictplugin PRIVATE
  plugin/dict_plugin.cpp
  plugin/dict_object.cpp
  plugin/dictionariesmodel.cpp
)
target_link_libraries(dictplugin PRIVATE
    KF6::I18n
    Qt::Quick
    Qt::WebEngineQuick
    engine_dict_static
)
ecm_finalize_qml_module(dictplugin)
