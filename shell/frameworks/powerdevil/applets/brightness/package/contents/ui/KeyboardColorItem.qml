/*
 * SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kcmutils // KCMLauncher
import org.kde.config as KConfig  // KAuthorized.authorizeControlModule
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.private.brightnesscontrolplugin

LingmoComponents3.ItemDelegate {
    id: root

    background.visible: highlighted
    highlighted: activeFocus
    hoverEnabled: false

    visible: keyboardColorControl.supported

    KeyboardColorControl {
        id: keyboardColorControl
    }

    contentItem: RowLayout {
        spacing: LingmoUI.Units.gridUnit

        LingmoUI.Icon {
            id: image
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
            Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
            source: "input-keyboard-color"
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: LingmoUI.Units.smallSpacing

            RowLayout {
                Layout.fillWidth: true
                spacing: LingmoUI.Units.smallSpacing

                LingmoComponents3.Label {
                    id: title
                    text: root.text
                    textFormat: Text.PlainText

                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Rectangle {
                    id: colorIndicator

                    width: Math.round(LingmoUI.Units.gridUnit * 1.25)
                    height: Math.round(LingmoUI.Units.gridUnit * 1.25)
                    radius: 180

                    color: keyboardColorControl.enabled ? LingmoUI.Theme.highlightColor : "transparent"
                    border.color: Qt.rgba(0, 0, 0, 0.15)
                }
            }

            RowLayout {
                LingmoComponents3.Switch {
                    id: syncAccentSwitch
                    checked: keyboardColorControl.enabled
                    text: i18n("Follow accent color")

                    Layout.fillWidth: true

                    KeyNavigation.up: root.KeyNavigation.up
                    KeyNavigation.tab: root.KeyNavigation.tab
                    KeyNavigation.right: root.KeyNavigation.right
                    KeyNavigation.backtab: root.KeyNavigation.backtab

                    Keys.onPressed: (event) => {
                        if (event.key == Qt.Key_Space || event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                            toggle();
                        }
                    }
                    onToggled: {
                        keyboardColorControl.enabled = checked;
                    }
                }
            }
        }
    }
}
