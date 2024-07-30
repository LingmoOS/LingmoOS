# SPDX-FileCopyrightText: 2022-2023 Devin Lin <devin@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.mobile.homescreen.halcyon\")

set(homescreen_SRCS
    homescreen.cpp
)

add_library(org.kde.lingmo.mobile.homescreen.halcyon MODULE ${homescreen_SRCS})

target_link_libraries(org.kde.lingmo.mobile.homescreen.halcyon
                      Qt::Gui
                      Qt::Qml
                      Qt::Quick
                      Lingmo::Lingmo
                      KF6::I18n
                      KF6::Service
                      KF6::KIOGui
                      KF6::Notifications
                      Lingmo::KWaylandClient
                      KF6::WindowSystem
)

install(TARGETS org.kde.lingmo.mobile.homescreen.halcyon DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/applets)

lingmo_install_package(package org.kde.lingmo.mobile.homescreen.halcyon)

add_subdirectory(plugin)
