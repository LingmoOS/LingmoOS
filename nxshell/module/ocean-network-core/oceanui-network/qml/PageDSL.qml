// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.qmlmodels 1.2

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0

OceanUIObject {
    id: root
    property var item: null

    displayName: qsTr("DSL")
    description: qsTr("Set up a dial-up network connection")
    icon: "oceanui_dsl"
    visible: item
    page: OceanUISettingsView {}

    OceanUIObject {
        name: "body"
        parentName: root.name
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "networkList"
            parentName: root.name + "/body"
            weight: 20
            pageType: OceanUIObject.Item
            backgroundType: OceanUIObject.Normal
            page: ColumnLayout {
                clip: true
                spacing: 0
                Repeater {
                    id: repeater
                    model: NetItemModel {
                        root: item
                    }

                    delegate: ItemDelegate {
                        id: itemDelegate
                        implicitHeight: 36
                        text: model.item.name
                        hoverEnabled: true
                        checked: true
                        backgroundVisible: false
                        corners: getCornersForBackground(index, repeater.count)
                        cascadeSelected: true
                        Layout.fillWidth: true
                        content: RowLayout {
                            BusyIndicator {
                                running: model.item.status === NetType.CS_Connecting
                                visible: running
                            }
                            OceanUICheckIcon {
                                visible: model.item.status === NetType.CS_Connected && !itemDelegate.hovered
                            }
                            NetButton {
                                visible: model.item.status !== NetType.CS_Connecting && itemDelegate.hovered
                                text: model.item.status === NetType.CS_Connected ? qsTr("Disconnect") : qsTr("Connect")
                                Layout.alignment: Qt.AlignCenter
                                onClicked: {
                                    oceanuiData.exec(model.item.status === NetType.CS_Connected ? NetManager.Disconnect : NetManager.ConnectOrInfo, model.item.id, {})
                                }
                            }
                            D.IconLabel {
                                icon.name: "arrow_ordinary_right"
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.LeftButton
                                    onClicked: {
                                        oceanuiData.exec(NetManager.ConnectInfo, model.item.id, {})
                                    }
                                }
                            }
                        }
                        onDoubleClicked: {
                            if (model.item.status === NetType.CS_UnConnected) {
                                oceanuiData.exec(NetManager.ConnectOrInfo, model.item.id, {})
                            }
                        }
                        background: OceanUIItemBackground {
                            separatorVisible: true
                        }
                    }
                }
            }
        }
        PageDSLSettings {
            id: dslSettings
            name: "dslSettings"
            parentName: root.name + "/body/networkList"
            onFinished: OceanUIApp.showPage(root)
            type: NetType.WiredItem
            Connections {
                target: oceanuiData
                function onRequest(cmd, id, param) {
                    if (cmd !== NetManager.ConnectInfo) {
                        return
                    }
                    const items = new Array(root.item)
                    while (items.length !== 0) {
                        let tmpItem = items[0]
                        if (tmpItem.id === id) {
                            if (tmpItem.itemType === NetType.DSLControlItem) {
                                dslSettings.displayName = qsTr("Add PPPoE connection")
                            } else {
                                dslSettings.displayName = tmpItem.name
                            }
                            dslSettings.item = tmpItem
                            dslSettings.config = param

                            OceanUIApp.showPage(dslSettings)
                            break
                        }
                        for (let i in tmpItem.children) {
                            items.push(tmpItem.children[i])
                        }
                        // items=items.concat(tmpItem.children)
                        items.shift()
                    }
                }
            }
        }
    }
    OceanUIObject {
        name: "footer"
        parentName: root.name
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "addDSL"
            parentName: root.name + "/footer"
            weight: 40
            pageType: OceanUIObject.Item
            page: NetButton {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Add PPPoE connection")
                onClicked: {
                    oceanuiData.exec(NetManager.ConnectInfo, item.id, {})
                }
            }
        }
    }
}
