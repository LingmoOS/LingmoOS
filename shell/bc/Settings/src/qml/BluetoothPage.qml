import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Lingmo.Settings 1.0
import LingmoUI 1.0 as LingmoUI
import org.kde.bluezqt 1.0 as BluezQt
import org.kde.plasma.private.bluetooth 1.0

ItemPage {
    headerTitle: qsTr("Bluetooth")

    function setBluetoothEnabled(enabled) {
        BluezQt.Manager.bluetoothBlocked = !enabled

        for (var i = 0; i < BluezQt.Manager.adapters.length; ++i) {
            var adapter = BluezQt.Manager.adapters[i];
            adapter.powered = enabled;
        }
    }

//    Label {
//        id: noBluetoothMessage
//        anchors.centerIn: parent
//        visible: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown
//        text: qsTr("No Bluetooth adapters found")
//    }

    Label {
        anchors.centerIn: parent
        text: qsTr("Bluetooth is disabled")
        visible: BluezQt.Manager.operational && !BluezQt.Manager.bluetoothOperational // && !noBluetoothMessage.visible
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            anchors.bottomMargin: LingmoUI.Units.largeSpacing

            Switch {
                id: bluetoothSwitch
                height: 50
                onCheckedChanged: setBluetoothEnabled(checked)
                Component.onCompleted: bluetoothSwitch.checked = BluezQt.Manager.operational
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                DevicesProxyModel {
                    id: devicesModel
                    sourceModel: BluezQt.DevicesModel { }
                }

                model: BluezQt.Manager.bluetoothOperational ? devicesModel : []

                delegate: Item {
                    width: ListView.view.width
                    height: 50

                    Rectangle {
                        anchors.fill: parent
                        color: "red"
                    }

                    ColumnLayout {
                        anchors.fill: parent

                        Label {
                            text: model.name
                        }
                    }
                }
            }
        }
    }
}
