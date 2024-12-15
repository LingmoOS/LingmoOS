// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// import org.lingmo.dtk 1.0 as D
import QtQuick 2.15
import QtQuick.Controls 2.0

import org.lingmo.oceanui 1.0
import QtQuick.Layouts 1.15
import org.lingmo.dtk 1.0

OceanUIObject{
    OceanUIObject {
        name: "blueToothSwitch"
        parentName: "blueToothCtl" + model.name
        displayName: model.name
        pageType: OceanUIObject.Item
        weight: 10
        backgroundType: OceanUIObject.Normal
        page: RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            height: 60
            id: root

            DciIcon {
                id: deviceIcon
                Layout.leftMargin: 10
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                name: "qrc:/icons/lingmo/builtin/icons/bluetoothNomal"
                sourceSize:  Qt.size(36, 36)
            }

            Column {
                spacing: 0
                Label {
                    width: 100
                    id: myDeviceName
                    height: 25
                    text: oceanuiObj.displayName
                    font: DTK.fontManager.t6
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignBottom
                    leftPadding: 0
                }

                Row {
                    width: root.width - deviceSwitch.width - 36 - 52
                    spacing: 5
                    Label {
                        id: nameDetail
                        height: 25
                        width: Math.min(implicitWidth, root.width - deviceSwitch.width - 36 - 52)
                        text: model.nameDetail
                        horizontalAlignment: Qt.AlignLeft
                        verticalAlignment: Qt.AlignTop
                        font.pointSize: 8
                        color:"#5A000000"
                        elide: Text.ElideRight
                    }

                    ToolButton {
                        id: editBtn
                        flat: false
                        height: 25
                        font.pointSize: 8
                        text: qsTr("Edit")
                        checked: true
                        spacing: 0
                        topPadding: -5
                        onClicked: {
                            nameEdit.visible = true
                            nameDetail.visible = false
                            myDeviceName.visible = false
                            nameEditBackgrd.visible = true
                            nameEdit.forceActiveFocus(true)
                            nameEdit.selectAll()
                            editBtn.visible = false
                        }
                    }
                }

                Rectangle {
                    id: nameEditBackgrd
                    width: root.width - deviceSwitch.width - 36 - 52
                    height: 50
                    color: "transparent"
                    visible: false
                    LineEdit {
                        id: nameEdit
                        visible: false
                        anchors.centerIn: parent
                        width: root.width - deviceSwitch.width - 36 - 52
                        height: 30
                        text: myDeviceName.text
                        topPadding: 5
                        bottomPadding: 5

                        onTextChanged: {
                            if (text.length > 32) {
                                text = text.substr(0, 32);  // 截断到31个字符
                                nameEdit.alertText = qsTr("Length greater than or equal to 32")
                                nameEdit.showAlert = true
                            } else {
                                nameEdit.showAlert = false
                            }
                        }

                        // background: Rectangle {
                        //     color: "transparent" // 设置为透明
                        //     border.color: nameEdit.selectionColor // 边框颜色设置为蓝色
                        //     border.width: 2      // 边框宽度
                        //     radius: 5            // 可选：设置圆角
                        // }

                        onEditingFinished: {
                            nameEdit.visible = false
                            nameDetail.visible = true
                            myDeviceName.visible = true
                            nameEditBackgrd.visible = false
                            editBtn.visible = true

                            oceanuiData.work().setAdapterAlias(model.id, nameEdit.text)
                        }
                        Keys.onPressed: {
                            if (event.key === Qt.Key_Return) {
                                nameEdit.forceActiveFocus(false); // 结束编辑
                            }
                        }
                    }
                }
            }

            BusyIndicator {
                id: initAnimation
                //    Layout.alignment: Qt.AlignRight
                running: true
                visible: false
                implicitWidth: 32
                implicitHeight: 32
            }


            Timer {
                id: timer
                interval: 6000  // 1000毫秒，即1秒
                repeat: false    // 设置为重复
                running: false  // 初始状态不运行
                onTriggered: {

                    initAnimation.visible = false
                    deviceSwitch.enabled = true
                }

            }

            Switch {
                id: deviceSwitch
                checked: model.powered
                onCheckedChanged: {
                    if (checked === model.powered) {
                        return
                    }

                    oceanuiData.work().setAdapterPowered(model.id ,checked)
                    deviceSwitch.enabled = false
                    timer.running = true
                    initAnimation.visible = true
                    oceanuiData.work().setAdapterDiscovering(model.id, checked)

                    if (checked) {
                        oceanuiData.work().setAdapterDiscoverable(model.id)
                    }
                }
            }

        }
    }

    OceanUIObject {
        name: "blueToothSwitch"
        parentName: "blueToothCtl"  + model.name
        icon: "audio"
        pageType: OceanUIObject.Item
        weight: 20
        visible: !oceanuiData.model().airplaneEnable && model.powered

        page: CheckBox {
            checked: model.discoverabled
            leftPadding: 15
            text: qsTr("Allow other Bluetooth devices to find this device")
            
            onCheckedChanged: {
                oceanuiData.work().setAdapterDiscoverable(model.id ,checked)
            }
        }
    }

    OceanUIObject {
        name: "airplaneModeTips"
        parentName: "blueToothCtl"  + model.name
        pageType: OceanUIObject.Item
        weight: 30
        visible: oceanuiData.model().airplaneEnable
        page: Label {
            height: 25
            text: qsTr("To use the Bluetooth function, please turn off") + "<a href=\"飞行模式\">" + qsTr("Airplane Mode") +"</a>"
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignTop
            font.pointSize: 8
            color:"#5A000000"
            // 超链接点击事件
            onLinkActivated: function(url) {
                console.log("点击的链接是: " + url)
                oceanuiData.work().jumpToAirPlaneMode()
            }
        }
    }
}
