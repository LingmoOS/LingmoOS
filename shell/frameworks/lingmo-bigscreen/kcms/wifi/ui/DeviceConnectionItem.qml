/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import Qt5Compat.GraphicalEffects
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.coreaddons 1.0 as KCoreAddons

Item {
    id: delegateSettingsItem
    property bool activating: model.ConnectionState == LingmoNM.Enums.Activating
    property bool deactivating: model.ConnectionState == LingmoNM.Enums.Deactivating
    property bool predictableWirelessPassword: !model.Uuid && model.Type == LingmoNM.Enums.Wireless &&
                                               (model.SecurityType == LingmoNM.Enums.StaticWep || model.SecurityType == LingmoNM.Enums.WpaPsk ||
                                                model.SecurityType == LingmoNM.Enums.Wpa2Psk || model.SecurityType == LingmoNM.Enums.SAE)
    property real rxBytes: 0
    property real txBytes: 0
    readonly property ListView listView: ListView.view
    property bool showSpeed: ConnectionState == LingmoNM.Enums.Activated &&
                             (Type == LingmoNM.Enums.Wired ||
                              Type == LingmoNM.Enums.Wireless ||
                              Type == LingmoNM.Enums.Gsm ||
                              Type == LingmoNM.Enums.Cdma)
    width: listView.width
    height: listView.height
    
    Component.onCompleted: {
        if (model.ConnectionState == LingmoNM.Enums.Activated) {
            connectionModel.setDeviceStatisticsRefreshRateMs(DevicePath, showSpeed ? 2000 : 0)
        }
    }
    
    function itemText() {
        if (model.ConnectionState == LingmoNM.Enums.Activating) {
            if (model.Type == LingmoNM.Enums.Vpn)
                return model.VpnState
            else
                return model.DeviceState
        } else if (model.ConnectionState == LingmoNM.Enums.Deactivating) {
            if (model.Type == LingmoNM.Enums.Vpn)
                return model.VpnState
            else
                return model.DeviceState
        } else if (model.ConnectionState == LingmoNM.Enums.Deactivated) {
            var result = model.LastUsed
            if (model.SecurityType > LingmoNM.Enums.NoneSecurity)
                result += ", " + model.SecurityTypeString
            return result
        } else if (model.ConnectionState == LingmoNM.Enums.Activated) {
            return i18n("Connected")
        }
    }
    
    function itemSignalIcon(signalState) {
        if (signalState <= 25){
            return "network-wireless-connected-25"
        } else if (signalState <= 50){
            return "network-wireless-connected-50"
        } else if (signalState <= 75){
            return "network-wireless-connected-75"
        } else if (signalState <= 100){
            return "network-wireless-connected-100"
        } else {
            return "network-wireless-connected-00"
        }
    }
    
    onShowSpeedChanged: {
        connectionModel.setDeviceStatisticsRefreshRateMs(DevicePath, showSpeed ? 2000 : 0)
    }
    
    Timer {
        id: timer
        repeat: true
        interval: 2000
        running: showSpeed
        property real prevRxBytes
        property real prevTxBytes
        Component.onCompleted: {
            prevRxBytes = RxBytes
            prevTxBytes = TxBytes
        }
        onTriggered: {
            rxBytes = (RxBytes - prevRxBytes) * 1000 / interval
            txBytes = (TxBytes - prevTxBytes) * 1000 / interval
            prevRxBytes = RxBytes
            prevTxBytes = TxBytes
        }
    }
    
    ColumnLayout {
        id: colLayoutSettingsItem
        anchors {
            fill: parent
            margins: LingmoUI.Units.largeSpacing
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: model.ConnectionState == LingmoNM.Enums.Activated ? parent.height : parent.height / 3
            Layout.alignment: Qt.AlignTop

            LingmoUI.Icon {
                id: dIcon
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: width / 3
                source: switch(model.Type){
                        case LingmoNM.Enums.Wireless:
                            return itemSignalIcon(model.Signal)
                        case LingmoNM.Enums.Wired:
                            return "network-wired-activated"
                        }
            }
            
            LingmoUI.Heading {
                id: label2
                width: parent.width
                anchors.top: dIcon.bottom
                anchors.topMargin: LingmoUI.Units.largeSpacing
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                level: 2
                maximumLineCount: 2
                elide: Text.ElideRight
                color: LingmoUI.Theme.textColor
                text: model.ItemUniqueName
            }
            
            LingmoUI.Separator {
                id: lblSept
                anchors.top: label2.bottom
                anchors.topMargin: LingmoUI.Units.largeSpacing
                height: 1
                anchors.left: parent.left
                anchors.leftMargin: LingmoUI.Units.largeSpacing
                anchors.right: parent.right
                anchors.rightMargin: LingmoUI.Units.largeSpacing
            }
            
            Rectangle {
                id: setCntStatus
                width: parent.width
                height: LingmoUI.Units.gridUnit * 2
                anchors.top: lblSept.bottom
                anchors.topMargin: LingmoUI.Units.smallSpacing
                color: LingmoUI.Theme.backgroundColor
                
                Item {
                    anchors.fill: parent
                    
                    RowLayout {
                        anchors.centerIn: parent
                        LingmoUI.Icon {
                            Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
                            Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
                            source: Qt.resolvedUrl("images/green-tick-thick.svg")
                            visible: model.ConnectionState == LingmoNM.Enums.Activated ? 1 : 0
                        }
                        LingmoUI.Heading {
                            level: 3
                            text: itemText()
                        }
                    }
                }
            }
            
            LingmoUI.Separator {
                id: lblSept2
                anchors.top: setCntStatus.bottom
                anchors.topMargin: LingmoUI.Units.smallSpacing
                height: 1
                anchors.left: parent.left
                anchors.leftMargin: LingmoUI.Units.largeSpacing
                anchors.right: parent.right
                anchors.rightMargin: LingmoUI.Units.largeSpacing
            }

            DetailsText {
                id: detailsTxtArea
                visible: true
                details: ConnectionDetails
                connected: model.ConnectionState == LingmoNM.Enums.Activated ? 1 : 0
                connectionType: model.Type
                clip: true
                anchors {
                    left: parent.left
                    right: parent.right
                    top: lblSept2.bottom
                    topMargin: LingmoUI.Units.largeSpacing
                    bottom: lblSept3.top
                    bottomMargin: LingmoUI.Units.smallSpacing
                }
            }
            
            LingmoUI.Separator {
                id: lblSept3
                anchors.bottom: label3.top
                anchors.bottomMargin: LingmoUI.Units.smallSpacing
                visible: model.ConnectionState == LingmoNM.Enums.Activated ? 1 : 0
                height: 1
                anchors.left: parent.left
                anchors.leftMargin: LingmoUI.Units.largeSpacing
                anchors.right: parent.right
                anchors.rightMargin: LingmoUI.Units.largeSpacing
            }
            
            RowLayout {
                id: label3
                width: parent.width
                anchors.bottom: parent.bottom
                anchors.bottomMargin: LingmoUI.Units.smallSpacing
                visible: model.ConnectionState == LingmoNM.Enums.Activated ? 1 : 0

                LingmoUI.Heading {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: LingmoUI.Units.largeSpacing
                    wrapMode: Text.WordWrap
                    level: 3
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    color: LingmoUI.Theme.textColor
                    text: i18n("Speed")
                }
                
                LingmoComponents.Label {
                    Layout.alignment: Qt.AlignRight
                    Layout.maximumWidth: LingmoUI.Units.gridUnit * 4
                    text: "⬇ " + KCoreAddons.Format.formatByteSize(rxBytes)
                }
                
                LingmoComponents.Label {
                    Layout.alignment: Qt.AlignRight
                    Layout.maximumWidth: LingmoUI.Units.gridUnit * 4
                    text: "⬆ " + KCoreAddons.Format.formatByteSize(txBytes)
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: LingmoUI.Units.gridUnit
        }
    }
}
