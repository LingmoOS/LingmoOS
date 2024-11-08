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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "../"

import LingmoUI 1.0 as LingmoUI
import Lingmo.NetworkManagement 1.0 as NM

ColumnLayout {
    id: _contentLayout
    spacing: LingmoUI.Units.largeSpacing

    ConnectDialog {
        id: connectDialog

        onConnect: {
            handler.addAndActivateConnection(connectDialog.devicePath,
                                             connectDialog.specificPath,
                                             password)

            console.log(connectDialog.devicePath + ", "
                        + connectDialog.specificPath + ", " +
                        password)
        }
    }

    RowLayout {
        spacing: LingmoUI.Units.smallSpacing * 1.5

        Label {
            id: wlanLabel
            text: qsTr("WLAN")
            color: LingmoUI.Theme.disabledTextColor
        }

        Item {
            Layout.fillWidth: true
        }

        Switch {
            id: wirelessSwitch
            height: wlanLabel.implicitHeight
            leftPadding: 0
            rightPadding: 0

            checked: enabledConnections.wirelessEnabled
            onCheckedChanged: {
                if (checked) {
                    if (!enabledConnections.wirelessEnabled) {
                        handler.enableWireless(checked)
                        handler.requestScan()
                    }
                } else {
                    if (enabledConnections.wirelessEnabled) {
                        handler.enableWireless(checked)
                    }
                }
            }
        }
    }

    ListView {
        id: wirelessView
        Layout.fillWidth: true
        Layout.preferredHeight: {
            var totalHeight = 0
            for (var i = 0; i < wirelessView.visibleChildren.length; ++i) {
                totalHeight += wirelessView.visibleChildren[i].height
            }
            return totalHeight
        }

        Component.onCompleted: {
            wirelessView.contentY = 0
        }

        clip: true

        model: NM.AppletProxyModel {
            type: NM.AppletProxyModel.WirelessType
            sourceModel: connectionModel
        }

        spacing: 0
        interactive: false
        visible: count > 0

        delegate: WifiItem {
            width: ListView.view.width
            // height: ListView.view.itemHeight
        }

        function hideAllItems() {
            for (var i = 0; i < wirelessView.count; ++i) {
                wirelessView.itemAtIndex(i).hideAdditional()
            }
        }
    }
}
