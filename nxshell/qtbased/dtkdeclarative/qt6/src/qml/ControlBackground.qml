// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11

Rectangle {
    id: control

    // 暴露给外部的属性
    property int focusBorderSpace: 1
    property alias focusBorder: contentBorder.border
    property alias focusBorderVisible: contentBorder.visible

    radius: 8
    color: "black"

    Rectangle {
        id: contentBorder

        anchors.centerIn: parent
        width: parent.width + 2 * (control.focusBorderSpace + border.width)
        height: parent.height + 2 * (control.focusBorderSpace + border.width)
        radius: parent.radius + control.focusBorderSpace + border.width
        border { width: 2; color: "black" }
        color: "transparent"
    }
}
