// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

import org.deepin.ds.dock 1.0
import org.deepin.dtk

Item {
    id: control
    property bool isActive
    property real radius: 4

    implicitWidth: 24
    implicitHeight: 24
    property Palette backgroundColor: Palette {
        normal {
            common: ("transparent")
        }
        hovered {
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.15)
        }
        hoveredDark {
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.15)
        }
        pressed {
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.25)
        }
        pressedDark {
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.25)
        }
    }
    property Palette activeBackgroundColor: Palette {
        normal {
            common: ("transparent")
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.15)
        }
        normalDark: normal
        hovered {
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.25)
        }
        hoveredDark: hovered
        pressed {
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.30)
        }
        pressedDark: pressed
    }
    property Palette insideBorderColor: Palette {
        normal {
            common: ("transparent")
        }
        hovered {
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.10)
        }
        hoveredDark: hovered
        pressed: hovered
        pressedDark: pressed
    }
    property Palette activeInsideBorderColor: Palette {
        normal {
            common: ("transparent")
            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.10)
        }
        normalDark: normal
    }
    property Palette outsideBorderColor: Palette {
        normal {
            common: ("transparent")
        }
        hovered {
            crystal: Qt.rgba(0.0, 0.0, 0.0, 0.10)
        }
        hoveredDark: hovered
        pressed: hovered
        pressedDark: pressed
    }
    property Palette activeOutsideBorderColor: Palette {
        normal {
            common: ("transparent")
            crystal: Qt.rgba(0.0, 0.0, 0.0, 0.10)
        }
        normalDark: normal
    }

    Rectangle {
        anchors.fill: parent
        radius: control.radius
        color: isActive ? control.ColorSelector.activeBackgroundColor
                        : control.ColorSelector.backgroundColor
    }
    InsideBoxBorder {
        anchors.fill: parent
        radius: control.radius
        color: isActive ? control.ColorSelector.activeInsideBorderColor
                        : control.ColorSelector.insideBorderColor
        borderWidth: 1 / Screen.devicePixelRatio
    }
    OutsideBoxBorder {
        anchors.fill: parent
        radius: control.radius
        color: isActive ? control.ColorSelector.activeOutsideBorderColor
                        : control.ColorSelector.outsideBorderColor
        borderWidth: 1 / Screen.devicePixelRatio
    }
}
