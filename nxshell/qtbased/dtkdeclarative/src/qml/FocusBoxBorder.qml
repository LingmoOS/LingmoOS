// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Item {
    id: control
    property color color: "transparent"
    property real borderWidth: DS.Style.control.focusBorderWidth
    property real radius: 0

    BoxShadow {
        anchors.fill: _border
        shadowColor: D.DTK.makeColor(parent.color).opacity(-50).color()
        cornerRadius: _border.radius
        shadowBlur: 4
        hollow: true
    }

    Rectangle {
        id: _border

        readonly property real paddings: borderWidth + DS.Style.control.focusBorderPaddings

        anchors {
            fill: parent
            margins: -paddings
        }

        radius: parent.radius + paddings
        border { width: borderWidth; color: parent.color }
        color: "transparent"
    }
}
