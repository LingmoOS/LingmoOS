import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import LingmoUI 1.0 as LingmoUI
import Lingmo.NetworkManagement 1.0 as NM

import "../"

ItemPage {
    id: control
    headerTitle: qsTr("WLAN")

    property var itemHeight: 45
    property var settingsMap: ({})

    NM.Handler {
        id: handler
    }

    NM.WifiSettings {
        id: wifiSettings
    }

    NM.NetworkModel {
        id: networkModel
    }

    NM.EnabledConnections {
        id: enabledConnections
    }

    NM.IdentityModel {
        id: connectionModel
    }

    NM.Configuration {
        id: configuration
    }

    NewNetworkDialog {
        id: newNetworkDialog

        onConnect: {
            wifiSettings.addOtherConnection(ssid, username, pwd, type)
        }
    }

    Component.onCompleted: {
        handler.requestScan()
    }

    Timer {
        id: scanTimer
        interval: 10200
        repeat: true
        running: control.visible
        onTriggered: handler.requestScan()
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: mainLayout.implicitHeight
        visible: enabledConnections.wirelessHwEnabled

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.bottomMargin: LingmoUI.Units.largeSpacing
            spacing: LingmoUI.Units.largeSpacing * 2

            RoundedItem {
                WifiView {
                    Layout.fillWidth: true
                    visible: enabledConnections.wirelessHwEnabled
                }
            }

            StandardButton {
                Layout.fillWidth: true
                text: ""
                onClicked: newNetworkDialog.show()

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Add other...")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("→")
                    }
                }
            }



            // Hotspot
            // 还未完善
//            RoundedItem {
//                id: hotspotItem
//                visible: handler.hotspotSupported

//                RowLayout {
//                    Label {
//                        text: qsTr("Hotspot")
//                        color: LingmoUI.Theme.disabledTextColor
//                    }

//                    Item {
//                        Layout.fillWidth: true
//                    }

//                    Switch {
//                        Layout.fillHeight: true
//                        rightPadding: 0

//                        onToggled: {
//                            if (checked) {
//                                handler.createHotspot()
//                            } else {
//                                handler.stopHotspot()
//                            }
//                        }
//                    }
//                }

//                Item {
//                    height: LingmoUI.Units.largeSpacing
//                }

//                TextField {
//                    id: ssidName
//                    text: configuration.hotspotName
//                    placeholderText: qsTr("SSID")
//                }

//                TextField {
//                    id: hotspotPassword
//                    placeholderText: qsTr("Password")
//                    text: configuration.hotspotPassword
//                }
//            }

            Item {
                height: LingmoUI.Units.largeSpacing
            }
        }
    }
}
