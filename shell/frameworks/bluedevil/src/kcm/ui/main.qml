/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Tom Zander <tom@flowee.org>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 * SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 * SPDX-FileCopyrightText: 2024 Shubham Arora <shubhamarora@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI
import org.kde.lingmoui.delegates as KD
import org.kde.kcmutils as KCMUtils

import org.kde.bluezqt as BluezQt

import org.kde.lingmo.private.bluetooth

import "script.js" as Script

KCMUtils.ScrollViewKCM {
    id: root

    actions: [
        LingmoUI.Action {
            id: toggleBluetoothAction
            text: i18nc("@action: button as in, 'enable Bluetooth'", "Enabled")
            icon.name: "network-bluetooth-symbolic"
            checkable: true
            checked: BluezQt.Manager.bluetoothOperational
            visible: BluezQt.Manager.rfkill.state !== BluezQt.Rfkill.Unknown
            onToggled: source => {
                root.toggleBluetooth();
            }
            displayComponent: QQC2.Switch {
                action: toggleBluetoothAction
            }
        },
        LingmoUI.Action {
            text: i18n("Add New Device…")
            icon.name: "list-add-symbolic"
            onTriggered: root.KCMUtils.ConfigModule.runWizard()
            visible: BluezQt.Manager.bluetoothOperational
        },
        LingmoUI.Action {
            text: i18n("Configure…")
            icon.name: "configure-symbolic"
            onTriggered: root.KCMUtils.ConfigModule.push("General.qml")
            visible: BluezQt.Manager.bluetoothOperational
        }
    ]

    function makeCall(call: BluezQt.PendingCall): void {
        busyIndicator.running = true
        call.finished.connect(call => {
            busyIndicator.running = false
            if (call.error) {
                errorMessage.text = call.errorText
                errorMessage.visible = true
            }
        })
    }

    Connections {
        target: root.KCMUtils.ConfigModule

        function onErrorOccured(errorText: string): void {
            errorMessage.text = errorText
            errorMessage.visible = true
        }
    }

    ForgetDeviceDialog {
        id: forgetDeviceDialog

        parent: root.QQC2.Overlay.overlay

        onCall: call => {
            root.makeCall(call);
        }
    }

    implicitHeight: LingmoUI.Units.gridUnit * 28
    implicitWidth: LingmoUI.Units.gridUnit * 28

    function toggleBluetooth(): void {
        const enable = !BluezQt.Manager.bluetoothOperational;
        setBluetoothEnabled(enable);
    }

    function setBluetoothEnabled(enable: bool): void {
        BluezQt.Manager.bluetoothBlocked = !enable;

        BluezQt.Manager.adapters.forEach(adapter => {
            adapter.powered = enable;
        });
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: LingmoUI.InlineMessage {
        id: errorMessage
        position: LingmoUI.InlineMessage.Position.Header
        type: LingmoUI.MessageType.Error
        showCloseButton: true
    }

    view: ListView {
        id: list
        clip: true

        LingmoUI.PlaceholderMessage {
            id: noBluetoothMessage
            // We cannot use the adapter count here because that can be zero when
            // bluetooth is disabled even when there are physical devices
            visible: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown
            icon.name: "edit-none"
            text: i18n("No Bluetooth adapters found")
            explanation: i18n("Connect an external Bluetooth adapter")
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        LingmoUI.PlaceholderMessage {
            id: bluetoothDisabledMessage
            visible: BluezQt.Manager.operational && !BluezQt.Manager.bluetoothOperational && !noBluetoothMessage.visible
            icon.name: "network-bluetooth-inactive"
            text: i18n("Bluetooth is disabled")
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)
            anchors.centerIn: parent

            helpfulAction: LingmoUI.Action {
                icon.name: "network-bluetooth-symbolic"
                text: i18n("Enable")
                onTriggered: {
                    root.setBluetoothEnabled(true);
                }
            }
        }

        LingmoUI.PlaceholderMessage {
            visible: !noBluetoothMessage.visible && !bluetoothDisabledMessage.visible && list.count === 0
            icon.name: "network-bluetooth-activated"
            text: i18n("No devices paired")
            explanation: xi18nc("@info", "Click <interface>Add New Device…</interface> to pair some")
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        model: BluezQt.Manager.bluetoothOperational ? devicesModel : null

        QQC2.BusyIndicator {
            id: busyIndicator
            running: false
            anchors.centerIn: parent
        }


        DevicesProxyModel {
            id: devicesModel
            sourceModel: BluezQt.DevicesModel { }
        }

        section.property: "Section"
        section.delegate: LingmoUI.ListSectionHeader {
            required property string section

            width: ListView.view.width
            text: {
                switch (section) {
                case "Blocked":
                    return i18n("Blocked");
                case "Connected":
                    return i18n("Connected");
                case "Available":
                    return i18n("Available");
                default:
                    return "";
                }
            }
        }

        delegate: QQC2.ItemDelegate {
            id: delegate

            required property var model

            width: ListView.view.width

            onClicked: root.KCMUtils.ConfigModule.push("Device.qml", { device: model.Device })

            contentItem: RowLayout {
                spacing: LingmoUI.Units.smallSpacing

                KD.IconTitleSubtitle {
                    title: delegate.model.Name
                    subtitle: root.infoText(delegate.model.Device)
                    icon.name: delegate.model.Icon
                    icon.width: LingmoUI.Units.iconSizes.medium
                    Layout.fillWidth: true
                }

                QQC2.ToolButton {
                    text: delegate.model.Connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: delegate.model.Connected ? "network-disconnect-symbolic" : "network-connect-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    visible: !delegate.model.Blocked
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered

                    onClicked: {
                        if (delegate.model.Connected) {
                            root.makeCall(delegate.model.Device.disconnectFromDevice())
                        } else {
                            root.makeCall(delegate.model.Device.connectToDevice())
                        }
                    }
                }

                QQC2.ToolButton {
                    action: ForgetDeviceAction {
                        dialog: forgetDeviceDialog
                        device: delegate.model.Device
                    }
                    display: QQC2.AbstractButton.IconOnly
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                }
            }
        }
    }

    function infoText(device: BluezQt.Device): string {
        const { battery } = device;
        const labels = [];

        labels.push(Script.deviceTypeToString(device));

        if (battery) {
            labels.push(i18n("%1% Battery", battery.percentage));
        }

        return labels.join(" · ");
    }
}
