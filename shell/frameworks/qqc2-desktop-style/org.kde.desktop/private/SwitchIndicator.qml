/*
    SPDX-FileCopyrightText: 2022 Tanbir Jishan <tantalising007@gmail.com>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

Item {
    id: indicator
    implicitWidth: implicitHeight * 2
    implicitHeight: LingmoUI.Units.gridUnit
    layer.enabled: control.opacity < 1.0

    property T.AbstractButton control
    property alias handle: handle

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false

    QtObject { // colors collected in one place so that main code remains clean and these properties are not exposed
        id: colorFactory

        readonly property color switchBorderColor: indicator.control.checked ? LingmoUI.Theme.highlightColor : handleBorderColor

        readonly property color handleColor: LingmoUI.Theme.backgroundColor
        readonly property color handleBorderColor: (indicator.control.hovered || indicator.control.visualFocus) ? LingmoUI.Theme.hoverColor : LingmoUI.ColorUtils.linearInterpolation(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, LingmoUI.Theme.frameContrast)

        function blendBackgroundWithTextColorWithRatio(factor: double): color {
            // blending of background color with text color for producing a border color. The usual ratios are 70:30, 80:20 and 75:25. The more the background color, the more the contrast.
            return Qt.tint(LingmoUI.Theme.textColor, Qt.alpha(LingmoUI.Theme.backgroundColor, factor))
        }
    }

    Rectangle {
        id: inactive

        anchors {
            fill: parent
            // margins so that the background is a bit shorter than the handle
            topMargin: Math.floor(parent.height / 6)
            bottomMargin: Math.floor(parent.height / 6)
        }

        radius: Math.round(height / 2)
        color: colorFactory.blendBackgroundWithTextColorWithRatio(0.9)
        border.color: colorFactory.switchBorderColor
    }

    Rectangle {
        anchors {
            left: inactive.left
            top: inactive.top
            bottom: inactive.bottom
            right: handle.right
        }

        radius: inactive.radius
        color: Qt.alpha(LingmoUI.Theme.highlightColor, 0.5)
    }

    Rectangle {
        id: handle

        x: Math.max(0, Math.min(parent.width - width, indicator.control.visualPosition * parent.width - (width / 2)))

        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        width: height
        radius: Math.floor(width / 2)
        color: colorFactory.handleColor
        border.color: colorFactory.handleBorderColor

        Behavior on x {
            enabled: !indicator.control.pressed && LingmoUI.Units.shortDuration > 0
            SmoothedAnimation {
                duration: LingmoUI.Units.shortDuration
            }
        }

        Behavior on color {
            enabled: LingmoUI.Units.shortDuration > 0
            ColorAnimation {
                easing.type: Easing.InCubic
                duration: LingmoUI.Units.shortDuration
            }
        }
    }
}
