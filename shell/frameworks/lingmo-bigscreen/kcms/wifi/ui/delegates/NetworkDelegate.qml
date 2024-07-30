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
import org.kde.lingmoui as LingmoUI
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import Qt5Compat.GraphicalEffects
import org.kde.lingmo.networkmanagement as LingmoNM

BigScreen.AbstractDelegate {
    id: delegate

    property bool activating: model.ConnectionState == LingmoNM.Enums.Activating
    property bool deactivating: model.ConnectionState == LingmoNM.Enums.Deactivating
    property bool predictableWirelessPassword: !model.Uuid && model.Type == LingmoNM.Enums.Wireless &&
                                               (model.SecurityType == LingmoNM.Enums.StaticWep || model.SecurityType == LingmoNM.Enums.WpaPsk ||
                                                model.SecurityType == LingmoNM.Enums.Wpa2Psk || model.SecurityType == LingmoNM.Enums.SAE)
    property alias connectionStatusLabelText: connectionStatusLabel.text

    checked: connectionView.currentIndex === index && connectionView.activeFocus

    implicitWidth: listView.cellWidth * 2
    implicitHeight: listView.height

    Behavior on implicitWidth {
        NumberAnimation {
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    contentItem: Item {
        id: connectionItemLayout

        LingmoUI.Icon {
            id: connectionSvgIcon
            width: LingmoUI.Units.iconSizes.huge
            height: width
            y: connectionItemLayout.height/2 - connectionSvgIcon.height/2
            source: switch(model.Type){
                    case LingmoNM.Enums.Wireless:
                        return itemSignalIcon(model.Signal)
                    case LingmoNM.Enums.Wired:
                        return "network-wired-activated"
                    }
        }

        ColumnLayout {
            id: textLayout

            anchors {
                left: connectionSvgIcon.right
                right: connectionItemLayout.right
                top: connectionSvgIcon.top
                bottom: connectionSvgIcon.bottom
                leftMargin: LingmoUI.Units.smallSpacing
            }

            LingmoComponents.Label {
                id: connectionNameLabel
                Layout.fillWidth: true
                visible: text.length > 0
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
                color: LingmoUI.Theme.textColor
                textFormat: Text.PlainText
                font.weight: model.ConnectionState == LingmoNM.Enums.Activated ? Font.DemiBold : Font.Normal
                font.italic: model.ConnectionState == LingmoNM.Enums.Activating ? true : false
                text: model.ItemUniqueName
            }

            LingmoComponents.Label {
                id: connectionStatusLabel
                Layout.fillWidth: true
                visible: text.length > 0
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 3
                color: LingmoUI.Theme.textColor
                textFormat: Text.PlainText
                opacity: 0.6
                text: itemText()
            }
        }

        LingmoUI.Icon {
            id: dIcon
            anchors.bottom: parent.bottom
            anchors.bottomMargin: -LingmoUI.Units.largeSpacing
            anchors.right: parent.right
            anchors.rightMargin: -LingmoUI.Units.largeSpacing
            width: LingmoUI.Units.iconSizes.smallMedium
            height: width
            source: Qt.resolvedUrl("../images/green-tick-thick.svg")
            visible: model.ConnectionState == LingmoNM.Enums.Activated ? 1 : 0
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
        listView.currentIndex = 0
        listView.positionViewAtBeginning()
        if (!model.ConnectionPath) {
            networkSelectionView.devicePath = model.DevicePath
            networkSelectionView.specificPath = model.SpecificPath
            networkSelectionView.connectionName = connectionNameLabel.text
            networkSelectionView.securityType = model.SecurityType
            if(model.SecurityType == -1 ){
                console.log("Open Network")
                networkSelectionView.connectToOpenNetwork()
            } else {
                passwordLayer.open();
                passField.forceActiveFocus();
            }
        } else if (model.ConnectionState == LingmoNM.Enums.Deactivated) {
            handler.activateConnection(model.ConnectionPath, model.DevicePath, model.SpecificPath)
        } else {
            handler.deactivateConnection(model.ConnectionPath, model.DevicePath)
        }
    }

    Keys.onMenuPressed: {
        pathToRemove = model.ConnectionPath
        nameToRemove = model.ItemUniqueName
        networkActions.open()
    }

    onPressAndHold: {
        pathToRemove = model.ConnectionPath
        nameToRemove = model.ItemUniqueName
        networkActions.open()
    }
}
