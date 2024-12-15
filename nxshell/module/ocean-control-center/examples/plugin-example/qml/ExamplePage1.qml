// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    property real calcValue
    property real calcType: 0
    OceanUIObject {
        name: "calc"
        parentName: root.name
        displayName: qsTr("calc")
        icon: "oceanui_example"
        weight: 10
        backgroundType: OceanUIObject.Normal // 设置背景样式
        pageType: OceanUIObject.Editor // Editor为page是右边的控件，左边显示displayName、icon等
        page: Button {
            text: oceanuiObj.displayName
            onClicked: {
                calcValue = oceanuiData.calc(calcValue, 2)
            }
        }
    }
    OceanUIObject {
        name: "value"
        parentName: root.name
        displayName: qsTr("value")
        weight: 20
        pageType: OceanUIObject.Editor
        backgroundType: OceanUIObject.ClickStyle // ClickStyl表示有点击效果，点击时会发出active信号
        page: RowLayout {
            Text {
                text: calcValue
            }
            ComboBox {}
        }
        onActive: cmd => console.log(this, "onActive:", cmd)
    }
    OceanUIObject {
        name: "group"
        parentName: root.name
        displayName: qsTr("group")
        weight: 30
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {} // 组效果，其子项会放在一个组里
        OceanUIObject {
            name: "item2" // name要求当前组内唯一
            parentName: root.name + "/group" // parentName要求可定位到对应项，可用多个OceanUIObject的name组合
            displayName: qsTr("value")
            weight: 20
            pageType: OceanUIObject.Item // Item的page将占整个区域
            page: Text {
                text: calcValue
            }
        }
        OceanUIObject {
            name: "item1"
            parentName: root.name + "/group"
            displayName: qsTr("value")
            weight: 10
            pageType: OceanUIObject.Editor
            page: Text {
                text: calcValue
            }
        }
        OceanUIObject {
            id: calcTypeObj
            name: "calcType"
            parentName: root.name
            displayName: qsTr("calc type")
            description: qsTr("description")
            icon: "oceanui_example"
            weight: 30
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Button {
                text: oceanuiObj.displayName
                onClicked: {
                    calcType++
                    if (calcType >= 4) {
                        calcType = 0
                    }
                    oceanuiData.setCalcType(calcType)
                }
            }
            Connections {
                target: oceanuiData
                function onCalcTypeChanged(cType) {
                    calcTypeObj.displayName = cType
                }
            }
        }
        OceanUIObject {
            name: "calcType" // 该OceanUIObject会显示在example_2中，不建议这样写
            parentName: "example_2/body" // OceanUIObject位置只与parentName和weight有关，与其自身位置无关
            weight: 80
            displayName: qsTr("calc type")
            pageType: OceanUIObject.Editor
            backgroundType: OceanUIObject.Normal
            page: Text {
                text: calcType
            }
        }
    }

    OceanUIObject {
        name: "group2"
        parentName: root.name
        displayName: qsTr("group2")
        weight: 40
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIRepeater { // OceanUIRepeater配合model可实现多个OceanUIObject
            model: 3
            delegate: OceanUIObject {
                name: "item" + (index + 1)
                parentName: root.name + "/group2"
                displayName: qsTr("Item") + (index + 1)
                weight: 30 + index
                pageType: OceanUIObject.Editor
                page: Switch {}
            }
        }
    }
}
