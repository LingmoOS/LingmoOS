/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Controls 2.15
import org.kde.lingmoui as LingmoUI

Item {
    implicitWidth: parent.width/1.2
    implicitHeight: LingmoUI.Units.gridUnit * 1.6

    LingmoUI.ShadowedRectangle {
        id: button
        anchors.fill: parent
        radius: LingmoUI.Units.smallSpacing/2
        color: buttonMouse.pressed ? Qt.darker(buttonBackgroundColor, 1.5) : buttonBackgroundColor
        shadow {
            xOffset: 0
            yOffset: LingmoUI.Units.smallSpacing/4
            size: LingmoUI.Units.smallSpacing
            color: buttonMouse.pressed ? Qt.rgba(0, 0, 0, 0) : Qt.rgba(0, 0, 0, 0.5)
        }
        Label {
            anchors.centerIn: parent
            text: i18n("Button")
            color: buttonTextColor
        }
        MouseArea {
            id: buttonMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: button.focus = true
        }
        Rectangle {
            anchors.fill: parent
            radius: LingmoUI.Units.smallSpacing/2
            visible: buttonMouse.containsMouse || button.focus
            color: "transparent"
            border {
                color: buttonMouse.containsMouse ? buttonHoverColor : buttonFocusColor
            }
        }
    }
}
