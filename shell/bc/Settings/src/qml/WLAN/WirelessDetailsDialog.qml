/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Window 2.3
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import LingmoUI 1.0 as LingmoUI
import Lingmo.NetworkManagement 1.0 as NM

Dialog {
    id: control
    title: model.itemUniqueName

    // width: Math.max(detailsLayout.implicitWidth, footer.implicitWidth)

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    height: detailsLayout.childrenRect.height + footerLayout.childrenRect.height + LingmoUI.Units.largeSpacing * 1.5
    modal: true

    signal forgetBtnClicked()

    NM.WirelessItemSettings {
        id: settings
    }

    Component.onCompleted: {
        if (model.connectionPath) {
            settings.path = model.connectionPath
            autoJoinSwitch.checked = settings.autoConnect
            autoJoinSwitch.visible = true
            autoJoinLabel.visible = true
        }
    }

    ColumnLayout {
        id: detailsLayout
        spacing: LingmoUI.Units.largeSpacing * 1.5

        GridLayout {
            id: gridLayout
            columns: 2
            columnSpacing: LingmoUI.Units.largeSpacing
            rowSpacing: LingmoUI.Units.smallSpacing

            Label {
                id: autoJoinLabel
                text: qsTr("Auto-Join")
                visible: false
                Layout.alignment: Qt.AlignRight
                color: LingmoUI.Theme.disabledTextColor
            }

            Switch {
                id: autoJoinSwitch
                rightPadding: 0
                Layout.fillHeight: true
                visible: false
                Layout.alignment: Qt.AlignRight
                onCheckedChanged: settings.autoConnect = checked
            }

            Label {
                text: qsTr("Security")
                color: LingmoUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: securityLabel
                text: model.securityTypeString
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("Signal")
                color: LingmoUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: signalLabel
                text: model.signal
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("IPv4 Address")
                color: LingmoUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: ipv4AddressLabel
                // text: model.ipV4Address
                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true
            }

            Label {
                font.bold: true
                text: qsTr("IPv6 Address")
                color: LingmoUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: ipV6AddressLabel
                // text: model.ipV6Address
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
            }

            Label {
                font.bold: true
                text: qsTr("MAC Address")
                color: LingmoUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: macAddressLabel
                // text: model.macAddress
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
            }

            Label {
                font.bold: true
                text: qsTr("Gateway")
                color: LingmoUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: routerLabel
                // text: model.gateway
                Layout.alignment: Qt.AlignRight
            }

            Label {
                font.bold: true
                text: qsTr("DNS")
                color: LingmoUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignRight
            }

            Label {
                id: dnsLabel
                // text: model.nameServer
                Layout.alignment: Qt.AlignRight
            }
        }

        RowLayout {
            id: footerLayout
            spacing: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("Close")
                Layout.fillWidth: true
                onClicked: control.reject()
            }

            Button {
                text: qsTr("Forget this network")
                Layout.fillWidth: true
                flat: true
                onClicked: {
                    handler.removeConnection(model.connectionPath)
                    control.reject()
                }
            }
        }
    }
}
