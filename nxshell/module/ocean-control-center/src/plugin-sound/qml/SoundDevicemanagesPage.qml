// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0
import org.lingmo.oceanui 1.0

OceanUIObject {
    OceanUIObject {
        name: "outputDevice"
        parentName: "sound/deviceManger"
        displayName: qsTr("Output Devices")
        description: qsTr("Select whether to enable the devices")
        visible: oceanuiData.model().outPutCount !== 0
        weight: 10
        pageType: OceanUIObject.Editor
    }
    OceanUIObject {
        name: "outputDeviceList"
        parentName: "sound/deviceManger"
        weight: 20
        visible: oceanuiData.model().outPutCount !== 0
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: DeviceListView {
            model: oceanuiData.model().soundOutputDeviceModel()
            onClicked: function (index, checked) {
                oceanuiData.worker().setPortEnableIndex(index, checked, 1)
            }
        }
    }
    OceanUIObject {
        name: "inputDevice"
        parentName: "sound/deviceManger"
        displayName: qsTr("Input Devices")
        description: qsTr("Select whether to enable the devices")
        visible: oceanuiData.model().inPutPortCount !== 0
        weight: 30
        pageType: OceanUIObject.Editor
    }
    OceanUIObject {
        name: "inputDeviceList"
        parentName: "sound/deviceManger"
        weight: 40
        visible: oceanuiData.model().inPutPortCount !== 0
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: DeviceListView {
            model: oceanuiData.model().soundInputDeviceModel()
            onClicked: function (index, checked) {
                oceanuiData.worker().setPortEnableIndex(index, checked, 2)
            }
        }
    }
}
