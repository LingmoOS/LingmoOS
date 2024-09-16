// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.RadioButton {
    id: control

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    baselineOffset: contentItem.y + contentItem.baselineOffset
    padding: DS.Style.control.padding
    spacing: DS.Style.radioButton.spacing
    topPadding: DS.Style.radioButton.topPadding
    bottomPadding: DS.Style.radioButton.bottomPadding
    opacity: D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1
    D.DciIcon.mode: D.ColorSelector.controlState
    D.DciIcon.theme: D.ColorSelector.controlTheme
    icon {
        width: DS.Style.radioButton.iconSize
        height: DS.Style.radioButton.iconSize
    }
    indicator: Item {
        x: text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        implicitWidth:  DS.Style.radioButton.indicatorSize
        implicitHeight: implicitWidth

        D.DciIcon {
            anchors.centerIn: parent
            palette: control.D.DTK.makeIconPalette(control.palette)
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            name: control.checked ? "radio_checked" : "radio_unchecked"
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
                name: "radio_focus"
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
}
