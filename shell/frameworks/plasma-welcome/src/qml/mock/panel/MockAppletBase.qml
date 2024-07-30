/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

import org.kde.plasma.welcome

Item {
    id: applet

    property bool active: false

    implicitHeight: 44 // Default panel height

    // Styled after System Tray icons - the implementation for panel
    // applets does not currently use animations so this looks better
    KSvg.FrameSvgItem {
        anchors.fill: parent

        imagePath: "widgets/tabbar"
        prefix: "south-active-tab"

        opacity: active ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: active ? Easing.OutCubic : Easing.InCubic
            }
        }
    }
}
