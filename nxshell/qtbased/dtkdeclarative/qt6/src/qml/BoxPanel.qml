// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Item {
    id: control

    property int radius: DS.Style.control.radius
    property D.Palette color1: DS.Style.button.background1
    property D.Palette color2: DS.Style.button.background2
    property D.Palette insideBorderColor: DS.Style.button.insideBorder
    property D.Palette outsideBorderColor: DS.Style.button.outsideBorder
    property D.Palette dropShadowColor: DS.Style.button.dropShadow
    property D.Palette innerShadowColor1: DS.Style.button.innerShadow1
    property D.Palette innerShadowColor2: DS.Style.button.innerShadow2
    property int boxShadowBlur: 6
    property int boxShadowOffsetY: 4
    property int innerShadowOffsetY1: -1
    // Background color changes with hover state if `backgroundFlowingHovered` is `true`.
    property bool backgroundFlowsHovered: true

    Loader {
        anchors.fill: backgroundRect
        active: control.D.ColorSelector.family === D.Palette.CommonColor
        sourceComponent: BoxShadow {
            shadowBlur: control.boxShadowBlur
            shadowOffsetY: control.boxShadowOffsetY
            shadowColor: control.D.ColorSelector.dropShadowColor
            cornerRadius: backgroundRect.radius
        }
    }

    Rectangle {
        id: backgroundRect
        property alias color1: control.color1
        property alias color2: control.color2
        D.ColorSelector.hovered: backgroundFlowsHovered ? undefined : false
        objectName: "background"

        Gradient {
            id: backgroundGradient
            // Use the backgroundRect's colorselecor can filter the hovered state.
            GradientStop { position: 0.0; color: backgroundRect.D.ColorSelector.color1}
            GradientStop { position: 1.0; color: backgroundRect.D.ColorSelector.color2}
        }

        anchors.fill: parent
        radius: control.radius
        gradient: D.ColorSelector.color1 === D.ColorSelector.color2 ? null : backgroundGradient
        color: D.ColorSelector.color1
    }

    Loader {
        anchors.fill: backgroundRect
        readonly property color innerShadowColor: control.D.ColorSelector.innerShadowColor1
        active: innerShadowColor1 && innerShadowColor.a !== 0 && control.D.ColorSelector.family === D.Palette.CommonColor
        z: D.DTK.AboveOrder

        sourceComponent: BoxInsetShadow {
            shadowBlur: 2
            shadowOffsetY: control.innerShadowOffsetY1
            spread: 1
            shadowColor: innerShadowColor
            cornerRadius: backgroundRect.radius
        }
    }

    Loader {
        anchors.fill: backgroundRect
        readonly property color innerShadowColor: control.D.ColorSelector.innerShadowColor2
        active: innerShadowColor2 && innerShadowColor.a !== 0 && control.D.ColorSelector.family === D.Palette.CommonColor
        z: D.DTK.AboveOrder

        sourceComponent: BoxInsetShadow {
            shadowBlur: 1
            shadowOffsetY: 1
            shadowColor: innerShadowColor
            cornerRadius: backgroundRect.radius
        }
    }

    Loader {
        active: insideBorderColor
        anchors.fill: backgroundRect
        z: D.DTK.AboveOrder

        sourceComponent: InsideBoxBorder {
            radius: backgroundRect.radius
            color: control.D.ColorSelector.insideBorderColor
            borderWidth: DS.Style.control.borderWidth
        }
    }

    Loader {
        active: outsideBorderColor
        anchors.fill: backgroundRect
        z: D.DTK.AboveOrder

        sourceComponent: OutsideBoxBorder {
            radius: backgroundRect.radius
            color: control.D.ColorSelector.outsideBorderColor
            borderWidth: DS.Style.control.borderWidth
        }
    }
}
