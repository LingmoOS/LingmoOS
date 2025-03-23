// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private 1.0 as P

T.ToolButton {
    id: control
    property D.Palette textColor: {
        if (D.DTK.hasAnimation)
            return checked ? DS.Style.highlightedButton.text : (highlighted ? DS.Style.highlightedButton.text : DS.Style.button.text)

        return checked ? DS.Style.checkedButton.text : (highlighted ? DS.Style.highlightedButton.text : DS.Style.button.text)
    }

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

    states: [
        State {
            name: "normal"
            when: !control.hovered && !control.checked && D.DTK.hasAnimation
            PropertyChanges {
                target: background
                scale : 0.9
            }

        },
        State {
            name: "hovered"
            when: control.hovered && !control.checked && D.DTK.hasAnimation
            PropertyChanges {
                target: contentItem
                scale : 1.1
            }
            PropertyChanges {
                target: background
                scale : 1.0
            }

        },
        State {
            name: "checked"
            when: control.checked && D.DTK.hasAnimation
            PropertyChanges {
                target: contentItem
                scale : 1.0
            }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "scale"; easing.type: Easing.InOutQuad }
    }

    background: P.ButtonPanel {
        id: buttonPanel
        implicitWidth: DS.Style.toolButton.width
        implicitHeight: DS.Style.toolButton.height
        button: control
        outsideBorderColor: null

        Binding on color1 {
            when: D.DTK.hasAnimation
            value: D.Palette {
                normal {
                    common: Qt.rgba(0, 0, 0, 0.1)
                }
            }
        }
        Binding on color2 {
            when: D.DTK.hasAnimation
            value: buttonPanel.color1
        }
        Binding on visible {
            when: D.DTK.hasAnimation
            value: control.hovered && !control.checked
        }
    }
}
