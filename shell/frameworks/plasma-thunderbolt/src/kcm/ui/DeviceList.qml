/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls

import org.kde.kirigami 2.12 as Kirigami
import org.kde.bolt 0.1 as Bolt
import "utils.js" as Utils

Kirigami.ScrollablePage {
    id: page

    property Bolt.DeviceModel deviceModel: null

    signal itemClicked(Bolt.Device device)

    header: RowLayout {
        CheckBox {
            id: enableBox
            text: i18n("Enable Thunderbolt devices")

            checked: deviceModel.manager.authMode == Bolt.Bolt.AuthMode.Enabled

            onToggled: {
                deviceModel.manager.authMode = enableBox.checked
                    ? Bolt.Bolt.AuthMode.Enabled
                    : Bolt.Bolt.AuthMode.Disabled
            }
        }
    }

    ListView {
        id: view
        model: deviceModel
        enabled: enableBox.checked

        property int _evalTrigger: 0

        Timer {
            interval: 2000
            running: view.visible
            repeat: true
            onTriggered: view._evalTrigger++;
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.largeSpacing

            visible: view.count === 0

            icon.name: "preferences-desktop-thunderbolt"
            text: i18n("No Thunderbolt devices connected")
            explanation: i18n("Plug in a Thunderbolt device")
        }

        delegate: ItemDelegate {
            id: item

            property var _deviceStatus: Utils.deviceStatus(model.device, true)

            width: view.width

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                BusyIndicator {
                    visible: model.device.status == Bolt.Bolt.Status.Authorizing
                    running: visible
                    implicitWidth: Kirigami.Units.iconSizes.smallMedium
                    implicitHeight: Kirigami.Units.iconSizes.smallMedium
                }

                Label {
                    Layout.fillWidth: true
                    text: model.device.label
                    elide: Text.ElideRight
                    color: item.highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.TextColor
                }

                Label {
                    text: view._evalTrigger, item._deviceStatus.text
                    color: item.highlighted ? Kirigami.Theme.highlightedTextColor : item._deviceStatus.color
                }
            }

            onClicked: {
                page.itemClicked(model.device)
            }
        }
    }
}
