// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0

import org.lingmo.oceanui 1.0
import QtQuick.Layouts 1.15
import org.lingmo.dtk 1.0

OceanUIObject{
    OceanUIObject {
        name: "OtherDeviceTitle"
        parentName: "otherDevice" + model.name
        displayName: qsTr("Other Devices")
        weight: 10
        visible: model.powered
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font.bold: true
                font.pixelSize: 16
                text: oceanuiObj.displayName
            }
        }
    }

    OceanUIObject {
        name: "blueToothSwitch"
        parentName: "otherDevice" + model.name
        pageType: OceanUIObject.Item
        weight: 20
        visible: model.powered

        page: RowLayout {
            CheckBox {
                Layout.leftMargin: 10
                Layout.alignment: Qt.AlignLeft
                checked: oceanuiData.model().displaySwitch
                text: qsTr("Show Bluetooth devices without names")
                onCheckedChanged: {
                    oceanuiData.work().setDisplaySwitch(checked)
                }
            }

            IconButton {
                id: redobtn
                Layout.alignment: Qt.AlignRight
                flat: true
                visible: !model.discovering

                icon.name: "qrc:/icons/lingmo/builtin/icons/bluetooth_redo.dci"
                onClicked: {
                    oceanuiData.work().setAdapterDiscoverable(model.id)
                }
            }

            BusyIndicator {
                id: scanAnimation
                Layout.alignment: Qt.AlignRight
                running: model.discovering
                visible: model.discovering
                implicitWidth: 32
                implicitHeight: 32                
            }
        }

        Component.onCompleted: {
            console.log(" blueToothSwitch  checked changed")
            oceanuiData.work().setAdapterDiscoverable(model.id)
        }
    }


    OceanUIObject {
        name: "otherDeviceList"
        parentName: "otherDevice" + model.name
        weight: 40
        visible: model.powered
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: BlueToothDeviceListView {
            showMoreBtn: false
            showConnectStatus: false
            showPowerStatus: false
            deviceModel: model.otherDevice

            onClicked: function (index, checked) {
            }
        }
    }
}
