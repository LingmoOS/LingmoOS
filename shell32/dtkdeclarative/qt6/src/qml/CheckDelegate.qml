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
    property string indicatorIcon: control.checkState === Qt.Unchecked ? "item_unchecked" : "item_checked"
    property bool indicatorVisible: control.checked

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
        active: indicatorVisible

        sourceComponent: D.DciIcon {
            palette: control.D.DTK.makeIconPalette(control.palette)
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            name: indicatorIcon
            sourceSize: Qt.size(DS.Style.itemDelegate.checkIndicatorIconSize, DS.Style.itemDelegate.checkIndicatorIconSize)
            fallbackToQIcon: false
            onNameChanged: {
                play(D.DTK.NormalState);
            }
            Component.onCompleted: {
                if (indicatorVisible)
                    play(D.DTK.NormalState);
            }
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

    background: Control {
        implicitWidth: DS.Style.itemDelegate.width
        implicitHeight: DS.Style.itemDelegate.height
        Rectangle {
            anchors.fill: parent
            visible: !control.checked && !D.DTK.hasAnimation && control.hovered
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

    onHoveredChanged: {
        if (checked || !ListView.view)
            return

        if (ListView.view)
            ListView.view.setHoverItem(control.hovered ? control : null)
    }
}
