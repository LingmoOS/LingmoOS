/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import Qt5Compat.GraphicalEffects
import org.kde.lingmoui 2.20 as LingmoUI

Item {
    implicitWidth: parent.width/1.2
    implicitHeight: LingmoUI.Units.gridUnit * 1.6

    Rectangle {
        id: view
        anchors.fill: parent
        radius: LingmoUI.Units.smallSpacing/2
        color: viewBackgroundColor
        MouseArea {
            id: viewMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: input.focus = true
        }
        TextInput {
            id: input
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                margins: LingmoUI.Units.smallSpacing
            }
            text: i18n("Text input…")
            color: viewTextColor
        }
        Rectangle {
            anchors.fill: parent
            radius: LingmoUI.Units.smallSpacing/2
            visible: viewMouse.containsMouse || input.focus
            color: "transparent"
            border {
                color: viewMouse.containsMouse ? viewHoverColor : viewFocusColor
            }
        }
    }
    InnerShadow {
        anchors.fill: view
        horizontalOffset: 0
        verticalOffset: LingmoUI.Units.smallSpacing/4
        radius: LingmoUI.Units.smallSpacing
        samples: 16
        color: viewMouse.pressed ? Qt.rgba(0, 0, 0, 0) : Qt.rgba(0, 0, 0, 0.5)
        source: view
    }
}
