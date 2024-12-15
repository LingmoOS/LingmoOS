// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0

import org.lingmo.oceanui 1.0
import QtQuick.Layouts 1.15

OceanUIObject{
    OceanUIObject {
        name: "blueToothCtl" + model.name
        parentName: "blueToothAdapters" + model.name + index
        weight: 10
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 0
            isGroup: false
        }
        BluetoothCtl{}
    }

    OceanUIObject {
        name: "myDevice" + model.name
        parentName: "blueToothAdapters" + model.name+ index
        weight: 30
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 0
            isGroup: false
        }

        MyDevice{}
    }

    OceanUIObject {
        name: "otherDevice" + model.name
        parentName: "blueToothAdapters" + model.name+ index
        weight: 40
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 0
            isGroup: false
        }

        OtherDevice{}       
    }
}
