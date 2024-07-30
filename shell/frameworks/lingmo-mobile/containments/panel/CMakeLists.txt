# SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
# SPDX-FileCopyrightText: 2021 Aleix Pol <apol@kde.org>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

set(phonepanel_SRCS
    phonepanel.cpp
)

add_library(org.kde.lingmo.mobile.panel MODULE ${phonepanel_SRCS})

target_link_libraries(org.kde.lingmo.mobile.panel
                      Qt::Gui
                      Qt::DBus
                      Lingmo::Lingmo
                      KF6::I18n
                      KF6::Service
                     )

install(TARGETS org.kde.lingmo.mobile.panel DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/applets)
#install(FILES lingmo-phonepanel-default.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

lingmo_install_package(package org.kde.lingmo.mobile.panel)

