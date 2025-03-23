// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.MenuItem {
    id: control

    property bool useIndicatorPadding: menu && menu.existsChecked || false
    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    baselineOffset: contentItem.y + contentItem.baselineOffset
    padding: DS.Style.control.padding
    spacing: DS.Style.control.spacing
    opacity: D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1
    icon {
        height: DS.Style.menu.item.iconSize.height
        width: DS.Style.menu.item.iconSize.height
    }

    property D.Palette textColor: control.highlighted ? DS.Style.checkedButton.text
                                                      : DS.Style.menu.itemText
    property D.Palette subMenuBackgroundColor: DS.Style.menu.subMenuOpenedBackground

    palette.windowText: D.ColorSelector.textColor
    D.DciIcon.mode: D.ColorSelector.controlState
    D.DciIcon.theme: D.ColorSelector.controlTheme
    D.DciIcon.palette: D.DTK.makeIconPalette(palette)
    contentItem: D.IconLabel {
        readonly property real arrowPadding: control.subMenu && control.arrow ? control.arrow.width + control.spacing : 0
        readonly property real indicatorPadding: control.useIndicatorPadding && control.indicator ? control.indicator.width + control.spacing : 0

        leftPadding: (!control.mirrored ? indicatorPadding : arrowPadding) + DS.Style.menu.item.contentPadding
        rightPadding: (control.mirrored ? indicatorPadding : arrowPadding) + DS.Style.menu.item.contentPadding
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        alignment: Qt.AlignLeft
        text: control.text
        font: control.font
        color: control.palette.windowText
        Behavior on color {
            ColorAnimation { duration: control.hovered ? 100 : 0; }
        }
        icon: D.DTK.makeIcon(control.icon, control.D.DciIcon)
    }

    indicator: Loader {
        width: DS.Style.menu.item.iconSize.width
        height: DS.Style.menu.item.iconSize.height
        active: control.checked
        anchors {
            left: control.left
            leftMargin: control.mirrored ? control.width - width - DS.Style.menu.item.indicatorMargin
                                         : DS.Style.menu.item.indicatorMargin
            verticalCenter: parent.verticalCenter
        }

        sourceComponent: D.DciIcon {
            sourceSize: Qt.size(DS.Style.menu.item.iconSize.width,
                                DS.Style.menu.item.iconSize.height)
            name: "menu_select"
            palette: control.D.DciIcon.palette
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            fallbackToQIcon: false
        }
    }

    arrow: Loader {
        width: DS.Style.menu.item.iconSize.width
        height: DS.Style.menu.item.iconSize.height
        active: control.subMenu
        anchors {
            right: parent.right
            rightMargin: control.mirrored ? control.width - width - control.rightPadding
                                          : control.rightPadding
            verticalCenter: parent.verticalCenter
        }

        sourceComponent: D.DciIcon {
            sourceSize: Qt.size(DS.Style.menu.item.iconSize.width,
                                DS.Style.menu.item.iconSize.height)
            mirror: control.mirrored
            name: "menu_arrow"
            palette: control.D.DciIcon.palette
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            fallbackToQIcon: false
        }
    }

    background: Item {
        implicitWidth: DS.Style.menu.item.width
        implicitHeight: DS.Style.menu.item.height
        Loader {
            anchors.fill: parent
            active: !control.hovered && control.subMenu && control.subMenu.opened
            sourceComponent: Rectangle {
                color: control.D.ColorSelector.subMenuBackgroundColor
                radius: DS.Style.control.radius
            }
        }
    }
}
