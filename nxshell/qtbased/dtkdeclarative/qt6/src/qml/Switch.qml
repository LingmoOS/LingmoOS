// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.Switch {
    id: control

    property D.Palette backgroundColor: DS.Style.switchButton.background
    property D.Palette handleColor: DS.Style.switchButton.handle

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)

    baselineOffset: contentItem.y + contentItem.baselineOffset
    topPadding: DS.Style.control.vPadding
    bottomPadding: DS.Style.control.vPadding
    spacing: DS.Style.control.spacing

    indicator: Rectangle {
        implicitWidth: DS.Style.switchButton.indicatorWidth
        implicitHeight: DS.Style.switchButton.indicatorHeight

        x: text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        radius: DS.Style.control.radius
        color: control.D.ColorSelector.backgroundColor
        opacity: control.D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1

        D.DciIcon {
            id: handle
            x: Math.max(0, Math.min(parent.width - width, control.visualPosition * parent.width - (width / 2)))
            y: (parent.height - height) / 2
            width: DS.Style.switchButton.handleWidth
            height: DS.Style.switchButton.handleHeight
            sourceSize: Qt.size(DS.Style.switchButton.handleWidth, DS.Style.switchButton.handleHeight)
            name: DS.Style.switchButton.iconName
            opacity: control.D.ColorSelector.controlState === D.DTK.DisabledState && control.checked ? 0.4 : 1
            palette {
                highlight: control.checked ? control.palette.highlight : control.D.ColorSelector.handleColor
                highlightForeground: control.palette.highlightedText
                foreground: control.palette.windowText
                background: control.palette.window
            }
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            fallbackToQIcon: false

            Behavior on x {
                enabled: !control.down
                SmoothedAnimation { velocity: 200 }
            }
        }
    }

    contentItem: Label {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0

        text: control.text
        verticalAlignment: Text.AlignVCenter
    }
}
