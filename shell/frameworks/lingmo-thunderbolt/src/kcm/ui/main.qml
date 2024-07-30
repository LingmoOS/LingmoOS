/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3

import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.kcmutils
import org.kde.bolt 0.1 as Bolt

LingmoUI.Page {
    ConfigModule.buttons: ConfigModule.NoAdditionalButton
    id: root

    title: kcm.name
    implicitWidth: LingmoUI.Units.gridUnit * 20
    implicitHeight: pageRow.contentHeight > 0 ? Math.min(pageRow.contentHeight, LingmoUI.Units.gridUnit * 20)
                                              : LingmoUI.Units.gridUnit * 20

    Bolt.Manager {
        id: boltManager
    }

    LingmoUI.PlaceholderMessage {
        visible: boltManager.isAvailable && (boltManager.securityLevel == Bolt.Bolt.Security.DPOnly || boltManager.securityLevel == Bolt.Bolt.Security.USBOnly)
        anchors.centerIn: parent
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)
        icon.name: "preferences-desktop-thunderbolt"
        text: i18n("Thunderbolt support has been disabled in BIOS");
        explanation: i18n("Follow your system manufacturer's guide to enable Thunderbolt support") 
    }

    LingmoUI.PlaceholderMessage {
        visible: !boltManager.isAvailable
        anchors.centerIn: parent
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)
        icon.name: "preferences-desktop-thunderbolt"
        text: i18n("Thunderbolt subsystem is disabled or unavailable");
        explanation: i18n("If the device supports Thunderbolt, try plugging in a Thunderbolt device") 
    }

    LingmoUI.PageRow {
        id: pageRow
        clip: true
        anchors.fill: parent

        Component.onCompleted: {
            if (boltManager.isAvailable) {
                if (!(boltManager.securityLevel == Bolt.Bolt.Security.DPOnly
                        || boltManager.securityLevel == Bolt.Bolt.Security.USBOnly)) {
                    pageRow.push(deviceList, { manager: boltManager })
                }
            }
        }
    }

    Component {
        id: noBoltPage
        LingmoUI.Page {
            property alias text: label.text
            LingmoUI.PlaceholderMessage {
                id: label

                anchors.centerIn: parent
                width: parent.width - (LingmoUI.Units.largeSpacing *4)
            }
        }
    }

    Component {
        id: deviceList
        DeviceList {
            property alias manager: model.manager
            deviceModel: Bolt.DeviceModel {
                id: model
                showHosts: false
            }

            onItemClicked: function(device) {
                pageRow.push(deviceView, { manager: manager, device: device })
            }
        }
    }

    Component {
        id: deviceView
        DeviceView {
        }
    }
}
