// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.CheckBox {
    id: control

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    baselineOffset: contentItem.y + contentItem.baselineOffset
    icon {
        width: DS.Style.checkBox.iconSize
        height: DS.Style.checkBox.iconSize
    }
    checkable: true
    font: D.DTK.fontManager.t8
    padding: DS.Style.checkBox.padding
    spacing: DS.Style.control.spacing
    opacity: D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1
    D.DciIcon.mode: D.ColorSelector.controlState
    D.DciIcon.theme: D.ColorSelector.controlTheme
    indicator: Item {
        x: control.text ? (!control.mirrored ? control.leftPadding : control.width - width - control.rightPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        implicitWidth: DS.Style.checkBox.indicatorWidth
        implicitHeight: DS.Style.checkBox.indicatorHeight
        D.DciIcon {
            anchors.centerIn: parent
            palette: control.D.DTK.makeIconPalette(control.palette)
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            name: control.checkState === Qt.Checked ? "checkbox_checked" : control.checkState === Qt.Unchecked ? "checkbox_unchecked" : "checkbox_mix"
            sourceSize: Qt.size(control.icon.width, control.icon.height)
            fallbackToQIcon: false
        }

        Loader {
            active: control.activeFocus
            anchors.centerIn: parent
            sourceComponent: D.DciIcon {
                palette: control.D.DTK.makeIconPalette(control.palette)
                mode: control.D.ColorSelector.controlState
                theme: control.D.ColorSelector.controlTheme
                name: control.checkState === Qt.Checked ? "checkbox_focus_checked" : "checkbox_focus"
                sourceSize: Qt.size(control.icon.width, control.icon.height)
                fallbackToQIcon: false
            }
        }
    }

    contentItem: Text {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0
        text: control.text
        font: control.font
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        color: control.palette.windowText
    }

    background: null
}
