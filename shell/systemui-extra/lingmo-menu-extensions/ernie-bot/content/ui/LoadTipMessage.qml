/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

Rectangle {
    color: Qt.rgba(255, 255, 255, 0.45)
    border.width: 1
    border.color: Qt.rgba(0, 0, 0, 0.1)
    radius: 6

    Component.onCompleted: {
        animation.start();
    }

    RotationAnimation {
        id: animation
        target: pathView
        to: 360
        duration: 1500
        direction: RotationAnimation.Clockwise

        onStopped: {
            if (pathView.rotation === 360) {
                pathView.rotation = 0;
            }

            start();
        }
    }

    PathView {
        id: pathView
        anchors.centerIn: parent
        width: 20; height: 20

        path: Path {
            startX: 10; startY: 0
            PathArc {
                x: 10; y: 20
                radiusX: 10
                radiusY: 10
                useLargeArc: true
            }
            PathArc {
                x: 10; y: 0
                radiusX: 10
                radiusY: 10
                useLargeArc: true
            }
        }
        interactive: false
        model: 8
        delegate: Rectangle {
            width: 5; height: 5
            radius: width/2
            color: "#BF3790FA"
        }
    }
}
