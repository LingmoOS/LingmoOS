/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

import QtQuick
import QtQuick.Templates as T
import LingmoUI

T.DelayButton {
    property bool disabled: false
    property string contentDescription: ""

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

    QtObject {
        id: d
        property bool checked: (Number(rect_back.height) === Number(
                                    background.height)) && (progress === 1)
    }
    property color normalColor: {
        if (d.checked) {
            return LingmoTheme.primaryColor
        } else {
            return LingmoTheme.dark ? Qt.rgba(
                                       62 / 255, 62 / 255, 62 / 255,
                                       1) : Qt.rgba(254 / 255, 254 / 255, 254 / 255, 1)
        }
    }
    property color hoverColor: {
        if (d.checked) {
            return LingmoTheme.dark ? Qt.darker(normalColor,
                                             1.1) : Qt.lighter(normalColor, 1.1)
        } else {
            return LingmoTheme.dark ? Qt.rgba(
                                       68 / 255, 68 / 255, 68 / 255,
                                       1) : Qt.rgba(246 / 255, 246 / 255, 246 / 255, 1)
        }
    }
    property color disableColor: {
        if (d.checked) {
            return LingmoTheme.dark ? Qt.rgba(
                                       82 / 255, 82 / 255, 82 / 255,
                                       1) : Qt.rgba(199 / 255, 199 / 255, 199 / 255, 1)
        } else {
            return LingmoTheme.dark ? Qt.rgba(
                                       59 / 255, 59 / 255, 59 / 255,
                                       1) : Qt.rgba(244 / 255, 244 / 255, 244 / 255, 1)
        }
    }
    property color pressedColor: LingmoTheme.dark ? Qt.darker(normalColor,
                                                           1.2) : Qt.lighter(
                                                     normalColor, 1.2)

    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    verticalPadding: 0
    horizontalPadding: 12

    font: LingmoTextStyle.Body
    focusPolicy: Qt.TabFocus

    property bool useSystemFocusVisuals: true

    transition: Transition {
        NumberAnimation {
            duration: control.delay * (control.pressed ? 1.0 - control.progress : 0.3 * control.progress)
        }
    }

    contentItem: LingmoText {
        text: control.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: {
            if (d.checked) {
                if (LingmoTheme.dark) {
                    if (!enabled) {
                        return Qt.rgba(173 / 255, 173 / 255, 173 / 255, 1)
                    }
                    return Qt.rgba(0, 0, 0, 1)
                } else {
                    return Qt.rgba(1, 1, 1, 1)
                }
            } else {
                if (LingmoTheme.dark) {
                    if (!enabled) {
                        return Qt.rgba(131 / 255, 131 / 255, 131 / 255, 1)
                    }
                    if (!d.checked) {
                        if (pressed) {
                            return Qt.rgba(162 / 255, 162 / 255, 162 / 255, 1)
                        }
                    }
                    return Qt.rgba(1, 1, 1, 1)
                } else {
                    if (!enabled) {
                        return Qt.rgba(160 / 255, 160 / 255, 160 / 255, 1)
                    }
                    if (!d.checked) {
                        if (pressed) {
                            return Qt.rgba(96 / 255, 96 / 255, 96 / 255, 1)
                        }
                    }
                    return Qt.rgba(0, 0, 0, 1)
                }
            }
        }
    }

    background: LingmoControlBackground {
        implicitWidth: 30
        implicitHeight: 30
        radius: LingmoUnits.smallRadius
        border.color: LingmoTheme.dark ? Qt.rgba(
                                          48 / 255, 48 / 255, 48 / 255,
                                          1) : Qt.rgba(188 / 255, 188 / 255, 188 / 255, 1)
        border.width: d.checked ? 0 : 1
        color: {
            if (!enabled) {
                return disableColor
            }
            if (d.checked) {
                if (pressed) {
                    return pressedColor
                }
            }
            return hovered ? hoverColor : normalColor
        }
        LingmoClip {
            anchors.fill: parent
            radius: {return new Array(4).fill(LingmoUnits.smallRadius);}
            Rectangle {
                id: rect_back
                width: parent.width * control.progress
                height: control.progress === 1 ? background.height : 3
                visible: !d.checked
                color: LingmoTheme.primaryColor
                anchors.bottom: parent.bottom
                Behavior on height {
                    enabled: control.progress !== 0
                    SequentialAnimation {
                        PauseAnimation {
                            duration: LingmoTheme.animationEnabled ? 167 : 0
                        }
                        NumberAnimation {
                            duration: LingmoTheme.animationEnabled ? 167 : 0
                            from: 3
                            to: background.height
                        }
                    }
                }
            }
        }
        LingmoFocusRectangle {
            visible: control.activeFocus
            radius: 4
        }
    }
}
