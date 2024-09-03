/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
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
import Lingmo.Updator 1.0

Item {
    id: control

    ColumnLayout {
        anchors.fill: parent

        LingmoUI.BusyIndicator {
            width: 32
            height: 32
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            height: LingmoUI.Units.smallSpacing
        }

        Label {
            text: qsTr("Updating, please wait")
            Layout.alignment: Qt.AlignHCenter
        }

        ScrollView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true

            TextArea {
                id: _textArea
                text: updator.statusDetails
                enabled: false

                background: Item {
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: LingmoUI.Units.largeSpacing
                        radius: LingmoUI.Theme.smallRadius
                        color: LingmoUI.Theme.secondBackgroundColor
                    }
                }

                leftPadding: LingmoUI.Units.largeSpacing * 2
                rightPadding: LingmoUI.Units.largeSpacing * 2
                topPadding: LingmoUI.Units.largeSpacing * 2
                bottomPadding: LingmoUI.Units.largeSpacing * 2

                // Auto scroll to bottom.
                onTextChanged: {
                    _textArea.cursorPosition = _textArea.text.length - 1
                }
            }
        }
    }
}
