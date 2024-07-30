/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick

import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

MockAppletBase {
    id: applet

    implicitWidth: icon.implicitWidth

    LingmoUI.Icon {
        id: icon
        anchors.centerIn: parent

        implicitWidth: 36  // Hardcoded from original
        implicitHeight: 36 // Hardcoded from original

        source: "start-here-kde-symbolic"
    }
}
