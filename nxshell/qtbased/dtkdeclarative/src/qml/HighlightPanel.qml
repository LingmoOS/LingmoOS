// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Item {
    id: panel

    property D.Palette backgroundColor: DS.Style.highlightPanel.background
    property D.Palette outerShadowColor: DS.Style.highlightPanel.dropShadow
    property D.Palette innerShadowColor: DS.Style.highlightPanel.innerShadow

    implicitWidth: DS.Style.highlightPanel.width
    implicitHeight: DS.Style.highlightPanel.height
    BoxShadow {
        anchors.fill: backgroundRect
        shadowColor: panel.D.ColorSelector.outerShadowColor
        shadowOffsetY: 4
        shadowBlur: 6
        cornerRadius: backgroundRect.radius
    }

    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: panel.D.ColorSelector.backgroundColor
        radius: DS.Style.control.radius
    }

    BoxInsetShadow {
        anchors.fill: backgroundRect
        shadowColor: panel.D.ColorSelector.innerShadowColor
        shadowOffsetY: -1
        shadowBlur: 2
        spread: 1
        cornerRadius: backgroundRect.radius
    }
}
