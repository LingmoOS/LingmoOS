/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

import QtQuick 2.15

Item {
    width: childrenRect.width
    height: childrenRect.height
    ListView {
        orientation: ListView.Horizontal
        layoutDirection: ListView.LeftToRight
        width: 364
        height: 20
        interactive: false

        model: ListModel {
            ListElement {text: qsTr("Mon")}
            ListElement {text: qsTr("Tue")}
            ListElement {text: qsTr("Wed")}
            ListElement {text: qsTr("Thu")}
            ListElement {text: qsTr("Fri")}
            ListElement {text: qsTr("Sat")}
            ListElement {text: qsTr("Sun")}
        }

        delegate: Rectangle {
            width: 52
            height: 20
            color: "transparent"

            Text {
                anchors.centerIn: parent
                font.pixelSize: 14
                font.bold: true
                text: model.text
            }
        }
    }
}
