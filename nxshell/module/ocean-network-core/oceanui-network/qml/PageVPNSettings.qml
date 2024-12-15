// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.5
import Qt.labs.platform 1.1

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0
import "NetUtils.js" as NetUtils

OceanUIObject {
    id: root
    property var config: null
    property var item: null
    property int vpnType: NetUtils.VpnTypeEnum["l2tp"]
    property bool modified: false
    signal finished
    readonly property string parentUrl: parentName + "/" + name

    function setConfig(c) {
        if (c === undefined) {
            return
        }
        config = c
        vpnType = NetUtils.toVpnTypeEnum(config.vpn["service-type"])
        sectionGeneric.setConfig(config.connection)
        sectionVPN.setConfig(config["vpn"])
        sectionIPv4.setConfig(config.ipv4)
        sectionIPv6.setConfig(config.ipv6)
        sectionDNS.setConfig((config.hasOwnProperty("ipv4") && config.ipv4.hasOwnProperty("dns")) ? config.ipv4.dns : null)
        modified = config.connection.uuid === "{00000000-0000-0000-0000-000000000000}"
    }

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
        OceanUIObject {
            name: "vpnType"
            parentName: root.parentUrl + "/body"
            weight: 50
            visible: config && config.connection.uuid === "{00000000-0000-0000-0000-000000000000}"
            displayName: qsTr("VPN Type")
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: indexOfValue(vpnType)
                model: [{
                        "text": qsTr("L2TP"),
                        "value": NetUtils.VpnTypeEnum["l2tp"]
                    }, {
                        "text": qsTr("PPTP"),
                        "value": NetUtils.VpnTypeEnum["pptp"]
                    }, {
                        "text": qsTr("OpenVPN"),
                        "value": NetUtils.VpnTypeEnum["openvpn"]
                    }, {
                        "text": qsTr("OpenConnect"),
                        "value": NetUtils.VpnTypeEnum["openconnect"]
                    }, {
                        "text": qsTr("StrongSwan"),
                        "value": NetUtils.VpnTypeEnum["strongswan"]
                    }, {
                        "text": qsTr("VPNC"),
                        "value": NetUtils.VpnTypeEnum["vpnc"]
                    }]
                onActivated: {
                    vpnType = currentValue
                    if (config.hasOwnProperty("optionalName")) {
                        let key = NetUtils.toVpnKey(vpnType).substring(31)
                        config.connection.id = config.optionalName[key]
                        sectionGeneric.setConfig(config.connection)
                    }
                }
                Component.onCompleted: currentIndex = indexOfValue(vpnType)
                Connections {
                    target: root
                    function onVpnTypeChanged() {
                        currentIndex = indexOfValue(vpnType)
                    }
                }
            }
        }

        SectionGeneric {
            id: sectionGeneric
            parentName: root.parentUrl + "/body"
            weight: 100
            onEditClicked: modified = true
        }
        SectionVPN {
            id: sectionVPN
            vpnType: root.vpnType
            parentName: root.parentUrl + "/body"
            weight: 200
            onEditClicked: modified = true
        }
        SectionIPv4 {
            id: sectionIPv4
            type: NetType.VPNControlItem
            parentName: root.parentUrl + "/body"
            weight: 1000
            onEditClicked: modified = true
        }
        SectionIPv6 {
            id: sectionIPv6
            type: NetType.VPNControlItem
            parentName: root.parentUrl + "/body"
            visible: vpnType & (NetUtils.VpnTypeEnum["openvpn"] | NetUtils.VpnTypeEnum["openconnect"])
            weight: 1100
            onEditClicked: modified = true
        }
        SectionDNS {
            id: sectionDNS
            parentName: root.parentUrl + "/body"
            weight: 1200
            onEditClicked: modified = true
        }
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
            name: "export"
            parentName: root.parentUrl + "/footer"
            weight: 20
            visible: config && config.connection.uuid !== "{00000000-0000-0000-0000-000000000000}" && (vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["openvpn"]))
            pageType: OceanUIObject.Item
            page: NetButton {
                text: qsTr("Export")
                onClicked: {
                    exportFileDialog.createObject(this).open()
                }
            }
            Component {
                id: exportFileDialog
                FileDialog {
                    visible: false
                    fileMode: FileDialog.SaveFile
                    nameFilters: [qsTr("*.conf")]
                    currentFile: root.config.connection.id
                    onAccepted: {
                        oceanuiData.exec(NetManager.ExportConnect, item.id, {
                                         "file": currentFile.toString().replace("file://", "")
                                     })
                        this.destroy(10)
                    }
                    onRejected: this.destroy(10)
                }
            }
        }
        OceanUIObject {
            name: "spacer"
            parentName: root.parentUrl + "/footer"
            weight: 30
            pageType: OceanUIObject.Item
            page: Item {
                Layout.fillWidth: true
            }
        }
        OceanUIObject {
            name: "cancel"
            parentName: root.parentUrl + "/footer"
            weight: 40
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
            weight: 50
            enabled: root.modified
            pageType: OceanUIObject.Item
            page: NetButton {
                text: qsTr("Save")
                onClicked: {
                    if (!sectionGeneric.checkInput() || !sectionVPN.checkInput() || !sectionIPv4.checkInput() || !sectionIPv6.checkInput() || !sectionDNS.checkInput()) {
                        return
                    }

                    let nConfig = {}
                    nConfig["connection"] = sectionGeneric.getConfig()
                    nConfig["vpn"] = sectionVPN.getConfig()
                    nConfig["ipv4"] = sectionIPv4.getConfig()
                    nConfig["ipv6"] = sectionIPv6.getConfig()
                    if (nConfig["ipv4"] === undefined) {
                        nConfig["ipv4"] = {}
                    }
                    nConfig["ipv4"]["dns"] = sectionDNS.getConfig()
                    nConfig["vpn"]["service-type"] = NetUtils.toVpnKey(vpnType)
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
