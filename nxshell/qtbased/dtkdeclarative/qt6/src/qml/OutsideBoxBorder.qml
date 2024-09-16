// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0

Item {
    property real borderWidth: 1
    property color color: "white"
    property real radius: 0

    Rectangle {
        id: rect

        color: "transparent"
        radius: parent.radius + border.width

        anchors {
            fill: parent
            margins: -border.width
        }

        border {
            width: borderWidth
            color: parent.color
        }
    }
}
