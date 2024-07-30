/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

Item {
    id: root

    readonly property string description: xi18nc("@info:usagetip", "Thank you for testing this beta release of Lingmo — your feedback is fundamental to helping us improve it! Please report any and all bugs you find so that we can fix them.")
    readonly property int iconSize: LingmoUI.Units.iconSizes.enormous

    readonly property list<QtObject> topContent: [
        LingmoUI.UrlButton {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@label:URL", "<b>Find out what’s new</b>")
            url: Controller.releaseUrl
        },
        LingmoUI.UrlButton {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@label:URL", "Report a bug")
            url: "https://bugs.kde.org"
        },
        LingmoUI.UrlButton {
            text: i18nc("@label:URL", "Help work on the next release")
            url: "https://community.kde.org/Get_Involved?source=lingmo-welcome"
        }
    ]

    LingmoUI.Icon {
        id: lingmoIcon
        anchors.centerIn: root

        width: LingmoUI.Units.iconSizes.enormous * 1.5
        height: LingmoUI.Units.iconSizes.enormous * 1.5

        source: "start-here-kde-lingmo"
    }

    LingmoUI.Icon {
        id: gearIcon

        anchors.horizontalCenter: lingmoIcon.right
        anchors.verticalCenter: lingmoIcon.bottom
        anchors.horizontalCenterOffset: -width / 4
        anchors.verticalCenterOffset: -height / 4

        width: LingmoUI.Units.iconSizes.huge
        height: LingmoUI.Units.iconSizes.huge

        source: "process-working-symbolic"

        TapHandler {
            onTapped: gearSpin.start()
        }

        RotationAnimation {
            id: gearSpin

            target: gearIcon
            from: 0
            to: 180
            duration: 800
            easing.type: Easing.InOutCubic
        }

    }
}
