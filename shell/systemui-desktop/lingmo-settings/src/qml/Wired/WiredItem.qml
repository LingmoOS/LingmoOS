/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import LingmoUI 1.0 as LingmoUI
import Lingmo.NetworkManagement 1.0 as NM

Item {
    id: control

    Rectangle {
        anchors.fill: parent
        radius: LingmoUI.Theme.smallRadius
        color: mouseArea.containsMouse ? Qt.rgba(LingmoUI.Theme.textColor.r,
                                                 LingmoUI.Theme.textColor.g,
                                                 LingmoUI.Theme.textColor.b,
                                                 0.1) : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: 125
                easing.type: Easing.InOutCubic
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if (model.connectionState === NM.Enums.Deactivated) {
                handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
            } else {
                handler.deactivateConnection(model.connectionPath, model.devicePath)
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.smallSpacing
        spacing: LingmoUI.Units.largeSpacing

        Image {
            width: 22
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark/" : "light/") + "network-wired.svg"
        }

        Label {
            text: model.itemUniqueName
            Layout.fillWidth: true
        }

        // Activated
        Image {
            width: 16
            height: width
            sourceSize: Qt.size(width, height)
            source: "qrc:/images/light/checked.svg"
            visible: model.connectionState === NM.NetworkModel.Activated

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: LingmoUI.Theme.highlightColor
                opacity: 1
                visible: true
            }
        }
    }
}
