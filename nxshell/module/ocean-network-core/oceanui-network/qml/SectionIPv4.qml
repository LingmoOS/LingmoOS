// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0
import "NetUtils.js" as NetUtils

OceanUIObject {
    id: root
    property var config: null
    property int type: NetType.WiredItem
    property var ipItems: []
    property var addressData: []
    property bool isEdit: false
    property string method: "auto"

    property string errorKey: ""
    signal editClicked

    function setConfig(c) {
        errorKey = ""
        if (c === undefined) {
            root.config = {}
            method = "auto"
            addressData = []
        } else {
            root.config = c
            method = root.config.method
            resetAddressData()
        }
    }
    function getConfig() {
        let sConfig = root.config
        sConfig["method"] = method
        if (method === "manual") {
            let tmpIpData = []
            for (let ipData of addressData) {
                let ip = NetUtils.ipToNum(ipData[0])
                let prefix = NetUtils.ipToPrefix(ipData[1])
                let gateway = NetUtils.ipToNum(ipData[2])
                tmpIpData.push([ip, prefix, gateway])
            }
            sConfig["addresses"] = tmpIpData
        }

        return sConfig
    }
    function checkInput() {
        errorKey = ""
        if (method === "manual") {
            for (let k in addressData) {
                if (!NetUtils.ipRegExp.test(addressData[k][0])) {
                    errorKey = k + "address"
                    return false
                }
                if (!NetUtils.maskRegExp.test(addressData[k][1])) {
                    errorKey = k + "prefix"
                    return false
                }
                if (addressData[k][2].length !== 0 && !NetUtils.ipRegExp.test(addressData[k][2])) {
                    errorKey = k + "gateway"
                    return false
                }
            }
        }
        return true
    }

    function resetAddressData() {
        if (method === "manual") {
            let tmpIpData = []
            if (root.config.hasOwnProperty("addresses")) {
                for (let ipData of root.config["addresses"]) {
                    let ip = NetUtils.numToIp(ipData[0])
                    let prefix = NetUtils.prefixToIp(ipData[1])
                    let gateway = ipData[2] === 0 ? "" : NetUtils.numToIp(ipData[2])
                    tmpIpData.push([ip, prefix, gateway])
                }
            }
            addressData = tmpIpData
            if (addressData.length === 0) {
                addressData.push(["0.0.0.0", "255.255.255.0", ""])
                addressDataChanged()
            }
        } else {
            addressData = []
        }
    }

    name: "ipv4Title"
    displayName: qsTr("IPv4")
    pageType: OceanUIObject.Item
    page: RowLayout {
        Label {
            text: oceanuiObj.displayName
            font: OceanUIUtils.copyFont(D.DTK.fontManager.t4, {
                                        "bold": true
                                    })
        }
        Item {
            Layout.fillWidth: true
        }
        Label {
            visible: root.method === "manual"
            text: isEdit ? qsTr("Done") : qsTr("Edit")
            color: palette.link
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    isEdit = !isEdit
                }
            }
        }
    }
    ListModel {
        id: allModel
        ListElement {
            value: "disabled"
            text: qsTr("Disabled")
        }
        ListElement {
            value: "manual"
            text: qsTr("Manual")
        }
        ListElement {
            value: "auto"
            text: qsTr("Auto")
        }
    }
    ListModel {
        id: vpnModel
        ListElement {
            value: "disabled"
            text: qsTr("Disabled")
        }
        ListElement {
            value: "auto"
            text: qsTr("Auto")
        }
    }
    OceanUIObject {
        name: "ipv4Type"
        parentName: root.parentName
        weight: root.weight + 20
        backgroundType: OceanUIObject.Normal
        displayName: qsTr("Method")
        pageType: OceanUIObject.Editor
        page: ComboBox {
            textRole: "text"
            valueRole: "value"
            currentIndex: indexOfValue(root.method)
            onActivated: {
                root.method = currentValue
                resetAddressData()
                root.editClicked()
            }
            model: type === NetType.VPNControlItem ? vpnModel : allModel
            Component.onCompleted: {
                currentIndex = indexOfValue(method)
                isEdit = false
                resetAddressData()
            }
            Connections {
                target: root
                function onConfigChanged() {
                    currentIndex = indexOfValue(root.method)
                }
            }
        }
    }
    OceanUIObject {
        name: "ipv4Never"
        parentName: root.parentName
        weight: root.weight + 90
        visible: type === NetType.VPNControlItem
        displayName: qsTr("Only applied in corresponding resources")
        pageType: OceanUIObject.Item
        page: D.CheckBox {
            text: oceanuiObj.displayName
            checked: root.config && root.config.hasOwnProperty("never-default") ? root.config["never-default"] : false
            onClicked: {
                root.config["never-default"] = checked
                root.editClicked()
            }
        }
    }
    Component {
        id: ipComponent
        OceanUIObject {
            id: ipv4Item
            property int index: 0
            weight: root.weight + 30 + index
            name: "ipv4_" + index
            displayName: "IP-" + index
            parentName: root.parentName
            pageType: OceanUIObject.Item
            page: OceanUIGroupView {
                isGroup: false
            }
            OceanUIObject {
                name: "ipTitel"
                parentName: ipv4Item.parentName + "/" + ipv4Item.name
                weight: 10
                displayName: "IP-" + index
                pageType: OceanUIObject.Item
                page: RowLayout {
                    Label {
                        text: oceanuiObj.displayName
                        font: OceanUIUtils.copyFont(D.DTK.fontManager.t6, {
                                                    "bold": true
                                                })
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    D.IconLabel {
                        Layout.margins: 0
                        Layout.maximumHeight: 16
                        visible: isEdit
                        icon {
                            name: "list_add"
                            width: 16
                            height: 16
                        }
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                root.addressData.push(["0.0.0.0", "255.255.255.0", ""])
                                root.addressDataChanged()
                            }
                        }
                    }
                    D.IconLabel {
                        Layout.margins: 0
                        Layout.maximumHeight: 16
                        visible: isEdit && root.addressData.length > 1
                        icon {
                            name: "list_delete"
                            width: 16
                            height: 16
                        }
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                root.addressData.splice(index, 1)
                                root.addressDataChanged()
                            }
                        }
                    }
                }
            }
            OceanUIObject {
                id: ipGroup
                name: "ipGroup"
                parentName: ipv4Item.parentName + "/" + ipv4Item.name
                weight: 20
                pageType: OceanUIObject.Item
                page: OceanUIGroupView {}
                OceanUIObject {
                    name: "address"
                    parentName: ipGroup.parentName + "/" + ipGroup.name
                    weight: 10
                    displayName: qsTr("IP Address")
                    pageType: OceanUIObject.Editor
                    page: D.LineEdit {
                        text: addressData.length > index ? addressData[index][0] : "0.0.0.0"
                        validator: RegularExpressionValidator {
                            regularExpression: NetUtils.ipRegExp
                        }
                        onTextChanged: {
                            if (showAlert) {
                                errorKey = ""
                            }
                            if (addressData.length > index && addressData[index][0] !== text) {
                                addressData[index][0] = text
                                root.editClicked()
                            }
                        }
                        showAlert: errorKey === index + oceanuiObj.name
                        alertDuration: 2000
                        alertText: qsTr("Invalid IP address")
                        onShowAlertChanged: {
                            if (showAlert) {
                                OceanUIApp.showPage(oceanuiObj)
                                forceActiveFocus()
                            }
                        }
                    }
                }
                OceanUIObject {
                    name: "prefix"
                    parentName: ipGroup.parentName + "/" + ipGroup.name
                    weight: 20
                    displayName: qsTr("Netmask")
                    pageType: OceanUIObject.Editor
                    page: D.LineEdit {
                        text: addressData.length > index ? addressData[index][1] : "255.255.255.0"
                        validator: RegularExpressionValidator {
                            regularExpression: NetUtils.maskRegExp
                        }
                        onTextChanged: {
                            if (showAlert) {
                                errorKey = ""
                            }
                            if (addressData.length > index && addressData[index][1] !== text) {
                                addressData[index][1] = text
                                root.editClicked()
                            }
                        }
                        showAlert: errorKey === index + oceanuiObj.name
                        alertDuration: 1
                        onShowAlertChanged: {
                            if (showAlert) {
                                OceanUIApp.showPage(oceanuiObj)
                                forceActiveFocus()
                            }
                        }
                    }
                }
                OceanUIObject {
                    name: "gateway"
                    parentName: ipGroup.parentName + "/" + ipGroup.name
                    weight: 30
                    displayName: qsTr("Gateway")
                    pageType: OceanUIObject.Editor
                    page: D.LineEdit {
                        enabled: index === 0
                        text: index === 0 && addressData.length > index ? addressData[index][2] : ""
                        validator: RegularExpressionValidator {
                            regularExpression: NetUtils.ipRegExp
                        }
                        onTextChanged: {
                            if (showAlert) {
                                errorKey = ""
                            }
                            if (addressData.length > index && addressData[index][2] !== text) {
                                addressData[index][2] = text
                                root.editClicked()
                            }
                        }
                        showAlert: errorKey === index + oceanuiObj.name
                        alertDuration: 2000
                        onShowAlertChanged: {
                            if (showAlert) {
                                OceanUIApp.showPage(oceanuiObj)
                                forceActiveFocus()
                            }
                        }
                    }
                }
            }
        }
    }
    onAddressDataChanged: {
        while (addressData.length > ipItems.length) {
            let ipItem = ipComponent.createObject(root, {
                                                      "index": ipItems.length
                                                  })
            OceanUIApp.addObject(ipItem)
            ipItems.push(ipItem)
        }
        while (addressData.length < ipItems.length) {
            let tmpItem = ipItems.pop()
            OceanUIApp.removeObject(tmpItem)
            tmpItem.destroy()
        }
    }
}
