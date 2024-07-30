/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 * SPDX-FileCopyrightText: 2024 Shubham Arora <shubhamarora@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCMUtils

import org.kde.bluezqt as BluezQt

import "script.js" as Script

KCMUtils.SimpleKCM {
    id: root

    required property BluezQt.Device device

    title: device.name

    Connections {
        target: root.KCMUtils.ConfigModule
        function onNetworkAvailable(service: string, available: bool): void {
            switch (service) {
            case "dun":
                dunButton.visible = available && root.device.connected;
                break;
            case "nap":
                napButton.visible = available && root.device.connected;
                break;
            default:
                break;
            }
        }
    }

    Connections {
        target: root.device

        function onConnectedChanged(connected: bool): void {
            root.checkNetworkConnection();
        }
    }

    Component.onCompleted: {
        checkNetworkConnection();
    }

    function checkNetworkConnection(): void {
        KCMUtils.ConfigModule.checkNetworkConnection(device.uuids, device.address);
    }

    function makeCall(call: BluezQt.PendingCall): void {
        indicator.running = true
        call.finished.connect(call => {
            indicator.running = false
            if (call.error) {
                errorMessage.text = call.errorText
                errorMessage.visible = true
            }
        })
    }

    ForgetDeviceDialog {
        id: forgetDeviceDialog

        parent: root.QQC2.Overlay.overlay

        onCall: call => {
            root.makeCall(call);
        }
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: LingmoUI.InlineMessage {
        id: errorMessage
        position: LingmoUI.InlineMessage.Position.Header
        type: LingmoUI.MessageType.Error
        showCloseButton: true
    }

    ColumnLayout {
        spacing: LingmoUI.Units.smallSpacing

        LingmoUI.Icon {
            source: root.device.icon
            Layout.preferredWidth: LingmoUI.Units.iconSizes.enormous
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
        }

        LingmoUI.FormLayout {

            Row {
                spacing: LingmoUI.Units.smallSpacing

                QQC2.Button {
                    id: connectButton
                    enabled: !indicator.running && !root.device.blocked
                    text: root.device.connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: root.device.connected ? "network-disconnect-symbolic" : "network-connect-symbolic"

                    QQC2.ToolTip.text: i18n("Unblock the device first to be able to connect to it")
                    QQC2.ToolTip.visible: !enabled && hovered

                    onClicked: {
                        if (root.device.connected) {
                            root.makeCall(root.device.disconnectFromDevice())
                        } else {
                            root.makeCall(root.device.connectToDevice())
                        }
                    }
                }

                QQC2.BusyIndicator {
                    id: indicator
                    running: false
                    height: connectButton.height
                }
            }

            QQC2.Label {
                text: Script.deviceTypeToString(root.device)
                LingmoUI.FormData.label: i18n("Type:")
            }

            QQC2.Label {
                text: root.device.battery !== null ? i18n("%1%", root.device.battery.percentage) : ""
                visible: root.device.battery !== null
                LingmoUI.FormData.label: i18n("Battery:")
            }

            QQC2.Label {
                text: root.device.address
                LingmoUI.FormData.label: i18n("Address:")
            }

            QQC2.Label {
                text: root.device.adapter.name
                LingmoUI.FormData.label: i18n("Adapter:")
            }

            QQC2.TextField {
                text: root.device.name
                onTextEdited: root.device.name = text
                LingmoUI.FormData.label: i18n("Name:")
            }

            QQC2.CheckBox {
                text: i18n("Trusted")
                checked: root.device.trusted
                onClicked: root.device.trusted = !root.device.trusted
            }

            RowLayout {
                spacing: LingmoUI.Units.smallSpacing

                QQC2.CheckBox {
                    text: i18n("Blocked")
                    checked: root.device.blocked
                    onClicked: root.device.blocked = !root.device.blocked
                }

                LingmoUI.ContextualHelpButton {
                    toolTipText: i18n("Any incoming connections from a blocked device will be immediately rejected.")
                }
            }

            QQC2.Button {
                icon.name: "document-share-symbolic"
                text: i18n("Send File")
                visible: root.device.uuids.includes(BluezQt.Services.ObexObjectPush) && root.device.connected
                onClicked: root.KCMUtils.ConfigModule.runSendFile(root.device.ubi)
            }

            QQC2.Button {
                id: napButton
                icon.name: "network-wireless-bluetooth-symbolic"
                text: i18n("Setup NAP Network…")
                visible: false
                onClicked: root.KCMUtils.ConfigModule.setupNetworkConnection("nap", root.device.address, root.device.name)
            }

            QQC2.Button {
                id: dunButton
                icon.name: "network-wireless-bluetooth-symbolic"
                text: i18n("Setup DUN Network…")
                visible: false
                onClicked: root.KCMUtils.ConfigModule.setupNetworkConnection("dun", root.device.address, root.device.name)
            }

            LingmoUI.Separator {
                LingmoUI.FormData.isSection: true
            }

            QQC2.Button {
                action: ForgetDeviceAction {
                    dialog: forgetDeviceDialog
                    device: root.device
                }
                // override action's text with a shorter label
                text: i18nc("@action:button Forget the Bluetooth device", "Forget");
            }
        }
    }
}
