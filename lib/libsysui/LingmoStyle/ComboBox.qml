
/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */
pragma ComponentBehavior

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls.impl
import QtQuick.Templates as T
import LingmoUI

T.ComboBox {
    property color textColor: {
        if (LingmoTheme.dark) {
            if (!enabled) {
                return Qt.rgba(131 / 255, 131 / 255, 131 / 255, 1)
            }
            if (pressed) {
                return Qt.rgba(162 / 255, 162 / 255, 162 / 255, 1)
            }
            return Qt.rgba(1, 1, 1, 1)
        } else {
            if (!enabled) {
                return Qt.rgba(160 / 255, 160 / 255, 160 / 255, 1)
            }
            if (pressed) {
                return Qt.rgba(96 / 255, 96 / 255, 96 / 255, 1)
            }
            return Qt.rgba(0, 0, 0, 1)
        }
    }

    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(
                        implicitBackgroundHeight + topInset + bottomInset,
                        implicitContentHeight + topPadding + bottomPadding,
                        implicitIndicatorHeight + topPadding + bottomPadding)

    rightPadding: 35
    verticalPadding: 0
    horizontalPadding: 12

    delegate: LingmoMenuItem {
        required property var model
        required property int index
        text: model[control.textRole]
        font.weight: control.currentIndex === index ? Font.DemiBold : Font.Normal
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
    }

    indicator: LingmoIcon {
        id: icon
        iconSource: LingmoIcons.ChevronDown
        iconSize: 15
        iconColor: control.textColor
        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: parent.verticalCenter
        }
    }

    contentItem: T.TextField {
        width: control.width
        height: control.height
        text: control.editable ? control.editText : control.displayText

        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator

        verticalAlignment: Text.AlignVCenter

        property bool disabled: !control.editable
        property int iconSource: 0
        property color normalColor: LingmoTheme.dark ? Qt.rgba(
                                                           255 / 255,
                                                           255 / 255,
                                                           255 / 255,
                                                           1) : Qt.rgba(27 / 255, 27
                                                                        / 255, 27 / 255, 1)
        property color disableColor: LingmoTheme.dark ? Qt.rgba(
                                                            131 / 255,
                                                            131 / 255,
                                                            131 / 255,
                                                            1) : Qt.rgba(160 / 255, 160
                                                                         / 255, 160 / 255, 1)
        property color placeholderNormalColor: LingmoTheme.dark ? Qt.rgba(
                                                                      210 / 255,
                                                                      210 / 255,
                                                                      210 / 255,
                                                                      1) : Qt.rgba(96 / 255, 96 / 255, 96 / 255, 1)
        property color placeholderFocusColor: LingmoTheme.dark ? Qt.rgba(
                                                                     152 / 255,
                                                                     152 / 255,
                                                                     152 / 255,
                                                                     1) : Qt.rgba(141 / 255, 141 / 255, 141 / 255, 1)
        property color placeholderDisableColor: LingmoTheme.dark ? Qt.rgba(
                                                                       131 / 255,
                                                                       131 / 255,
                                                                       131 / 255,
                                                                       1) : Qt.rgba(160 / 255, 160 / 255, 160 / 255, 1)
        id: text_field
        enabled: !disabled
        color: {
            if (!enabled) {
                return disableColor
            }
            return normalColor
        }
        font: LingmoTextStyle.Body
        renderType: LingmoTheme.nativeText ? Text.NativeRendering : Text.QtRendering
        selectionColor: LingmoTools.withOpacity(LingmoTheme.primaryColor, 0.5)
        selectedTextColor: color
        placeholderTextColor: {
            if (!enabled) {
                return placeholderDisableColor
            }
            if (focus) {
                return placeholderFocusColor
            }
            return placeholderNormalColor
        }
        selectByMouse: text_field.selectTextByMouse
    }

    background: LingmoControlBackground {
        implicitWidth: 120
        implicitHeight: 32
    }

    popup: T.Popup {
        width: control.width
        height: Math.min(contentItem.implicitHeight,
                         control.Window.height - topMargin - bottomMargin)
        topMargin: 8
        bottomMargin: 8

        contentItem: ListView {
            implicitHeight: contentHeight
            model: control.delegateModel
            interactive: Window.window ? contentHeight + control.topPadding
                                         + control.bottomPadding > Window.window.height : false
            clip: true
            currentIndex: control.currentIndex
            T.ScrollBar.vertical: LingmoScrollBar {}
        }

        background: Rectangle {
            implicitWidth: 150
            implicitHeight: 36
            color: LingmoTheme.dark ? Qt.rgba(
                                          45 / 255, 45 / 255, 45 / 255,
                                          1) : Qt.rgba(252 / 255, 252 / 255, 252 / 255, 1)
            border.color: LingmoTheme.dark ? Qt.rgba(
                                                 26 / 255, 26 / 255, 26 / 255,
                                                 1) : Qt.rgba(191 / 255, 191 / 255, 191 / 255, 1)
            border.width: 1
            radius: LingmoUnits.windowRadius + 1
            LingmoShadow {
                radius: LingmoUnits.windowRadius
            }
        }
        T.Overlay.modal: Rectangle {
            color: LingmoTools.withOpacity(control.palette.shadow, 0.5)
            radius: window.windowVisibility === Window.Maximized ? 0 : LingmoUnits.windowRadius
        }
        T.Overlay.modeless: Rectangle {
            color: LingmoTools.withOpacity(control.palette.shadow, 0.12)
            radius: window.windowVisibility === Window.Maximized ? 0 : LingmoUnits.windowRadius
        }
    }
}
