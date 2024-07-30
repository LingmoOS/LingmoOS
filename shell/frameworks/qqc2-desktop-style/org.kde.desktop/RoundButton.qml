/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.RoundButton {
    id: controlRoot

    LingmoUI.Theme.colorSet: !flat && (controlRoot.activeFocus || controlRoot.highlighted) ? LingmoUI.Theme.Selection : LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: flat && !down && !checked

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    baselineOffset: contentItem.y + contentItem.baselineOffset

    transform: Translate {
        x: controlRoot.down || controlRoot.checked ? 1 : 0
        y: controlRoot.down || controlRoot.checked ? 1 : 0
    }

    icon.width: LingmoUI.Units.iconSizes.smallMedium
    icon.height: LingmoUI.Units.iconSizes.smallMedium

    padding: 6 // Matches the button margins of Ocean, Fusion, Oxygen and QCommonStyle
    spacing: 4 // Matches the default/hardcoded icon+label spacing of Qt Widgets

    contentItem: GridLayout {
        rowSpacing: controlRoot.spacing
        columnSpacing: controlRoot.spacing
        flow: iconContent.visible && labelContent.visible && controlRoot.display === T.AbstractButton.TextUnderIcon ? GridLayout.TopToBottom : GridLayout.LeftToRight
        LingmoUI.Icon {
            id: iconContent
            Layout.alignment: {
                if (iconContent.visible && labelContent.visible) {
                    if (controlRoot.display === T.AbstractButton.TextBesideIcon) {
                        return Qt.AlignRight | Qt.AlignVCenter
                    } else if (controlRoot.display === T.AbstractButton.TextUnderIcon) {
                        return Qt.AlignHCenter | Qt.AlignBottom
                    }
                }
                return Qt.AlignCenter
            }
            color: controlRoot.icon.color // defaults to Qt::transparent
            implicitWidth: controlRoot.icon.width
            implicitHeight: controlRoot.icon.height
            visible: source.toString().length > 0 && controlRoot.display !== T.AbstractButton.TextOnly
            source: controlRoot.icon ? (controlRoot.icon.name || controlRoot.icon.source) : ""
        }
        Label {
            id: labelContent
            Layout.alignment: {
                if (iconContent.visible && labelContent.visible) {
                    if (controlRoot.display === T.AbstractButton.TextBesideIcon) {
                        return Qt.AlignLeft | Qt.AlignVCenter
                    } else if (controlRoot.display === T.AbstractButton.TextUnderIcon) {
                        return Qt.AlignHCenter | Qt.AlignTop
                    }
                }
                return Qt.AlignCenter
            }
            text: controlRoot.text
            visible: text.length > 0 && controlRoot.display !== T.AbstractButton.IconOnly
        }
    }
    background: Rectangle {
        property color borderColor: LingmoUI.ColorUtils.linearInterpolation(controlRoot.palette.button, controlRoot.palette.buttonText, LingmoUI.Theme.frameContrast)

        visible: !controlRoot.flat || controlRoot.hovered || controlRoot.activeFocus || controlRoot.highlighted || controlRoot.checked || controlRoot.down

        implicitWidth: LingmoUI.Units.gridUnit + 6 + 6
        implicitHeight: LingmoUI.Units.gridUnit + 6 + 6
        radius: controlRoot.radius
        color: {
            if (controlRoot.checked || controlRoot.down) {
                return Qt.tint(LingmoUI.Theme.textColor, Qt.alpha(borderColor, 0.8))
            } else if (controlRoot.flat) {
                return Qt.alpha(LingmoUI.Theme.backgroundColor, 0)
            } else {
                return LingmoUI.Theme.backgroundColor
            }
        }

        border.color: controlRoot.flat ? LingmoUI.Theme.highlightColor : borderColor
        border.width: controlRoot.flat && !(controlRoot.hovered || controlRoot.activeFocus || controlRoot.highlighted) ? 0 : 1

        Rectangle {
            id: gradientRect
            radius: controlRoot.radius
            anchors.fill: parent
            visible: !controlRoot.flat
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.13) }
                GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.03) }
            }
        }
        Rectangle {
            id: shadowRect
            z: -1
            radius: controlRoot.radius
            visible: !controlRoot.down && !controlRoot.checked && !controlRoot.flat
            anchors {
                topMargin: 1
                bottomMargin: -1
                fill: parent
            }
            color: Qt.rgba(0, 0, 0, 0.15)
        }
    }
}
