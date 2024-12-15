// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.lingmo.dtk 1.0
import org.lingmo.ds.notification

Control {
    id: control

    padding: radius / 2

    property int radius: 12
    property Palette backgroundColor: Palette {
        normal {
            common: ("transparent")
            crystal: Qt.rgba(240 / 255.0, 240 / 255.0, 240 / 255.0, 1)
        }
        normalDark {
            crystal: Qt.rgba(24 / 255.0, 24 / 255.0, 24 / 255.0, 1)
        }
    }
    property Palette insideBorderColor: Palette {
        normal {
            common: ("transparent")
            crystal: Qt.rgba(255 / 255.0, 255 / 255.0, 255 / 255.0, 0.2)
        }
        normalDark {
            crystal: Qt.rgba(255 / 255.0, 255 / 255.0, 255 / 255.0, 0.1)
        }
    }
    property Palette outsideBorderColor: Palette {
        normal {
            common: ("transparent")
            crystal: Qt.rgba(0, 0, 0, 0.1)
        }
        normalDark {
            crystal: Qt.rgba(0, 0, 0, 0.6)
        }
    }
    property Palette dropShadowColor: Palette {
        normal {
            common: ("transparent")
            crystal: Qt.rgba(0, 0, 0, 0.2)
        }
        normalDark {
            crystal: Qt.rgba(0, 0, 0, 0.4)
        }
    }

    background: Item {
        id: blur
        implicitWidth: 180
        implicitHeight: 40

        Loader {
            anchors.fill: backgroundRect
            active: control.dropShadowColor
            sourceComponent: BoxShadow {
                shadowOffsetX: 0
                shadowOffsetY: 6
                shadowColor: control.ColorSelector.dropShadowColor
                shadowBlur: 20
                cornerRadius: backgroundRect.radius
                spread: 0
                hollow: true
            }
        }

        Rectangle {
            id: backgroundRect
            anchors.fill: parent
            radius: control.radius
            color: control.ColorSelector.backgroundColor
        }

        Loader {
            anchors.fill: backgroundRect
            active: control.insideBorderColor && control.ColorSelector.controlTheme === ApplicationHelper.DarkType
            sourceComponent: InsideBoxBorder {
                radius: backgroundRect.radius
                color: control.ColorSelector.insideBorderColor
            }
        }

        Loader {
            anchors.fill: backgroundRect
            active: control.outsideBorderColor
            sourceComponent: OutsideBoxBorder {
                radius: backgroundRect.radius
                color: control.ColorSelector.outsideBorderColor
            }
        }
    }
}
