/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl
import LingmoUI

T.ToolButton {
    property bool disabled: !enabled
    property string contentDescription: ""
    property color normalColor: LingmoTheme.dark ? Qt.rgba(
                                                    62 / 255, 62 / 255,
                                                    62 / 255,
                                                    1) : Qt.rgba(254 / 255, 254 / 255, 254 / 255, 1)
    property color hoverColor: LingmoTheme.dark ? Qt.rgba(
                                                   68 / 255, 68 / 255,
                                                   68 / 255,
                                                   1) : Qt.rgba(246 / 255, 246 / 255, 246 / 255, 1)
    property color disableColor: LingmoTheme.dark ? Qt.rgba(
                                                     59 / 255, 59 / 255,
                                                     59 / 255,
                                                     1) : Qt.rgba(251 / 255, 251
                                                                  / 255, 251 / 255, 1)
    property color dividerColor: LingmoTheme.dark ? Qt.rgba(
                                                     80 / 255, 80 / 255,
                                                     80 / 255,
                                                     1) : Qt.rgba(233 / 255, 233
                                                                  / 255, 233 / 255, 1)
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
    Accessible.role: Accessible.Button
    Accessible.name: control.text
    Accessible.description: contentDescription
    Accessible.onPressAction: control.clicked()
    verticalPadding: 0
    horizontalPadding: 12
    font: LingmoTextStyle.Body
    focusPolicy: Qt.TabFocus

    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 6
    spacing: 8

    icon.width: 20
    icon.height: 20
    icon.color: Color.transparent(control.textColor, enabled ? 1.0 : 0.2)

    property bool useSystemFocusVisuals: true

    contentItem: IconLabel {
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display

        icon: control.icon
        text: control.text
        font: control.font
        color: control.textColor
    }

    background: LingmoControlBackground {
        implicitWidth: 30
        implicitHeight: 30
        radius: LingmoUnits.smallRadius
        color: {
            if (!enabled) {
                return disableColor
            }
            return hovered ? hoverColor : normalColor
        }
        shadow: !pressed && enabled
        LingmoFocusRectangle {
            visible: control.activeFocus
            radius: LingmoUnits.smallRadius
        }
    }
}
