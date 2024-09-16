// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.ToolButton {
    id: control
    property D.Palette textColor: checked ? DS.Style.checkedButton.text : (highlighted ? DS.Style.highlightedButton.text : DS.Style.button.text)

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    topPadding: DS.Style.button.vPadding
    bottomPadding: DS.Style.button.vPadding
    leftPadding: DS.Style.button.hPadding
    rightPadding: DS.Style.button.hPadding
    spacing: DS.Style.control.spacing
    opacity: D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1
    D.DciIcon.mode: D.ColorSelector.controlState
    D.DciIcon.theme: D.ColorSelector.controlTheme
    D.DciIcon.palette: D.DTK.makeIconPalette(palette)
    palette.windowText: D.ColorSelector.textColor
    D.ColorSelector.family: D.Palette.CrystalColor
    display: D.IconLabel.TextUnderIcon
    font: icon.name ? D.DTK.fontManager.t10: undefined
    flat: true

    icon {
        width: DS.Style.toolButton.iconSize
        height: DS.Style.toolButton.iconSize
        color: D.ColorSelector.textColor
    }

    contentItem: Item {
        implicitWidth: content.implicitWidth + (indicator ? indicator.width : 0)
        implicitHeight: content.implicitHeight
        D.IconLabel {
            id: content
            height: parent.height
            width: parent.width - (indicator ? indicator.width : 0)
            spacing: control.spacing
            mirrored: control.mirrored
            display: control.display
            alignment: indicator ? Qt.AlignLeft | Qt.AlignVCenter : Qt.AlignCenter
            text: control.text
            font: control.font
            color: control.D.ColorSelector.textColor
            icon: D.DTK.makeIcon(control.icon, control.D.DciIcon)
        }
        function updateIndicatorAnchors()
        {
            if (!indicator)
                return

            indicator.anchors.verticalCenter = control.verticalCenter
            indicator.anchors.right = control.right
            indicator.anchors.rightMargin = DS.Style.toolButton.indicatorRightMargin
        }
        Component.onCompleted: {
            updateIndicatorAnchors()
            control.indicatorChanged.connect(updateIndicatorAnchors)
        }
    }

    background: ButtonPanel {
        implicitWidth: DS.Style.toolButton.width
        implicitHeight: DS.Style.toolButton.height
        button: control
        outsideBorderColor: null
    }
}
