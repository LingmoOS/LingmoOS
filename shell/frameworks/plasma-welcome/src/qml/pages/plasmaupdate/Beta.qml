/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

Item {
    id: root

    readonly property string description: xi18nc("@info:usagetip", "Thank you for testing this beta release of Plasma — your feedback is fundamental to helping us improve it! Please report any and all bugs you find so that we can fix them.")
    readonly property int iconSize: Kirigami.Units.iconSizes.enormous

    readonly property list<QtObject> topContent: [
        Kirigami.UrlButton {
            Layout.topMargin: Kirigami.Units.largeSpacing
            text: i18nc("@label:URL", "<b>Find out what’s new</b>")
            url: Controller.releaseUrl
        },
        Kirigami.UrlButton {
            Layout.topMargin: Kirigami.Units.largeSpacing
            text: i18nc("@label:URL", "Report a bug")
            url: "https://bugs.kde.org"
        },
        Kirigami.UrlButton {
            text: i18nc("@label:URL", "Help work on the next release")
            url: "https://community.kde.org/Get_Involved?source=plasma-welcome"
        }
    ]

    Kirigami.Icon {
        id: plasmaIcon
        anchors.centerIn: root

        width: Kirigami.Units.iconSizes.enormous * 1.5
        height: Kirigami.Units.iconSizes.enormous * 1.5

        source: "start-here-kde-plasma"
    }

    Kirigami.Icon {
        id: gearIcon

        anchors.horizontalCenter: plasmaIcon.right
        anchors.verticalCenter: plasmaIcon.bottom
        anchors.horizontalCenterOffset: -width / 4
        anchors.verticalCenterOffset: -height / 4

        width: Kirigami.Units.iconSizes.huge
        height: Kirigami.Units.iconSizes.huge

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
