// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.touchscreen 1.0

OceanUIObject {
    OceanUIObject {
        name: "TouchScreen"
        parentName: "touchscreen"
        displayName: qsTr("TouchScreen")
        weight: 10
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Label {
                height: contentHeight
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
            Label {
                height: contentHeight
                Layout.leftMargin: 10
                wrapMode: Text.WordWrap
                text: qsTr("Set up here when connecting the touch screen")
            }
            Repeater {
                model: oceanuiData.touchScreenMatchModel()
                delegate: ItemDelegate{
                    id: touchItem
                    property var data: model
                    text: model.name
                    checkable: false
                    Layout.fillWidth: true
                    content: ComboBox{
                        id: combo
                        model: oceanuiData.monitors
                        currentIndex: oceanuiData.monitors.indexOf(touchItem.data.screenName)
                        onCurrentIndexChanged: {
                            if (touchItem.data.screenName !== oceanuiData.monitors[currentIndex]) {
                                oceanuiData.assoiateTouch(oceanuiData.monitors[currentIndex],touchItem.data.UUID)
                            }
                        }
                    }
                }
            }
        }
    }
}
