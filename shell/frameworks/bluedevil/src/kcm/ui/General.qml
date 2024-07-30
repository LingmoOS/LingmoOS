/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs

import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCMUtils
import org.kde.bluezqt as BluezQt
import org.kde.bluedevil.kcm

KCMUtils.SimpleKCM {
    id: root

    title: i18n("Settings")

    // FIXME: Manager.adapters property does not have any NOTIFY hook.
    // Somehow even the name won't update in the ComboBox
    readonly property BluezQt.Adapter adapter: BluezQt.Manager.adapters[box.currentIndex] ?? null

    LingmoUI.FormLayout {
        id: form

        QQC2.ComboBox {
            id: box
            LingmoUI.FormData.label: i18n("Device:")
            model: BluezQt.Manager.adapters
            textRole: "name"
            visible: count > 1
        }

        QQC2.TextField {
            text: root.adapter.name
            LingmoUI.FormData.label: i18n("Name:")
            onEditingFinished: root.adapter.name = text
        }

        QQC2.Label {
            text: root.adapter.address
            LingmoUI.FormData.label: i18n("Address:")
        }

        QQC2.CheckBox {
            LingmoUI.FormData.label: i18n("Enabled:")
            checked: root.adapter.powered
            onToggled: root.adapter.powered = checked
        }

        QQC2.CheckBox {
            LingmoUI.FormData.label: i18n("Visible:")
            checked: root.adapter.discoverable
            onToggled: root.adapter.discoverable = checked
        }

        LingmoUI.Separator {
            LingmoUI.FormData.isSection: true
        }

        QQC2.ButtonGroup {
            id: loginStateRadioGroup
        }

        QQC2.RadioButton {
            LingmoUI.FormData.label: i18n("On login:")
            text: i18n("Enable Bluetooth")
            QQC2.ButtonGroup.group: loginStateRadioGroup
            checked: root.KCMUtils.ConfigModule.bluetoothStatusAtLogin === "enable"
            onToggled: {
                if (enabled) {
                    root.KCMUtils.ConfigModule.bluetoothStatusAtLogin = "enable";
                }
            }
        }
        QQC2.RadioButton {
            text: i18n("Disable Bluetooth")
            QQC2.ButtonGroup.group: loginStateRadioGroup
            checked: root.KCMUtils.ConfigModule.bluetoothStatusAtLogin === "disable"
            onToggled: {
                if (enabled) {
                    root.KCMUtils.ConfigModule.bluetoothStatusAtLogin = "disable"
                }
            }
        }
        QQC2.RadioButton {
            text: i18n("Restore previous status")
            QQC2.ButtonGroup.group: loginStateRadioGroup
            checked: root.KCMUtils.ConfigModule.bluetoothStatusAtLogin === "remember"
            onToggled: {
                if (enabled) {
                    root.KCMUtils.ConfigModule.bluetoothStatusAtLogin = "remember"
                }
            }
        }

        LingmoUI.Separator {
            LingmoUI.FormData.isSection: true
        }

        QQC2.ButtonGroup {
            id: receivingFilesRadioGroup
        }

        QQC2.RadioButton {
            LingmoUI.FormData.label: i18n("When receiving files:")
            checked: FileReceiverSettings.autoAccept === 0
            text: i18n("Ask for confirmation")
            QQC2.ButtonGroup.group: receivingFilesRadioGroup
            onClicked: {
                FileReceiverSettings.autoAccept = 0
                FileReceiverSettings.save()
            }
        }

        QQC2.RadioButton {
            text: i18n("Accept for trusted devices")
            checked: FileReceiverSettings.autoAccept === 1
            QQC2.ButtonGroup.group: receivingFilesRadioGroup
            onClicked: {
                FileReceiverSettings.autoAccept = 1
                FileReceiverSettings.save()
            }
        }

        QQC2.RadioButton {
            text: i18n("Always accept")
            QQC2.ButtonGroup.group: receivingFilesRadioGroup
            checked: FileReceiverSettings.autoAccept === 2
            onClicked: {
                FileReceiverSettings.autoAccept = 2
                FileReceiverSettings.save()
            }
        }

        Row {
            LingmoUI.FormData.label: i18n("Save files in:")
            QQC2.TextField {
                function urlToFilePath(url: url): string {
                    const urlString = url.toString();
                    let encodedFilePath;
                    if (urlString.startsWith("file:///")) {
                        const start = urlString.charAt(9) === ':' ? 8 : 7;
                        encodedFilePath = urlString.substring(start);
                    } else {
                        encodedFilePath = urlString;
                    }
                    return decodeURIComponent(encodedFilePath);
                }

                text: urlToFilePath(FileReceiverSettings.saveUrl)

                onEditingFinished: {
                    FileReceiverSettings.saveUrl = Qt.resolvedUrl(text)
                    FileReceiverSettings.save()
                }
            }

            QQC2.Button {
                icon.name: "document-open-folder-symbolic"
                display: QQC2.AbstractButton.IconOnly
                text: i18n("Select folder")
                onClicked: folderDialogLoader.active = true
            }

            Loader {
                id: folderDialogLoader

                active: false

                sourceComponent: FolderDialog {
                    id: startupFileDialog
                    title: i18n("Select folder")

                    currentFolder: FileReceiverSettings.saveUrl

                    onAccepted: {
                        FileReceiverSettings.saveUrl = selectedFolder
                        folderDialogLoader.active = false
                        FileReceiverSettings.save()
                    }

                    onRejected: folderDialogLoader.active = false

                    Component.onCompleted: open()
                }
            }
        }
    }
}
