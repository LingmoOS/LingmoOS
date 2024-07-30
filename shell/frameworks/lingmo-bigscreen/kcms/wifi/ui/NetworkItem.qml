/*
    SPDX-FileCopyrightText: 2018 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmoui 2.12 as LingmoUI

LingmoUI.AbstractListItem {
    id: connectionItem

    property bool activating: model.ConnectionState == LingmoNM.Enums.Activating
    property bool deactivating: model.ConnectionState == LingmoNM.Enums.Deactivating
    property bool predictableWirelessPassword: !model.Uuid && model.Type == LingmoNM.Enums.Wireless &&
                                               (model.SecurityType == LingmoNM.Enums.StaticWep || model.SecurityType == LingmoNM.Enums.WpaPsk ||
                                                model.SecurityType == LingmoNM.Enums.Wpa2Psk)

    checked: connectionView.currentIndex === index && connectionView.activeFocus
    contentItem: Item {
        implicitWidth: delegateLayout.implicitWidth;
        implicitHeight: delegateLayout.implicitHeight;

        ColumnLayout {
            id: delegateLayout
            anchors {
                left: parent.left;
                top: parent.top;
                right: parent.right;
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Math.round(units.gridUnit / 2)

                LingmoUI.Icon {
                    id: connectionSvgIcon
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.preferredHeight: LingmoCore.Units.iconSizes.medium
                    Layout.preferredWidth: LingmoCore.Units.iconSizes.medium
                    color: LingmoUI.Theme.textColor
                    //elementId: model.ConnectionIcon
                    source: itemSignalIcon(model.Signal)
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    LingmoUI.Heading {
                        id: connectionNameLabel
                        Layout.alignment: Qt.AlignLeft
                        level: 2
                        elide: Text.ElideRight
                        font.weight: model.ConnectionState == LingmoNM.Enums.Activated ? Font.DemiBold : Font.Normal
                        font.italic: model.ConnectionState == LingmoNM.Enums.Activating ? true : false
                        text: model.ItemUniqueName
                        textFormat: Text.PlainText
                    }

                    LingmoUI.Heading {
                        id: connectionStatusLabel
                        Layout.alignment: Qt.AlignLeft
                        level: 3
                        elide: Text.ElideRight
                        opacity: 0.6
                        text: itemText()
                    }
                }
            }
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

    Keys.onReturnPressed: clicked()
    onClicked: {
        if (!model.ConnectionPath) {
            networkSelectionView.devicePath = model.DevicePath
            networkSelectionView.specificPath = model.SpecificPath
            networkSelectionView.connectionName = connectionNameLabel.text
            networkSelectionView.securityType = model.SecurityType
            passwordLayer.open();
        } else if (model.ConnectionState == LingmoNM.Enums.Deactivated) {
            handler.activateConnection(model.ConnectionPath, model.DevicePath, model.SpecificPath)
        } else {
            handler.deactivateConnection(model.ConnectionPath, model.DevicePath)
        }
    }

    onPressAndHold: {
        pathToRemove = model.ConnectionPath
        nameToRemove = model.ItemUniqueName
        networkActions.open()
    }
}
