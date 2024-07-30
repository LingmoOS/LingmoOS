/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg

import org.kde.lingmo.welcome

Item {
    id: applet

    property bool active: false
    property int activeIconIndex: -1 // -1 entire applet, else index of tray icon

    default property alias children: iconsLayout.children

    implicitWidth: trayLayout.width
    implicitHeight: 44 // Default panel height

    Layout.preferredWidth: implicitWidth

    KSvg.FrameSvgItem {
        x: ((activeIconIndex == -1) ? 0 : (30 * activeIconIndex + 2)) - LingmoUI.Units.largeSpacing
        y: parent.y

        width: ((activeIconIndex === -1) ? parent.width : 30) + (LingmoUI.Units.largeSpacing * 2)
        height: parent.height

        Behavior on x {
            NumberAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutCubic
            }
        }

        Behavior on width {
            NumberAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutCubic
            }
        }

        imagePath: "widgets/tabbar"
        prefix: "south-active-tab"

        opacity: active ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: active ? Easing.OutCubic : Easing.InCubic
            }
        }
    }

    RowLayout {
        id: trayLayout
        anchors.centerIn: applet

        spacing: 0

        RowLayout {
            id: iconsLayout
            Layout.leftMargin: 2  // For some reason, there's a
            Layout.rightMargin: 2 // 2px gap around the tray

            spacing: 0
        }

        LingmoUI.Icon {
            implicitWidth: LingmoUI.Units.iconSizes.smallMedium
            implicitHeight: LingmoUI.Units.iconSizes.smallMedium

            source: "arrow-up"
        }
    }
}
