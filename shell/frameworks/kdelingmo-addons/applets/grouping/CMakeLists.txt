add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.private.grouping\")

lingmo_install_package(package org.kde.lingmo.private.grouping)
kcoreaddons_add_plugin(org.kde.lingmo.private.grouping SOURCES groupingcontainment.cpp INSTALL_NAMESPACE "lingmo/applets")
target_link_libraries(org.kde.lingmo.private.grouping
    Qt::Gui
    Qt::Quick
    Qt::DBus
    Qt::Widgets
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::XmlGui
    KF6::I18n
)

add_subdirectory(container)
