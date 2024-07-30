add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.kscreen\")

set(kscreenapplet_SRCS
    kscreenapplet.cpp
    ../common/osdaction.cpp
)

add_library(org.kde.kscreen MODULE ${kscreenapplet_SRCS})

target_link_libraries(org.kde.kscreen
                      Qt::Qml
                      Qt::DBus
                      KF6::I18n
                      Plasma::Plasma
                      KF6::Screen)

install(TARGETS org.kde.kscreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

plasma_install_package(package org.kde.kscreen)
