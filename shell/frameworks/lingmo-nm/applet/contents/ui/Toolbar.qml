/*
    SPDX-FileCopyrightText: 2013-2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.2
import QtQuick.Layouts 1.2
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.kcmutils as KCMUtils

RowLayout {
    id: toolbar

    readonly property var displayWifiMessage: !wifiSwitchButton.checked && wifiSwitchButton.visible
    readonly property var displayWwanMessage: !wwanSwitchButton.checked && wwanSwitchButton.visible
    readonly property var displayplaneModeMessage: planeModeSwitchButton.checked && planeModeSwitchButton.visible

    property bool hasConnections
    property alias searchTextField: searchTextField

    LingmoNM.EnabledConnections {
        id: enabledConnections

        // When user interactively toggles a checkbox, a binding may be
        // preserved, but the state gets out of sync until next relevant
        // notify signal is dispatched. So, we refresh the bindings here.

        onWirelessEnabledChanged: wifiSwitchButton.checked = Qt.binding(() =>
            wifiSwitchButton.administrativelyEnabled && enabledConnections.wirelessEnabled
        );

        onWwanEnabledChanged: wwanSwitchButton.checked = Qt.binding(() =>
            wwanSwitchButton.administrativelyEnabled && enabledConnections.wwanEnabled
        );
    }

    spacing: LingmoUI.Units.smallSpacing * 3

    RowLayout {
        // Add margin before switches for consistency with other applets
        Layout.leftMargin: LingmoUI.Units.smallSpacing

        spacing: parent.spacing

        // Only show when switches are visible (and avoid parent spacing otherwise)
        visible: availableDevices.wirelessDeviceAvailable || availableDevices.modemDeviceAvailable

        LingmoComponents3.Switch {
            id: wifiSwitchButton

            // can't overload Item::enabled, because it's being used for other things, like Edit Mode on a desktop
            readonly property bool administrativelyEnabled:
                !LingmoNM.Configuration.airplaneModeEnabled
                && availableDevices.wirelessDeviceAvailable
                && enabledConnections.wirelessHwEnabled

            checked: administrativelyEnabled && enabledConnections.wirelessEnabled
            enabled: administrativelyEnabled

            icon.name: administrativelyEnabled ? "network-wireless-on" : "network-wireless-off"
            visible: availableDevices.wirelessDeviceAvailable

            KeyNavigation.right: wwanSwitchButton.visible ? wwanSwitchButton : wwanSwitchButton.KeyNavigation.right

            onToggled: handler.enableWireless(checked);

            LingmoComponents3.ToolTip {
                text: i18n("Enable Wi-Fi")
            }

            LingmoComponents3.BusyIndicator {
                parent: wifiSwitchButton
                anchors {
                    fill: wifiSwitchButton.contentItem
                    leftMargin: wifiSwitchButton.indicator.width + wifiSwitchButton.spacing
                }
                z: 1

                // Scanning may be too fast to notice. Prolong the animation up to at least `humanMoment`.
                running: handler.scanning || timer.running
                Timer {
                    id: timer
                    interval: LingmoUI.Units.humanMoment
                }
                Connections {
                    target: handler
                    function onScanningChanged() {
                        if (handler.scanning) {
                            timer.restart();
                        }
                    }
                }
            }
        }

        LingmoComponents3.Switch {
            id: wwanSwitchButton

            // can't overload Item::enabled, because it's being used for other things, like Edit Mode on a desktop
            readonly property bool administrativelyEnabled:
                !LingmoNM.Configuration.airplaneModeEnabled
                && availableDevices.modemDeviceAvailable
                && enabledConnections.wwanHwEnabled

            checked: administrativelyEnabled && enabledConnections.wwanEnabled
            enabled: administrativelyEnabled

            icon.name: administrativelyEnabled ? "network-mobile-on" : "network-mobile-off"
            visible: availableDevices.modemDeviceAvailable

            KeyNavigation.left: wifiSwitchButton
            KeyNavigation.right: planeModeSwitchButton.visible ? planeModeSwitchButton : planeModeSwitchButton.KeyNavigation.right

            onToggled: handler.enableWwan(checked);

            LingmoComponents3.ToolTip {
                text: i18n("Enable mobile network")
            }
        }

        LingmoComponents3.Switch {
            id: planeModeSwitchButton

            property bool initialized: false

            checked: LingmoNM.Configuration.airplaneModeEnabled

            icon.name: LingmoNM.Configuration.airplaneModeEnabled ? "network-flightmode-on" : "network-flightmode-off"

            visible: availableDevices.modemDeviceAvailable || availableDevices.wirelessDeviceAvailable

            KeyNavigation.left: wwanSwitchButton.visible ? wwanSwitchButton : wwanSwitchButton.KeyNavigation.left
            KeyNavigation.right: hotspotButton.visible ? hotspotButton : hotspotButton.KeyNavigation.right

            onToggled: {
                handler.enableAirplaneMode(checked);
                LingmoNM.Configuration.airplaneModeEnabled = checked;
            }

            LingmoComponents3.ToolTip {
                text: planeModeSwitchButton.checked ?
                    xi18nc("@info", "Disable airplane mode<nl/><nl/>This will enable Wi-Fi and Bluetooth") :
                    xi18nc("@info", "Enable airplane mode<nl/><nl/>This will disable Wi-Fi and Bluetooth")
            }
        }
    }

    LingmoComponents3.ToolButton {
        id: hotspotButton

        visible: handler.hotspotSupported
        checkable: true

        text: i18n("Hotspot")
        icon.name: "network-wireless-on"

        KeyNavigation.left: planeModeSwitchButton.visible ? planeModeSwitchButton : planeModeSwitchButton.KeyNavigation.left
        KeyNavigation.right: searchTextField

        onClicked: {
            if (LingmoNM.Configuration.hotspotConnectionPath) {
                checked = false
                handler.stopHotspot()
            } else {
                checked = true
                handler.createHotspot()
            }
        }

        LingmoComponents3.ToolTip {
            id: tooltip
        }

        Connections {
            target: handler
            function onHotspotCreated() {
                hotspotButton.checked = true
                tooltip.text = i18n("Disable Hotspot")
            }

            function onHotspotDisabled() {
                hotspotButton.checked = false
                tooltip.text = i18n("Create Hotspot")
            }
        }

        Component.onCompleted: {
            checked = LingmoNM.Configuration.hotspotConnectionPath
            tooltip.text = LingmoNM.Configuration.hotspotConnectionPath ? i18n("Disable Hotspot") : i18n("Create Hotspot")
        }
    }

    LingmoExtras.SearchField {
        id: searchTextField

        Layout.fillWidth: true

        enabled: toolbar.hasConnections || text.length > 0

        // This uses expanded to ensure the binding gets reevaluated
        // when the plasmoid is shown again and that way ensure we are
        // always in the correct state on show.
        focus: mainWindow.expanded && !LingmoUI.InputMethod.willShowOnActive

        KeyNavigation.left: hotspotButton.visible ? hotspotButton : hotspotButton.KeyNavigation.left
        KeyNavigation.right: openEditorButton

        onTextChanged: {
            appletProxyModel.setFilterFixedString(text)
        }
    }

    LingmoComponents3.ToolButton {
        id: openEditorButton

        visible: mainWindow.kcmAuthorized && !(plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentDrawsPlasmoidHeading)

        icon.name: "configure"

        LingmoComponents3.ToolTip {
            text: i18n("Configure network connections…")
        }

        onClicked: {
            KCMUtils.KCMLauncher.openSystemSettings(mainWindow.kcm)
        }
    }
}
