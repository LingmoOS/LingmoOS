/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.bluezqt as BluezQt
import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.plasmoid
import org.kde.lingmo.private.bluetooth as LingmoBt

LingmoExtras.Representation {
    id: root

    required property PlasmoidItem plasmoidItem
    required property LingmoCore.Action addDeviceAction
    required property LingmoCore.Action toggleBluetoothAction

    readonly property bool emptyList: BluezQt.Manager.devices.length === 0

    implicitWidth: LingmoUI.Units.gridUnit * 24
    implicitHeight: LingmoUI.Units.gridUnit * 24

    focus: true
    collapseMarginsHint: true

    LingmoBt.DevicesProxyModel {
        id: devicesModel
        hideBlockedDevices: true
        sourceModel: LingmoBt.SharedDevicesStateProxyModel
    }

    Keys.onDownPressed: event => {
        if (listView.count === 0) {
            return;
        }
        if (listView.currentIndex < 0 || toolbar.onSwitch.activeFocus) {
            listView.incrementCurrentIndex();
            listView.currentItem.forceActiveFocus();
        } else {
            event.accepted = false;
        }
    }

    QQC2.Action {
        id: addBluetoothDeviceAction

        text: root.addDeviceAction.text
        icon.name: root.addDeviceAction.icon.name

        onTriggered: source => root.addDeviceAction.trigger()
    }

    // Unlike the associated Lingmo Action, this one is for a non-checkable button
    QQC2.Action {
        id: toggleBluetoothAction

        text: i18n("Enable")
        icon.name: root.toggleBluetoothAction.icon.name

        onTriggered: source => root.toggleBluetoothAction.trigger()
    }

    header: Toolbar {
        id: toolbar

        plasmoidItem: root.plasmoidItem
        addDeviceAction: root.addDeviceAction
        toggleBluetoothAction: root.toggleBluetoothAction

        visible: BluezQt.Manager.adapters.length > 0
        focus: true
    }

    LingmoComponents3.ScrollView {
        id: scrollView
        anchors.fill: parent

        // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
        LingmoComponents3.ScrollBar.horizontal.policy: LingmoComponents3.ScrollBar.AlwaysOff

        contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

        contentItem: ListView {
            id: listView

            model: BluezQt.Manager.adapters.length > 0 && !BluezQt.Manager.bluetoothBlocked ? devicesModel : null
            clip: true
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds

            spacing: LingmoUI.Units.smallSpacing
            topMargin: LingmoUI.Units.largeSpacing
            leftMargin: LingmoUI.Units.largeSpacing
            rightMargin: LingmoUI.Units.largeSpacing
            bottomMargin: LingmoUI.Units.largeSpacing

            section.property: "Section"
            // We want to hide the section delegate for the "Connected"
            // group because it's unnecessary; all we want to do here is
            // separate the connected devices from the available ones
            section.delegate: Loader {
                required property string section

                active: section !== "Connected" && BluezQt.Manager.connectedDevices.length > 0

                width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                // Need to manually set the height or else the loader takes up
                // space after the first time it unloads a previously-loaded item
                height: active ? LingmoUI.Units.gridUnit : 0

                // give us 2 frames to try and figure out a layout, this reduces jumpyness quite a bit but doesn't
                // entirely eliminate it https://bugs.kde.org/show_bug.cgi?id=438610
                Behavior on height { PropertyAnimation { duration: 32 } }

                sourceComponent: Item {
                    KSvg.SvgItem {
                        width: parent.width - LingmoUI.Units.gridUnit * 2
                        anchors.centerIn: parent
                        imagePath: "widgets/line"
                        elementId: "horizontal-line"
                    }
                }
            }
            highlight: LingmoExtras.Highlight {}
            highlightMoveDuration: LingmoUI.Units.shortDuration
            highlightResizeDuration: LingmoUI.Units.shortDuration
            delegate: DeviceItem {}

            Keys.onUpPressed: event => {
                if (listView.currentIndex === 0) {
                    listView.currentIndex = -1;
                    toolbar.onSwitch.forceActiveFocus(Qt.BacktabFocusReason);
                } else {
                    event.accepted = false;
                }
            }

            Loader {
                anchors.centerIn: parent
                width: parent.width - (4 * LingmoUI.Units.gridUnit)
                active: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown || BluezQt.Manager.bluetoothBlocked || root.emptyList
                sourceComponent: LingmoExtras.PlaceholderMessage {
                    iconName: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown || BluezQt.Manager.bluetoothBlocked ? "network-bluetooth" : "edit-none"

                    text: {
                        // We cannot use the adapter count here because that can be zero when
                        // bluetooth is disabled even when there are physical devices
                        if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                            return i18n("No Bluetooth adapters available");
                        } else if (BluezQt.Manager.bluetoothBlocked) {
                            return i18n("Bluetooth is disabled");
                        } else if (root.emptyList) {
                            return i18n("No devices found");
                        } else {
                            return "";
                        }
                    }

                    helpfulAction: {
                        if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                            return null;
                        } else if (BluezQt.Manager.bluetoothBlocked) {
                            return toggleBluetoothAction;
                        } else if (root.emptyList) {
                            return addBluetoothDeviceAction;
                        } else {
                            return null;
                        }
                    }
                }
            }
        }
    }
}
