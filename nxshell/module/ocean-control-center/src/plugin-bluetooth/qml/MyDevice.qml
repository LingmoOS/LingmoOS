// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0

import org.lingmo.oceanui 1.0
import QtQuick.Layouts 1.15
import org.lingmo.dtk 1.0

OceanUIObject{
    OceanUIObject {
        name: "myDeviceTitle"
        parentName: "myDevice" + model.name
        displayName: qsTr("My Devices")
        weight: 10
        pageType: OceanUIObject.Item
        visible: model.myDeviceVisiable
        page: Label {
            leftPadding: 10
            font.bold: true
            font.pixelSize: 16
            text: oceanuiObj.displayName
        }

    }

    OceanUIObject {
        name: "myDeviceList"
        parentName: "myDevice" + model.name
        weight: 11
        backgroundType: OceanUIObject.Normal
        visible: model.myDeviceVisiable
        pageType: OceanUIObject.Item
        page: BlueToothDeviceListView {
            deviceModel: model.myDevice
            onClicked: function (index, checked) {
            }
        }
    }
}
