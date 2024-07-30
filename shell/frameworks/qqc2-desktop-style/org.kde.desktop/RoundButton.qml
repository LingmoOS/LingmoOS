/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.RoundButton {
    id: controlRoot

    Kirigami.Theme.colorSet: !flat && (controlRoot.activeFocus || controlRoot.highlighted) ? Kirigami.Theme.Selection : Kirigami.Theme.Button
    Kirigami.Theme.inherit: flat && !down && !checked

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

    icon.width: Kirigami.Units.iconSizes.smallMedium
    icon.height: Kirigami.Units.iconSizes.smallMedium

    padding: 6 // Matches the button margins of Breeze, Fusion, Oxygen and QCommonStyle
    spacing: 4 // Matches the default/hardcoded icon+label spacing of Qt Widgets

    contentItem: GridLayout {
        rowSpacing: controlRoot.spacing
        columnSpacing: controlRoot.spacing
        flow: iconContent.visible && labelContent.visible && controlRoot.display === T.AbstractButton.TextUnderIcon ? GridLayout.TopToBottom : GridLayout.LeftToRight
        Kirigami.Icon {
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
        property color borderColor: Kirigami.ColorUtils.linearInterpolation(controlRoot.palette.button, controlRoot.palette.buttonText, Kirigami.Theme.frameContrast)

        visible: !controlRoot.flat || controlRoot.hovered || controlRoot.activeFocus || controlRoot.highlighted || controlRoot.checked || controlRoot.down

        implicitWidth: Kirigami.Units.gridUnit + 6 + 6
        implicitHeight: Kirigami.Units.gridUnit + 6 + 6
        radius: controlRoot.radius
        color: {
            if (controlRoot.checked || controlRoot.down) {
                return Qt.tint(Kirigami.Theme.textColor, Qt.alpha(borderColor, 0.8))
            } else if (controlRoot.flat) {
                return Qt.alpha(Kirigami.Theme.backgroundColor, 0)
            } else {
                return Kirigami.Theme.backgroundColor
            }
        }

        border.color: controlRoot.flat ? Kirigami.Theme.highlightColor : borderColor
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
