# SPDX-FileCopyrightText: 2015-2016 Marco Martin <mart@kde.org>
# SPDX-FileCopyrightText: 2021 Aleix Pol <apol@kde.org>
# SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

add_library(org.kde.lingmo.mobile.taskpanel MODULE taskpanel.cpp ${DBUS_SRCS})

target_link_libraries(org.kde.lingmo.mobile.taskpanel
                      Qt::Gui
                      Qt::DBus
                      Qt::Qml
                      Qt::Quick
                      Lingmo::Lingmo
                     )


install(TARGETS org.kde.lingmo.mobile.taskpanel DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/applets)

lingmo_install_package(package org.kde.lingmo.mobile.taskpanel)

