/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

import org.kde.plasma.welcome

Item {
    id: root

    default property alias children: appletContainer.children

    // NOTE: Distros overriding our default floating panels should patch this to false
    readonly property bool floating: true

    readonly property int floatingMargins: Kirigami.Units.largeSpacing // Floating margins
    readonly property int panelHeight: 44 // Default panel height

    height: panelHeight + (floating ? (floatingMargins * 2) : 0)

    implicitWidth: appletContainer.implicitWidth + (floating ? (floatingMargins * 2) : 0)
                   + appletContainer.anchors.leftMargin + appletContainer.anchors.rightMargin

    // Panel shadow
    KSvg.FrameSvgItem {
        visible: root.floating

        anchors.fill: panelContainer
        anchors.topMargin: -margins.top
        anchors.leftMargin: -margins.left
        anchors.rightMargin: -margins.right
        anchors.bottomMargin: -margins.bottom

        imagePath: "solid/widgets/panel-background"
        prefix: "shadow"
    }

    Item {
        id: panelContainer

        height: root.panelHeight

        anchors.fill: root
        anchors.margins: root.floating ? root.floatingMargins : 0

        // Panel background
        KSvg.FrameSvgItem {
            anchors.fill: panelContainer
            imagePath: "widgets/panel-background"
            enabledBorders: root.floating ? "AllBorders" : "TopBorder"
        }

        RowLayout {
            id: appletContainer
            anchors.fill: panelContainer
            anchors.leftMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing

            spacing: Kirigami.Units.smallSpacing
        }
    }
}
