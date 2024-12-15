// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.5

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0

OceanUIObject {
    id: root
    property var config: null
    property var item: null
    property int type: NetType.WiredItem
    property bool modified: false
    signal finished
    readonly property string parentUrl: parentName + "/" + name

    weight: 10
    page: OceanUISettingsView {}
    OceanUIObject {
        name: "body"
        parentName: root.parentUrl
        weight: 10
        pageType: OceanUIObject.Item
        OceanUITitleObject {
            name: "mainTitle"
            parentName: root.parentUrl + "/body"
            weight: 10
            displayName: sectionGeneric.settingsID + qsTr(" Network Properties")
        }
        SectionGeneric {
            id: sectionGeneric
            parentName: root.parentUrl + "/body"
            weight: 100
            onEditClicked: modified = true
        }
        SectionPPPOE {
            id: sectionPPPOE
            parentName: root.parentUrl + "/body"
            weight: 200
            onEditClicked: modified = true
        }
        SectionIPv4 {
            id: sectionIPv4
            parentName: root.parentUrl + "/body"
            weight: 300
            onEditClicked: modified = true
        }
        SectionDNS {
            id: sectionDNS
            parentName: root.parentUrl + "/body"
            weight: 400
            onEditClicked: modified = true
        }
        SectionDevice {
            id: sectionDevice
            parentName: root.parentUrl + "/body"
            weight: 500
            canNotBind: false
            onEditClicked: modified = true
        }
        SectionPPP {
            id: sectionPPP
            parentName: root.parentUrl + "/body"
            weight: 600
            onEditClicked: modified = true
        }
    }
    onConfigChanged: {
        sectionGeneric.setConfig(config.connection)
        sectionPPPOE.setConfig(config["pppoe"])
        sectionIPv4.setConfig(config.ipv4)
        sectionDNS.setConfig((config.hasOwnProperty("ipv4") && config.ipv4.hasOwnProperty("dns")) ? config.ipv4.dns : null)
        sectionDevice.setConfig(config["802-3-ethernet"])
        sectionPPP.setConfig(config["ppp"])
        modified = config.connection.uuid === "{00000000-0000-0000-0000-000000000000}"
    }

    OceanUIObject {
        name: "footer"
        parentName: root.parentUrl
        weight: 20
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "delete"
            parentName: root.parentUrl + "/footer"
            weight: 10
            pageType: OceanUIObject.Item
            visible: config && config.connection.uuid !== "{00000000-0000-0000-0000-000000000000}"
            page: NetButton {
                contentItem: D.IconLabel {
                    text: qsTr("Delete")
                    color: "red"
                }
                onClicked: {
                    deleteDialog.createObject(this).show()
                }
            }
        }
        Component {
            id: deleteDialog
            D.DialogWindow {
                modality: Qt.ApplicationModal
                width: 380
                icon: "preferences-system"
                onClosing: destroy(10)
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.fillWidth: true
                        Layout.leftMargin: 50
                        Layout.rightMargin: 50
                        text: qsTr("Are you sure you want to delete this configuration?")
                        font.bold: true
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }
                    RowLayout {
                        Layout.topMargin: 10
                        Layout.bottomMargin: 10
                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Cancel")
                            onClicked: close()
                        }
                        Rectangle {
                            implicitWidth: 2
                            Layout.fillHeight: true
                            color: this.palette.button
                        }

                        D.Button {
                            Layout.fillWidth: true
                            contentItem: D.IconLabel {
                                text: qsTr("Delete")
                                color: "red"
                            }
                            onClicked: {
                                close()
                                oceanuiData.exec(NetManager.DeleteConnect, item.id, {
                                                 "uuid": config.connection.uuid
                                             })
                                root.finished()
                            }
                        }
                    }
                }
            }
        }

        OceanUIObject {
            name: "spacer"
            parentName: root.parentUrl + "/footer"
            weight: 20
            pageType: OceanUIObject.Item
            page: Item {
                Layout.fillWidth: true
            }
        }
        OceanUIObject {
            name: "cancel"
            parentName: root.parentUrl + "/footer"
            weight: 30
            pageType: OceanUIObject.Item
            page: NetButton {
                text: qsTr("Cancel")
                onClicked: {
                    root.finished()
                }
            }
        }
        OceanUIObject {
            name: "save"
            parentName: root.parentUrl + "/footer"
            weight: 40
            enabled: root.modified
            pageType: OceanUIObject.Item
            page: NetButton {
                text: qsTr("Save")
                onClicked: {
                    if (!sectionGeneric.checkInput() || !sectionPPPOE.checkInput() || !sectionIPv4.checkInput() || !sectionDNS.checkInput() || !sectionDevice.checkInput() || !sectionPPP.checkInput()) {
                        return
                    }

                    let nConfig = {}
                    nConfig["connection"] = sectionGeneric.getConfig()
                    nConfig["pppoe"] = sectionPPPOE.getConfig()
                    nConfig["ipv4"] = sectionIPv4.getConfig()
                    if (nConfig["ipv4"] === undefined) {
                        nConfig["ipv4"] = {}
                    }
                    nConfig["ipv4"]["dns"] = sectionDNS.getConfig()
                    let devConfig = sectionDevice.getConfig()
                    if (devConfig.interfaceName.length !== 0) {
                        nConfig["pppoe"]["parent"] = devConfig.interfaceName
                    }
                    nConfig["connection"]["interface-name"] = devConfig.interfaceName
                    nConfig["802-3-ethernet"] = devConfig
                    nConfig["ppp"] = sectionPPP.getConfig()
                    if (item) {
                        oceanuiData.exec(NetManager.SetConnectInfo, item.id, nConfig)
                    } else {
                        oceanuiData.exec(NetManager.SetConnectInfo, "", nConfig)
                    }
                    root.modified = false
                    root.finished()
                }
            }
        }
    }
}
