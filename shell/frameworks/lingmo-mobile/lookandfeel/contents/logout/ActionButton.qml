/*
 *   SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *   SPDX-FileCopyrightText: 2022 Seshan Ravikumar <seshan10@me.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.8

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.20 as LingmoUI

Item {
    id: root
    property alias text: label.text
    property alias iconSource: icon.source
    property alias containsMouse: mouseArea.containsMouse
    property alias font: label.font
    property alias labelRendering: label.renderType
    property alias circleOpacity: buttonRect.opacity
    property alias circleVisiblity: buttonRect.visible
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    signal clicked

    activeFocusOnTab: true

    property int iconSize: LingmoUI.Units.gridUnit

    implicitWidth: LingmoUI.Units.gridUnit * 14
    implicitHeight: iconSize + LingmoUI.Units.smallSpacing + label.implicitHeight

    Rectangle {
        id: buttonRect
        width: root.width
        height: iconSize * 2.2
        radius: LingmoUI.Units.cornerRadius
        color: LingmoUI.Theme.backgroundColor
        opacity: mouseArea.containsPress ? 1 : 0.6
        border {
            color: Qt.rgba(255, 255, 255, 0.8)
            width: 1
        }
    }

    LingmoUI.Icon {
        id: icon
        anchors {
            verticalCenter: buttonRect.verticalCenter
            left: buttonRect.left
            leftMargin: LingmoUI.Units.mediumSpacing
        }
        width: iconSize
        height: iconSize
    }

    LingmoComponents3.Label {
        id: label
        font.pointSize: LingmoUI.Theme.defaultFont.pointSize + 1
        anchors {
            centerIn: buttonRect
        }
        style: softwareRendering ? Text.Outline : Text.Normal
        styleColor: softwareRendering ? LingmoUI.Theme.backgroundColor : "transparent" //no outline, doesn't matter
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        wrapMode: Text.WordWrap
        font.underline: root.activeFocus
    }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        onClicked: root.clicked()
        anchors.fill: parent
    }

    Keys.onEnterPressed: clicked()
    Keys.onReturnPressed: clicked()
    Keys.onSpacePressed: clicked()

    Accessible.onPressAction: clicked()
    Accessible.role: Accessible.Button
    Accessible.name: label.text
}
