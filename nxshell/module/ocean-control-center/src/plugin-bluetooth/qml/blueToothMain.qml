// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0

import org.lingmo.oceanui 1.0
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.qmlmodels 1.2

OceanUIObject{
    name: "bluetoothSetting"
    parentName: "bluetooth"
    weight: 10
    pageType: OceanUIObject.Item
    page: OceanUIGroupView {
        spacing: 0
        isGroup: false
    }
    OceanUIRepeater {
        model: oceanuiData.model().blueToothAdaptersModel()

        delegate: OceanUIObject {
            name: "blueToothAdapters" + model.name + index
            parentName: "bluetoothSetting"
            weight: 10
            pageType: OceanUIObject.Item

            page: OceanUIGroupView {
                spacing: 0
                isGroup: false
            }

            Adapters{}
        }
    }
}
