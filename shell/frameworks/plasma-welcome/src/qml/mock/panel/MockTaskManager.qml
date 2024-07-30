/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

MockAppletBase {
    id: applet

    implicitWidth: tasksLayout.implicitWidth

    RowLayout {
        id: tasksLayout

        height: applet.height

        spacing: 0

        Repeater {
            model: ["org.kde.dolphin", "systemsettings", "plasmadiscover"]
            delegate: Kirigami.Icon {
                Layout.leftMargin: 10 // Hardcoded from original
                Layout.rightMargin: 10 // Hardcoded from original

                implicitWidth: Kirigami.Units.iconSizes.medium
                implicitHeight: Kirigami.Units.iconSizes.medium

                source: modelData
            }
        }
    }
}
