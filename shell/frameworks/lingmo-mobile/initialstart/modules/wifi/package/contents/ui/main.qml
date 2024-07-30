// SPDX-FileCopyrightText: 2017 Martin Kacej <m.kacej@atlas.sk>
// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmouiaddons.formcard 1.0 as FormCard
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmo.mobileinitialstart.wifi 1.0 as WiFi

Item {
    id: root
    property string name: i18n("Network")

    readonly property real cardWidth: Math.min(LingmoUI.Units.gridUnit * 30, root.width - LingmoUI.Units.gridUnit * 2)

    LingmoNM.Handler {
        id: handler
    }

    LingmoNM.EnabledConnections {
        id: enabledConnections
    }

    LingmoNM.NetworkModel {
        id: connectionModel
    }

    LingmoNM.MobileProxyModel {
        id: mobileProxyModel
        sourceModel: connectionModel
        showSavedMode: false
    }

    ConnectDialog {
        id: connectionDialog
    }

    Component.onCompleted: handler.requestScan()

    Timer {
        id: scanTimer
        interval: 10200
        repeat: true
        running: parent.visible

        onTriggered: handler.requestScan()
    }

    ColumnLayout {
        anchors {
            fill: parent
            topMargin: LingmoUI.Units.gridUnit
            bottomMargin: LingmoUI.Units.largeSpacing
        }

        width: root.width
        spacing: LingmoUI.Units.gridUnit

        Label {
            Layout.leftMargin: LingmoUI.Units.gridUnit
            Layout.rightMargin: LingmoUI.Units.gridUnit
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true

            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: i18n("Connect to a WiFi network for network access.")
        }

        FormCard.FormCard {
            id: savedCard
            maximumWidth: root.cardWidth
            visible: enabledConnections.wirelessEnabled && count > 0

            // number of visible entries
            property int count: 0
            function updateCount() {
                count = 0;
                for (let i = 0; i < connectedRepeater.count; i++) {
                    let item = connectedRepeater.itemAt(i);
                    if (item && item.shouldDisplay) {
                        count++;
                    }
                }
            }

            Repeater {
                id: connectedRepeater
                model: mobileProxyModel
                delegate: ConnectionItemDelegate {
                    editMode: false

                    // connected or saved
                    property bool shouldDisplay: (Uuid != "") || ConnectionState === LingmoNM.Enums.Activated
                    onShouldDisplayChanged: savedCard.updateCount()

                    // separate property for visible since visible is false when the whole card is not visible
                    visible: (Uuid != "") || ConnectionState === LingmoNM.Enums.Activated
                }
            }
        }

        FormCard.FormCard {
            Layout.fillHeight: true
            maximumWidth: root.cardWidth
            visible: enabledConnections.wirelessEnabled

            ListView {
                id: listView

                clip: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: mobileProxyModel

                delegate: ConnectionItemDelegate {
                    width: ListView.view.width
                    editMode: false
                    height: visible ? implicitHeight : 0
                    visible: !((Uuid != "") || ConnectionState === LingmoNM.Enums.Activated)
                }
            }
        }
    }
}


