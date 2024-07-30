/*
    SPDX-FileCopyrightText: 2013-2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.2
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.networkmanager as NMQt
import QtQuick.Layouts 1.1
import org.kde.kcmutils as KCMUtils
import org.kde.config as KConfig

PlasmoidItem {
    id: mainWindow

    readonly property string kcm: "kcm_networkmanagement"
    readonly property bool kcmAuthorized: KConfig.KAuthorized.authorizeControlModule("kcm_networkmanagement")
    readonly property bool delayModelUpdates: fullRepresentationItem !== null
        && fullRepresentationItem.connectionModel !== null
        && fullRepresentationItem.connectionModel.delayModelUpdates
    readonly property bool airplaneModeAvailable: availableDevices.modemDeviceAvailable || availableDevices.wirelessDeviceAvailable
    readonly property bool inPanel: (Plasmoid.location === LingmoCore.Types.TopEdge
        || Plasmoid.location === LingmoCore.Types.RightEdge
        || Plasmoid.location === LingmoCore.Types.BottomEdge
        || Plasmoid.location === LingmoCore.Types.LeftEdge)
    property alias planeModeSwitchAction: planeAction

    toolTipMainText: i18n("Networks")
    toolTipSubText: {
        const activeConnections = networkStatus.activeConnections;

        if (!airplaneModeAvailable) {
            return activeConnections;
        }

        if (LingmoNM.Configuration.airplaneModeEnabled) {
            return i18nc("@info:tooltip", "Middle-click to turn off Airplane Mode");
        } else {
            const hint = i18nc("@info:tooltip", "Middle-click to turn on Airplane Mode");
            return activeConnections ? `${activeConnections}\n${hint}` : hint;
        }
    }

    Plasmoid.busy: connectionIconProvider.connecting
    Plasmoid.icon: inPanel ? connectionIconProvider.connectionIcon + "-symbolic" : connectionIconProvider.connectionTooltipIcon
    switchWidth: LingmoUI.Units.gridUnit * 10
    switchHeight: LingmoUI.Units.gridUnit * 10

    // Only exists because the default CompactRepresentation doesn't expose
    // a middle-click action.
    // TODO remove once it gains that feature.
    compactRepresentation: CompactRepresentation {
        airplaneModeAvailable: mainWindow.airplaneModeAvailable
        iconName: Plasmoid.icon
    }
    fullRepresentation: PopupDialog {
        id: dialogItem
        nmHandler: handler
        nmStatus: networkStatus
        Layout.minimumWidth: LingmoUI.Units.iconSizes.medium * 10
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 20
        anchors.fill: parent
        focus: true
    }

    Plasmoid.contextualActions: [
        LingmoCore.Action {
            text: i18n("Enable Wi-Fi")
            icon.name: "network-wireless-on"
            priority: LingmoCore.Action.LowPriority
            checkable: true
            checked: enabledConnections.wirelessEnabled
            visible: enabledConnections.wirelessHwEnabled
                        && availableDevices.wirelessDeviceAvailable
                        && !LingmoNM.Configuration.airplaneModeEnabled
            onTriggered: checked => {handler.enableWireless(checked)}
        },
        LingmoCore.Action {
            text: i18n("Enable Mobile Network")
            icon.name: "network-mobile-on"
            priority: LingmoCore.Action.LowPriority
            checkable: true
            checked: enabledConnections.wwanEnabled
            visible: enabledConnections.wwanHwEnabled
                        && availableDevices.modemDeviceAvailable
                        && !LingmoNM.Configuration.airplaneModeEnabled
            onTriggered: checked => {handler.enableWwan(checked)}
        },
        LingmoCore.Action {
            id: planeAction
            text: i18n("Enable Airplane Mode")
            icon.name: "network-flightmode-on"
            priority: LingmoCore.Action.LowPriority
            checkable: true
            checked: LingmoNM.Configuration.airplaneModeEnabled
            visible: mainWindow.airplaneModeAvailable
            onTriggered: checked => {
                handler.enableAirplaneMode(checked)
                LingmoNM.Configuration.airplaneModeEnabled = checked
            }
        },
        LingmoCore.Action {
            text: i18n("Open Network Login Page…")
            icon.name: "network-flightmode-on"
            priority: LingmoCore.Action.LowPriority
            visible: networkStatus.connectivity === NMQt.NetworkManager.Portal
            onTriggered: Qt.openUrlExternally("http://networkcheck.kde.org")
        }
    ]

    LingmoCore.Action {
        id: configureAction
        text: i18n("&Configure Network Connections…")
        icon.name: "configure"
        visible: kcmAuthorized
        shortcut: "alt+d, s"
        onTriggered: KCMUtils.KCMLauncher.openSystemSettings(kcm)
    }

    Component.onCompleted: {
        plasmoid.setInternalAction("configure", configureAction);
    }

    LingmoNM.EnabledConnections {
        id: enabledConnections
    }

    LingmoNM.AvailableDevices {
        id: availableDevices
    }

    LingmoNM.NetworkStatus {
        id: networkStatus
    }

    LingmoNM.ConnectionIcon {
        id: connectionIconProvider
        connectivity: networkStatus.connectivity
    }

    LingmoNM.Handler {
        id: handler
    }

    Timer {
        id: scanTimer
        interval: 10200
        repeat: true
        running: mainWindow.expanded && !LingmoNM.Configuration.airplaneModeEnabled && !mainWindow.delayModelUpdates

        onTriggered: handler.requestScan()
    }
}
