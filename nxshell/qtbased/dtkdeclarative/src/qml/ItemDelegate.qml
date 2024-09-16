// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates 2.4 as T
import QtQuick.Layouts 1.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.ItemDelegate {
    id: control
    property bool indicatorVisible
    property bool backgroundVisible: true
    property bool cascadeSelected
    property bool contentFlow
    property Component content
    property D.Palette checkedTextColor: DS.Style.checkedButton.text
    property int corners: D.RoundRectangle.TopLeftCorner | D.RoundRectangle.TopRightCorner | D.RoundRectangle.BottomLeftCorner | D.RoundRectangle.BottomRightCorner
    function getCornersForBackground(index, count) {
        if (count <= 1)
            return D.RoundRectangle.TopLeftCorner | D.RoundRectangle.TopRightCorner | D.RoundRectangle.BottomLeftCorner | D.RoundRectangle.BottomRightCorner
        if (index === 0) {
            return D.RoundRectangle.TopLeftCorner | D.RoundRectangle.TopRightCorner
        } else if (index === count - 1) {
            return D.RoundRectangle.BottomLeftCorner | D.RoundRectangle.BottomRightCorner
        }
        return 0
    }

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    baselineOffset: contentItem.y + contentItem.baselineOffset
    padding: DS.Style.control.padding
    spacing: DS.Style.control.spacing
    checkable: true
    autoExclusive: true
    palette.windowText: checked && !control.cascadeSelected ? D.ColorSelector.checkedTextColor : undefined

    D.DciIcon.mode: D.ColorSelector.controlState
    D.DciIcon.theme: D.ColorSelector.controlTheme
    D.DciIcon.palette: D.DTK.makeIconPalette(palette)
    icon {
        width: DS.Style.itemDelegate.iconSize
        height: DS.Style.itemDelegate.iconSize
    }
    font: D.DTK.fontManager.t9

    indicator: Loader {
        x: control.text ? (control.mirrored ? control.leftPadding : control.width - width - control.rightPadding)
                        : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        active: control.indicatorVisible && control.checked

        sourceComponent: D.DciIcon {
            palette: D.DTK.makeIconPalette(control.palette)
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            fallbackToQIcon: false
            name: "menu_select"
            sourceSize: Qt.size(DS.Style.itemDelegate.checkIndicatorIconSize, DS.Style.itemDelegate.checkIndicatorIconSize)
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
            Layout.fillWidth: !control.contentFlow
        }
        Loader {
            sourceComponent: control.content
            Layout.fillWidth: control.contentFlow
            Layout.alignment: control.contentFlow ? Qt.AlignLeft : Qt.AlignRight
        }
    }

    background: Item {
        implicitWidth: DS.Style.itemDelegate.width
        implicitHeight: DS.Style.itemDelegate.height

        Loader {
            anchors.fill: parent
            active: checked && !control.cascadeSelected
            sourceComponent: HighlightPanel {}
        }

        Loader {
            anchors.fill: parent
            active: checked && control.cascadeSelected
            sourceComponent: D.RoundRectangle {
                color: DS.Style.itemDelegate.cascadeColor
                radius: DS.Style.control.radius
                corners: control.corners
            }
        }

        Loader {
            anchors.fill: parent
            active: !checked && control.backgroundVisible
            sourceComponent: D.RoundRectangle {
                color: DS.Style.itemDelegate.normalColor
                radius: DS.Style.control.radius
                corners: control.corners
            }
        }
    }
}
