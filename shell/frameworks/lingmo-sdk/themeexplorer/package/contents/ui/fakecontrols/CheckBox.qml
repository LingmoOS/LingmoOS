/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1
import org.kde.lingmoui 2.20 as LingmoUI

MouseArea {
    id: buttonMouse
    hoverEnabled: true
    implicitWidth: parent.width/1.2
    implicitHeight: layout.height 

    property bool checked: true
    //in real controls this is done by the color scope
    property bool complementary: false

    onClicked: {
        buttonMouse.focus = true
        checked = !checked
    }
    RowLayout {
        id: layout
        height: LingmoUI.Units.gridUnit * 1.6
        LingmoUI.ShadowedRectangle {
            id: button
            radius: LingmoUI.Units.smallSpacing/2
            color: buttonMouse.pressed ? Qt.darker(buttonBackgroundColor, 1.5) : buttonBackgroundColor
            height: parent.height / 1.2
            width: height
            shadow {
                xOffset: 0
                yOffset: LingmoUI.Units.smallSpacing/4
                size: LingmoUI.Units.smallSpacing
                color: buttonMouse.pressed ? Qt.rgba(0, 0, 0, 0) : Qt.rgba(0, 0, 0, 0.5)
            }
            Rectangle {
                anchors.fill: parent
                radius: LingmoUI.Units.smallSpacing/2
                visible: buttonMouse.containsMouse || buttonMouse.focus
                color: "transparent"
                border {
                    color: {
                        if (buttonMouse.checked) {
                            return buttonMouse.complementary ? complementaryHoverColor : highlightColor;
                        } else if (buttonMouse.containsMouse) {
                            return buttonHoverColor;
                        } else {
                           return buttonFocusColor;
                        }
                    }
                }
            }
            Rectangle {
                anchors {
                    fill: parent
                    margins: LingmoUI.Units.smallSpacing
                }
                radius: LingmoUI.Units.smallSpacing/2
                visible: buttonMouse.checked
                color: buttonMouse.complementary ? complementaryHoverColor : highlightColor
            }
        }
        Label {
            text: i18n("Checkbox")
            color: buttonMouse.complementary ? complementaryTextColor : textColor
        }
    }
}
