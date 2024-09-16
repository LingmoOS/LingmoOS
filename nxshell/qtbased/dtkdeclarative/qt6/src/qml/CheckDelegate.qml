// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.CheckDelegate {
    id: control
    property Component content
    property D.Palette backgroundColor: DS.Style.itemDelegate.checkBackgroundColor

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    baselineOffset: contentItem.y + contentItem.baselineOffset
    padding: DS.Style.control.padding
    spacing: DS.Style.control.spacing
    icon {
        width: DS.Style.itemDelegate.iconSize
        height: DS.Style.itemDelegate.iconSize
    }
    checkable: true
    indicator: Loader {
        x: control.mirrored ? control.leftPadding : control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        active: control.checked

        sourceComponent: D.DciIcon {
            palette: control.D.DTK.makeIconPalette(control.palette)
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            name: "menu_select"
            sourceSize: Qt.size(DS.Style.itemDelegate.checkIndicatorIconSize, DS.Style.itemDelegate.checkIndicatorIconSize)
            fallbackToQIcon: false
        }
    }

    contentItem: RowLayout {
        D.IconLabel {
            spacing: control.spacing
            mirrored: control.mirrored
            display: control.display
            alignment: control.display === D.IconLabel.IconOnly || control.display === D.IconLabel.TextUnderIcon
                       ? Qt.AlignCenter : Qt.AlignLeft | Qt.AlignVCenter
            text: control.text
            font: control.font
            color: control.palette.windowText
            icon: D.DTK.makeIcon(control.icon, control.D.DciIcon)
            Layout.fillWidth: !control.content
        }
        Loader {
            active: control.content
            sourceComponent: control.content
            Layout.fillWidth: true
        }
    }

    background: Item {
        implicitWidth: DS.Style.itemDelegate.width
        implicitHeight: DS.Style.itemDelegate.height
        Rectangle {
            anchors.fill: parent
            visible: !checked
            color: control.D.ColorSelector.backgroundColor
            radius: DS.Style.control.radius
        }
        Rectangle {
            anchors.fill: parent
            visible: control.checked
            color: DS.Style.itemDelegate.checkedColor
            radius: DS.Style.control.radius
        }
    }
}
