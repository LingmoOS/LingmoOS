import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Lingmo.Settings 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.Bluez 1.0 as Bluez
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Bluetooth")

    property bool bluetoothDisConnected: Bluez.Manager.bluetoothBlocked

    onBluetoothDisConnectedChanged: {
        bluetoothSwitch.checked = !bluetoothDisConnected
    }

    function setBluetoothEnabled(enabled) {
        Bluez.Manager.bluetoothBlocked = !enabled

        for (var i = 0; i < Bluez.Manager.adapters.length; ++i) {
            var adapter = Bluez.Manager.adapters[i]
            adapter.powered = enabled
        }
    }

    Bluez.DevicesProxyModel {
        id: devicesProxyModel
        sourceModel: devicesModel
    }

    Bluez.DevicesModel {
        id: devicesModel
    }

    Bluez.BluetoothManager {
        id: bluetoothMgr

        onShowPairDialog: {
            _pairDialog.title = name
            _pairDialog.pin = pin
            _pairDialog.visible = true
        }

        onPairFailed: {
            rootWindow.showPassiveNotification(qsTr("Pairing unsuccessful"), 3000)
        }

        onConnectFailed: {
            rootWindow.showPassiveNotification(qsTr("Connecting Unsuccessful"), 3000)
        }
    }

    PairDialog {
        id: _pairDialog
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            anchors.bottomMargin: LingmoUI.Units.largeSpacing

            RoundedItem {
                id: mainItem
                spacing: LingmoUI.Units.largeSpacing

                RowLayout {
                    Label {
                        text: qsTr("Bluetooth")
                        color: LingmoUI.Theme.disabledTextColor
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: bluetoothSwitch
                        Layout.fillHeight: true
                        rightPadding: 0
                        checked: !Bluez.Manager.bluetoothBlocked
                        onCheckedChanged: setBluetoothEnabled(checked)
                    }
                }

                ListView {
                    id: _listView
                    visible: count > 0
                    interactive: false
                    spacing: 0

                    Layout.fillWidth: true

                    Layout.preferredHeight: {
                        var totalHeight = 0
                        for (var i = 0; i < _listView.visibleChildren.length; ++i) {
                            totalHeight += _listView.visibleChildren[i].height
                        }
                        return totalHeight
                    }

                    model: Bluez.Manager.bluetoothOperational ? devicesProxyModel : []

                    section.property: "Section"
                    section.criteria: ViewSection.FullString
                    section.delegate: Label {
                        color: LingmoUI.Theme.disabledTextColor
                        topPadding: LingmoUI.Units.largeSpacing
                        bottomPadding: LingmoUI.Units.largeSpacing
                        text: section == "My devices" ? qsTr("My devices")
                                                     : qsTr("Other devices")
                    }

                    delegate: Item {
                        width: ListView.view.width
                        height: _itemLayout.implicitHeight + LingmoUI.Units.largeSpacing

                        property bool paired: model.Connected && model.Paired

                        ColumnLayout {
                            id: _itemLayout
                            anchors.fill: parent
                            anchors.leftMargin: 0
                            anchors.rightMargin: 0
                            anchors.topMargin: LingmoUI.Units.smallSpacing
                            anchors.bottomMargin: LingmoUI.Units.smallSpacing
                            spacing: 0

                            Item {
                                Layout.fillWidth: true
                                height: _contentLayout.implicitHeight + LingmoUI.Units.largeSpacing

                                Rectangle {
                                    anchors.fill: parent
                                    radius: LingmoUI.Theme.smallRadius
                                    color: LingmoUI.Theme.textColor
                                    opacity: mouseArea.pressed ? 0.15 :  mouseArea.containsMouse ? 0.1 : 0.0
                                }

                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    acceptedButtons: Qt.LeftButton

                                    onClicked: {
                                        if (model.Connected || model.Paired){
                                            additionalSettings.toggle()
                                            return
                                        }

                                        if (model.Paired) {
                                            bluetoothMgr.connectToDevice(model.Address)
                                        } else {
                                            bluetoothMgr.requestParingConnection(model.Address)
                                        }
                                    }
                                }

                                RowLayout {
                                    id: _contentLayout
                                    anchors.fill: parent
                                    anchors.rightMargin: LingmoUI.Units.smallSpacing

                                    Image {
                                        width: 16
                                        height: 16
                                        sourceSize: Qt.size(16, 16)
                                        source: LingmoUI.Theme.darkMode ? "qrc:/images/sidebar/all/bluetooth.svg"
                                                                      : "qrc:/images/sidebar/light/bluetooth.svg"
                                        Layout.alignment: Qt.AlignVCenter
                                    }

                                    Label {
                                        text: model.DeviceFullName
                                        Layout.fillWidth: true
                                        Layout.alignment: Qt.AlignVCenter
                                    }

                                    Label {
                                        visible: model.Paired
                                        text: model.Connected ? qsTr("Connected") : qsTr("Not Connected")
                                    }
                                }
                            }

                            Hideable {
                                id: additionalSettings
                                spacing: 0

                                ColumnLayout {
                                    Item {
                                        height: LingmoUI.Units.largeSpacing
                                    }

                                    RowLayout {
                                        spacing: LingmoUI.Units.largeSpacing
                                        Layout.leftMargin: LingmoUI.Units.smallSpacing

                                        Button {
                                            text: qsTr("Connect")
                                            visible: !model.Connected
                                            onClicked: {
                                                if (model.Paired) {
                                                    bluetoothMgr.connectToDevice(model.Address)
                                                } else {
                                                    bluetoothMgr.requestParingConnection(model.Address)
                                                }
                                            }
                                        }

                                        Button {
                                            text: qsTr("Disconnect")
                                            visible: model.Connected
                                            onClicked: {
                                                bluetoothMgr.deviceDisconnect(model.Address)
                                                additionalSettings.hide()
                                            }
                                        }

                                        Button {
                                            text: qsTr("Forget This Device")
                                            flat: true
                                            onClicked: {
                                                bluetoothMgr.deviceRemoved(model.Address)
                                                additionalSettings.hide()
                                            }
                                        }
                                    }
                                }

                                HorizontalDivider {}
                            }
                        }
                    }
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing * 2
            }
        }
    }
}
