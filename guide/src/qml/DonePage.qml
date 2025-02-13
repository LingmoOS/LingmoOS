/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     LingmoOS Team <team@lingmo.org>
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
import LingmoUI 1.0 as LingmoUI
import Lingmo.Setup 1.0

Item {
    id: control

    FirstSetupUI {
        id: setup
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Item {
            height: LingmoUI.Units.largeSpacing
        }

        RoundedItem {
            id: im

            Image {
                id: logo
                width: 350
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: im.width/20
                sourceSize: Qt.size(width, height)
                source: "qrc:/assets/welcome.svg"
            }

            Text {
                id: hello
                anchors.left: logo.right
                anchors.leftMargin: logo.width/6
                anchors.top: parent.top
                anchors.topMargin: parent.height/2.5
                text: qsTr("Done")
                font.pointSize: 25
                font.bold: true
                color: LingmoUI.Theme.textColor
            }

            Text {
                id: txt
                anchors.left: logo.right
                anchors.top: hello.bottom
                anchors.topMargin: 5
                anchors.leftMargin: logo.width/6
                anchors.verticalCenter: hello.verticalCenter
                text: qsTr("Enjoy Lingmo OS!")
                color: LingmoUI.Theme.textColor
                font.pointSize: 15
            }

            Button {
                // anchors.centerIn: parent
                id: exBt
                anchors.right: parent.right
                anchors.rightMargin: parent.width/20
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 35
                flat: true
                text: qsTr("Enter the system")
                onClicked: {
                    setup.configInstall()
                    Qt.quit()
                }
            }

            Button {
                // anchors.centerIn: parent
                id: back
                anchors.right: exBt.left
                anchors.rightMargin: 5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 35
                flat: false
                text: qsTr("Back")
                onClicked: {
                    stackView.pop()
                }
            }
        }
    }
}