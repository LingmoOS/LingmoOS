/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Reion Wong <aj@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property bool checked: false
    property alias icon: _image.source
    property alias text: _label.text

    signal clicked
    signal pressAndHold

    property var backgroundColor: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                        : Qt.rgba(0, 0, 0, 0.05)
    property var hoverColor: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.15)
                                                   : Qt.rgba(0, 0, 0, 0.1)
    property var pressedColor: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.2)
                                                     : Qt.rgba(0, 0, 0, 0.15)

    property var highlightHoverColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.highlightColor, 1.1)
                                                            : Qt.darker(LingmoUI.Theme.highlightColor, 1.1)
    property var highlightPressedColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.highlightColor, 1.1)
                                                              : Qt.darker(LingmoUI.Theme.highlightColor, 1.2)

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()

        onPressedChanged: {
            control.scale = pressed ? 0.95 : 1.0
        }

        onPressAndHold: {
            control.pressAndHold()
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: 200
            easing.type: Easing.OutSine
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: LingmoUI.Theme.bigRadius
        opacity: 1

        color: {
            if (control.checked) {
                if (_mouseArea.pressed)
                    return highlightPressedColor
                else if (_mouseArea.containsMouse)
                    return highlightHoverColor
                else
                    return LingmoUI.Theme.highlightColor
            } else {
                if (_mouseArea.pressed)
                    return pressedColor
                else if (_mouseArea.containsMouse)
                    return hoverColor
                else
                    return backgroundColor
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: LingmoUI.Theme.smallRadius
        anchors.rightMargin: LingmoUI.Theme.smallRadius
        spacing: LingmoUI.Units.smallSpacing

        Item {
            Layout.fillHeight: true
        }

        Image {
            id: _image
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            sourceSize: Qt.size(32, 32)
            asynchronous: true
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: LingmoUI.Units.largeSpacing
            antialiasing: true
            smooth: true

//            ColorOverlay {
//                anchors.fill: _image
//                source: _image
//                color: control.checked ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.disabledTextColor
//            }
        }

        Item {
            Layout.fillHeight: true
        }

        Label {
            id: _label
            color: control.checked ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
            Layout.preferredHeight: control.height * 0.15
            Layout.alignment: Qt.AlignHCenter
            visible: text
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
