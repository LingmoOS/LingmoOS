add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.kscreen\")

set(kscreenapplet_SRCS
    kscreenapplet.cpp
    ../common/osdaction.cpp
)

add_library(plasma_applet_kscreen MODULE ${kscreenapplet_SRCS})

kcoreaddons_desktop_to_json(plasma_applet_kscreen package/metadata.desktop)

target_link_libraries(plasma_applet_kscreen
                      Qt::Qml
                      Qt::DBus
                      KF5::I18n
                      KF5::Plasma
                      KF5::Screen)

install(TARGETS plasma_applet_kscreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

plasma_install_package(package org.kde.kscreen)
