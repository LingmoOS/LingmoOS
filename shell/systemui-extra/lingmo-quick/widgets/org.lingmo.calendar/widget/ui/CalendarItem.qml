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
import org.lingmo.quick.platform 1.0 as Platform

Rectangle {
    property string day: ""
    property string lunar: ""
    property bool isCurrentDay: false
    property bool isCurrentMonth: false

    width: 52
    height: 48
    radius: Platform.Theme.minRadius
    color: isCurrentDay ? "lightblue" : "transparent"
    border.color: "blue"
    border.width: control.containsMouse ? 1 : 0

    MouseArea {

        id: control
        anchors.fill: parent
        hoverEnabled: true

        Item {
            id: dayLabel
            x: 2
            y: 4
            width: 48
            height: 20
            Text {
                anchors.centerIn: parent
                font.pixelSize: 14
                font.bold: true
                opacity: isCurrentMonth ? 1 : 0.3
                text: day
            }
        }

        Item {
            x: 2
            y: dayLabel.y + dayLabel.height
            width: 48
            height: 20
            Text {
                anchors.centerIn: parent
                font.pixelSize: 12
                opacity: 0.3
                text: lunar
            }
        }
    }
}
