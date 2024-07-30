/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick

import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

Item {
    id: trayIcon

    property string source: "start-here-kde-symbolic"

    implicitWidth: 30
    implicitHeight: 30

    LingmoUI.Icon {
        anchors.centerIn: trayIcon

        implicitWidth: LingmoUI.Units.iconSizes.smallMedium
        implicitHeight: LingmoUI.Units.iconSizes.smallMedium

        source: trayIcon.source
    }
}
