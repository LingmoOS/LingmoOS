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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import org.lingmo.quick.items 1.0

Rectangle {
    color: "transparent"

    Image {
        id: image
        width: 1600
        height: 900
        source: "file:///usr/share/backgrounds/2-lingmo.jpg"
    }
    BlurItem {
        x: 100
        y: 100
        width: 500
        height: 500
        radius: 40
        samples: 81
        source: image
        Drag.active: dragArea.drag.active
        MouseArea {
            id: dragArea
            anchors.fill: parent
            drag.target: parent
        }
    }
}
