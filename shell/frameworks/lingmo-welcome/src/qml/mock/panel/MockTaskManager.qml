/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

MockAppletBase {
    id: applet

    implicitWidth: tasksLayout.implicitWidth

    RowLayout {
        id: tasksLayout

        height: applet.height

        spacing: 0

        Repeater {
            model: ["org.kde.dolphin", "systemsettings", "lingmodiscover"]
            delegate: LingmoUI.Icon {
                Layout.leftMargin: 10 // Hardcoded from original
                Layout.rightMargin: 10 // Hardcoded from original

                implicitWidth: LingmoUI.Units.iconSizes.medium
                implicitHeight: LingmoUI.Units.iconSizes.medium

                source: modelData
            }
        }
    }
}
