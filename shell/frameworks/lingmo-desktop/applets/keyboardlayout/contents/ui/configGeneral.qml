/*
    SPDX-FileCopyrightText: 2021 Andrey Butirsky <butirsky@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.workspace.keyboardlayout 1.0
import org.kde.lingmo.workspace.components 2.0 as WorkspaceComponents
import org.kde.lingmo.private.kcm_keyboard as KCMKeyboard
import org.kde.kcmutils

SimpleKCM {
    id: root

    property int cfg_displayStyle

    readonly property string layoutShortName: keyboardLayout.layoutsList.length ? keyboardLayout.layoutsList[keyboardLayout.layout].shortName
                                                                            : ""
    readonly property string displayName: keyboardLayout.layoutsList.length ? keyboardLayout.layoutsList[keyboardLayout.layout].displayName
                                                                            : ""
    KeyboardLayout { id: keyboardLayout }

    LingmoUI.FormLayout {
        RadioButton {
            id: showLabel
            LingmoUI.FormData.label: i18n("Display style:")
            text: root.displayName.length > 0 ? root.displayName: root.layoutShortName
            checked: cfg_displayStyle === 0
            onToggled: cfg_displayStyle = 0;
        }

        RadioButton {
            id: showFlag
            checked: cfg_displayStyle === 1
            onToggled: cfg_displayStyle = 1;
            contentItem: Item {
                implicitWidth: childrenRect.width + showFlag.indicator.width
                implicitHeight: childrenRect.height

                // Deliberately using this instead of Image to preserve visual fidelity
                // with what the widget will show
                LingmoUI.Icon {
                    id: flagImage
                    width: LingmoUI.Units.iconSizes.medium
                    height: LingmoUI.Units.iconSizes.medium
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    source: KCMKeyboard.Flags.getIcon(root.layoutShortName)
                    visible: valid
                }
                RowLayout {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: LingmoUI.Units.smallSpacing
                    width: visible ? implicitWidth : 0
                    height: visible ? implicitHeight : 0
                    visible: !flagImage.visible

                    LingmoUI.Icon {
                        implicitWidth: LingmoUI.Units.iconSizes.smallMedium
                        implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                        source: "emblem-warning"
                    }
                    Label {
                        text: i18nc("@info:placeholder Make this translation as short as possible", "No flag available")
                    }
                }
            }
        }

        RadioButton {
            id: showLabelOverFlag
            checked: cfg_displayStyle === 2
            onToggled: cfg_displayStyle = 2;
            contentItem: Item {
                implicitWidth: childrenRect.width + showLabelOverFlag.indicator.width
                implicitHeight: childrenRect.height

                // Deliberately using this instead of Image to preserve visual fidelity
                // with what the widget will show
                LingmoUI.Icon {
                    width: LingmoUI.Units.iconSizes.medium
                    height: LingmoUI.Units.iconSizes.medium
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    source: KCMKeyboard.Flags.getIcon(root.layoutShortName)
                    visible: valid

                    WorkspaceComponents.BadgeOverlay {
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right

                        text: showLabel.text
                        icon: parent
                    }
                }
            }
        }

        LingmoUI.Separator {
            LingmoUI.FormData.isSection: true
        }

        Button {
            LingmoUI.FormData.label: i18n("Layouts:")
            text: i18n("Configure…")
            icon.name: "configure"
            onClicked: KCMLauncher.openSystemSettings("kcm_keyboard", "--tab=layouts")
        }

        Component.onCompleted: {
            // hide Keyboard Shortcuts tab
            var appletConfiguration = app
            while (appletConfiguration.parent) {
                appletConfiguration = appletConfiguration.parent
            }
            if (appletConfiguration && typeof appletConfiguration.globalConfigModel !== "undefined") {
                appletConfiguration.globalConfigModel.removeCategoryAt(0)
            }
        }
    }
}
