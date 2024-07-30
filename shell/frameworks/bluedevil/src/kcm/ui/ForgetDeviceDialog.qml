/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.bluezqt as BluezQt
import org.kde.lingmoui as LingmoUI

QtObject {
    id: root

    required property Item parent

    function open(device: BluezQt.Device): void {
        const dialog = dialogComponent.createObject(this, { device });
        dialog.open();
    }

    signal call(BluezQt.PendingCall call)

    readonly property Component __dialogComponent: Component {
        id: dialogComponent

        LingmoUI.PromptDialog {
            id: dialog

            required property BluezQt.Device device

            title: i18n("Forget this Device?")
            subtitle: i18n("Are you sure you want to forget \"%1\"?", device.name)

            parent: root.parent
            showCloseButton: false

            // Need to use fully custom actions because it's not possible to override
            // the text and icon of a single standardbutton, and if we use just a
            // custom action for that one, then it's in the wrong visual position
            // relative to the StandardButton-provided Cancel button
            standardButtons: LingmoUI.Dialog.NoButton
            customFooterActions: [
                LingmoUI.Action {
                    text: i18nc("@action:button", "Forget Device")
                    icon.name: "edit-delete-remove-symbolic"
                    onTriggered: {
                        dialog.accept();
                    }
                },
                LingmoUI.Action {
                    text: i18nc("@action:button", "Cancel")
                    icon.name: "dialog-cancel-symbolic"
                    onTriggered: {
                        dialog.reject();
                    }
                    shortcut: StandardKey.Cancel
                }
            ]

            onAccepted: root.call(device.adapter.removeDevice(device))

            onClosed: destroy()

            contentData: [
                Connections {
                    target: dialog.device
                    function onDeviceRemoved(device: BluezQt.Device): void {
                        dialog.reject();
                    }
                },
                Connections {
                    target: dialog.device.adapter
                    function onAdapterRemoved(adapter: BluezQt.Adapter): void {
                        dialog.reject();
                    }
                    function onPoweredChanged(powered: bool): void {
                        if (!powered) {
                            dialog.reject();
                        }
                    }
                }
            ]
        }
    }
}
