/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

Item {
    id: trayIcon

    property string source: "start-here-kde-symbolic"

    implicitWidth: 30
    implicitHeight: 30

    Kirigami.Icon {
        anchors.centerIn: trayIcon

        implicitWidth: Kirigami.Units.iconSizes.smallMedium
        implicitHeight: Kirigami.Units.iconSizes.smallMedium

        source: trayIcon.source
    }
}
