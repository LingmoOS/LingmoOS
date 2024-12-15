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

    visible: item
    displayName: qsTr("VPN")
    description: qsTr("Connect, add, import")
    icon: "oceanui_vpn"
    page: OceanUISettingsView {}
    Component {
        id: devCheck
        D.Switch {
            checked: item.isEnabled
            enabled: item.enabledable
            onClicked: {
                oceanuiData.exec(item.isEnabled ? NetManager.DisabledDevice : NetManager.EnabledDevice, item.id, {})
            }
        }
    }

    OceanUIObject {
        name: "body"
        parentName: root.name
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "title"
            parentName: root.name + "/body"
            displayName: root.displayName
            icon: "oceanui_vpn"
            weight: 10
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: devCheck
        }
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
            PageVPNSettings {
                id: vpnSettings
                name: "vpnSettings"
                parentName: root.name + "/body/networkList"
                onFinished: OceanUIApp.showPage(root)
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
                                if (tmpItem.itemType === NetType.VPNControlItem) {
                                    vpnSettings.displayName = qsTr("Add VPN")
                                } else {
                                    vpnSettings.displayName = tmpItem.name
                                }
                                vpnSettings.item = tmpItem
                                vpnSettings.setConfig(param)

                                OceanUIApp.showPage(vpnSettings)
                                break
                            }
                            for (let i in tmpItem.children) {
                                items.push(tmpItem.children[i])
                            }
                            items.shift()
                        }
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
            name: "spacer"
            parentName: root.name + "/footer"
            weight: 20
            pageType: OceanUIObject.Item
            page: Item {
                Layout.fillWidth: true
            }
        }
        OceanUIObject {
            name: "importVPN"
            parentName: root.name + "/footer"
            weight: 30
            pageType: OceanUIObject.Item
            page: NetButton {
                text: qsTr("Import VPN")
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    importFileDialog.createObject(this).open()
                }
            }
            Connections {
                target: oceanuiData
                function onRequest(cmd, id, param) {
                    if (cmd === NetManager.ImportError && id === root.item.id) {
                        importErrorDialog.createObject(this).show()
                    }
                }
            }

            Component {
                id: importFileDialog
                FileDialog {
                    visible: false
                    nameFilters: [qsTr("*.conf")]
                    onAccepted: {
                        oceanuiData.exec(NetManager.ImportConnect, item.id, {
                                         "file": currentFile.toString().replace("file://", "")
                                     })
                        this.destroy(10)
                    }
                    onRejected: this.destroy(10)
                }
            }
            Component {
                id: importErrorDialog
                D.DialogWindow {
                    modality: Qt.ApplicationModal
                    width: 380
                    icon: "dialog-error"
                    onClosing: this.destroy(10)
                    ColumnLayout {
                        width: parent.width
                        Label {
                            Layout.fillWidth: true
                            Layout.leftMargin: 50
                            Layout.rightMargin: 50
                            text: qsTr("Import Error")
                            font.bold: true
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Label {
                            Layout.fillWidth: true
                            Layout.leftMargin: 50
                            Layout.rightMargin: 50
                            text: qsTr("File error")
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Button {
                            Layout.fillWidth: true
                            Layout.margins: 10
                            text: qsTr("OK")
                            onClicked: close()
                        }
                    }
                }
            }
        }
        OceanUIObject {
            name: "addVPN"
            parentName: root.name + "/footer"
            weight: 40
            pageType: OceanUIObject.Item
            page: NetButton {
                text: qsTr("Add VPN")
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    oceanuiData.exec(NetManager.ConnectInfo, item.id, {})
                }
            }
        }
    }
}
