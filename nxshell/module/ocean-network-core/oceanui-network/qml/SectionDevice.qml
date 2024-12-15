// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0
import "NetUtils.js" as NetUtils

OceanUITitleObject {
    id: root
    property var config: new Object
    property int type: NetType.WiredItem
    property bool canNotBind: true
    property var devItems: []
    property var devData: []
    property bool hasMTU: false
    property string interfaceName: ""

    property string errorKey: ""
    signal editClicked

    function setConfig(c) {
        errorKey = ""
        if (c === undefined) {
            root.config = {}
        } else {
            root.config = c
        }

        hasMTU = root.config.hasOwnProperty("mtu")
        hasMTUChanged()
        if (canNotBind) {
            if (devModel.count > 1) {
                devModel.remove(1, devModel.count - 1)
            }
        } else {
            devModel.clear()
        }
        if (root.config.hasOwnProperty("optionalDevice")) {
            for (let dev of root.config.optionalDevice) {
                devModel.append({
                                    "text": dev,
                                    "value": dev.split(' ')[0]
                                })
            }
        }
        root.config["mac-address"] = root.config.hasOwnProperty("mac-address") ? NetUtils.macToStr(root.config["mac-address"]) : ""
        if (root.config.hasOwnProperty("cloned-mac-address")) {
            root.config["cloned-mac-address"] = NetUtils.macToStr(root.config["cloned-mac-address"])
        }
    }
    function getConfig() {
        let saveConfig = root.config ? root.config : {}
        saveConfig["interfaceName"] = interfaceName
        if (root.config.hasOwnProperty("mac-address") && root.config["mac-address"] !== "") {
            saveConfig["mac-address"] = NetUtils.strToMac(root.config["mac-address"])
        }
        if (root.config.hasOwnProperty("cloned-mac-address")) {
            saveConfig["cloned-mac-address"] = NetUtils.strToMac(root.config["cloned-mac-address"])
        }
        if (hasMTU) {
            saveConfig["mtu"] = root.config["mtu"]
        } else {
            delete saveConfig["mtu"]
        }

        saveConfig["band"] = root.config["band"]
        return saveConfig
    }
    function checkInput() {
        errorKey = ""
        if (root.config.hasOwnProperty("cloned-mac-address") && !NetUtils.macRegExp.test(root.config["cloned-mac-address"])) {
            errorKey = "cloned-mac-address"
            console.log(errorKey, root.config[errorKey])
            return false
        }

        return true
    }
    onErrorKeyChanged: console.log("dev errorKey", errorKey)
    ListModel {
        id: devModel
        ListElement {
            text: qsTr("Not Bind")
            value: ""
        }
    }
    name: "devTitle"
    displayName: type === NetType.WirelessItem ? qsTr("WLAN") : qsTr("Ethernet")
    OceanUIObject {
        name: "devGroup"
        parentName: root.parentName
        weight: root.weight + 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "mac-address"
            parentName: root.parentName + "/devGroup"
            weight: 20
            displayName: qsTr("Device MAC Addr")
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: root.config.hasOwnProperty("mac-address") ? indexOfValue(root.config["mac-address"]) : 0
                model: devModel
                onActivated: {
                    root.config["mac-address"] = currentValue
                    let name = /\((\w+)\)/.exec(currentText)
                    interfaceName = (name && name.length > 1) ? name[1] : ""
                    root.editClicked()
                }
                Component.onCompleted: {
                    if (root.config.hasOwnProperty("mac-address")) {
                        currentIndex = indexOfValue(root.config["mac-address"])
                        let name = /\((\w+)\)/.exec(currentText)
                        interfaceName = (name && name.length > 1) ? name[1] : ""
                    }
                }
            }
        }
        OceanUIObject {
            name: "cloned-mac-address"
            parentName: root.parentName + "/devGroup"
            weight: 30
            displayName: qsTr("Cloned MAC Addr")
            visible: type === NetType.WiredItem
            pageType: OceanUIObject.Editor
            page: D.LineEdit {
                text: root.config.hasOwnProperty("cloned-mac-address") ? root.config["cloned-mac-address"] : ""
                validator: RegularExpressionValidator {
                    regularExpression: NetUtils.macRegExp
                }
                onTextChanged: {
                    if (showAlert) {
                        errorKey = ""
                    }
                    if (text.length === 0) {
                        delete root.config["cloned-mac-address"]
                        delete root.config["assigned-mac-address"]
                    } else if (root.config["cloned-mac-address"] !== text) {
                        root.config["cloned-mac-address"] = text
                        root.editClicked()
                    }
                }
                showAlert: errorKey === oceanuiObj.name
                alertDuration: 2000
                onShowAlertChanged: {
                    if (showAlert) {
                        OceanUIApp.showPage(oceanuiObj)
                        forceActiveFocus()
                    }
                }
            }
        }
        OceanUIObject {
            name: "customizeMTU"
            parentName: root.parentName + "/devGroup"
            weight: 40
            displayName: qsTr("Customize MTU")
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: hasMTU
                onClicked: {
                    hasMTU = checked
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "mtu"
            parentName: root.parentName + "/devGroup"
            weight: 50
            displayName: qsTr("MTU")
            visible: hasMTU
            pageType: OceanUIObject.Editor
            page: D.SpinBox {
                value: root.config.hasOwnProperty("mtu") ? root.config.mtu : 0
                onValueChanged: {
                    if (hasMTU && (!root.config.hasOwnProperty("mtu") || root.config.mtu !== value)) {
                        root.config.mtu = value
                        root.editClicked()
                    }
                }
            }
        }
    }
    // 配置在config[root.config.connection.type]中，显示在Generic中
    OceanUIObject {
        name: "band"
        parentName: root.parentName + "/genericGroup"
        displayName: qsTr("Band")
        weight: 30
        visible: type === NetType.WirelessItem
        pageType: OceanUIObject.Editor
        page: ComboBox {
            textRole: "text"
            valueRole: "value"
            model: [{
                    "text": qsTr("Auto"),
                    "value": undefined
                }, {
                    "text": qsTr("2.4 GHz"),
                    "value": "bg"
                }, {
                    "text": qsTr("5 GHz"),
                    "value": "a"
                }]
            currentIndex: indexOfValue(root.config["band"])
            onActivated: {
                root.config["band"] = currentValue
                root.editClicked()
            }
            Component.onCompleted: {
                currentIndex = indexOfValue(root.config["band"])
            }
        }
    }
}
