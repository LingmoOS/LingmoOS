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
    id: homePage

    Appearance {
        id: appearance
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Item {
            height: LingmoUI.Units.largeSpacing
        }

        RoundedItem {
            id: cr

            RowLayout {
                anchors.centerIn: parent
                spacing: LingmoUI.Units.largeSpacing * 2

                IconCheckBox {
                    source: "qrc:/assets/light_mode.svg"
                    text: qsTr("Light")
                    checked: !LingmoUI.Theme.darkMode
                    onClicked: appearance.switchDarkMode(false)
                }

                IconCheckBox {
                    source: "qrc:/assets/dark_mode.svg"
                    text: qsTr("Dark")
                    checked: LingmoUI.Theme.darkMode
                    onClicked: appearance.switchDarkMode(true)
                }
            }

            Button {
                // anchors.centerIn: parent
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                flat: true
                text: qsTr("Start")
                onClicked: {
                    stackView.push(secondPage)
                }
            }
            
            // Button {
            //     flat: false
            //     text: qsTr("Exit")
            //     onClicked: Qt.quit()
            // }
        }
    }
}