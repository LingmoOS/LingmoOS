// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0
import "NetUtils.js" as NetUtils

OceanUITitleObject {
    id: root
    property var configWSecurity
    property var config802_1x
    property string parentName: ""
    property int type: NetType.WiredItem
    property string eapType: "peap"
    property int pwdFlays: 0
    property string pwdStr: ""
    property string keyMgmt: ""

    property string errorKey: ""
    signal editClicked

    function setConfigWSecurity(c) {
        errorKey = ""
        if (c) {
            root.configWSecurity = c
            root.keyMgmt = root.configWSecurity.hasOwnProperty("key-mgmt") ? root.configWSecurity["key-mgmt"] : ""
        } else {
            root.configWSecurity = {}
            root.keyMgmt = ""
        }
        switch (root.keyMgmt) {
        case "none":
            pwdStr = root.configWSecurity.hasOwnProperty("wep-key0") ? root.configWSecurity["wep-key0"] : ""
            root.pwdFlays = root.configWSecurity.hasOwnProperty("wep-key-type") ? root.configWSecurity["wep-key-type"] : 0
            break
        case "wpa-psk":
        case "sae":
            pwdStr = root.configWSecurity.hasOwnProperty("psk") ? root.configWSecurity["psk"] : ""
            root.pwdFlays = root.configWSecurity.hasOwnProperty("psk-flags") ? root.configWSecurity["psk-flags"] : 0
            break
        }
    }
    function getConfigWSecurity(c) {
        let config = {}
        switch (root.keyMgmt) {
        case "none":
            config["key-mgmt"] = root.keyMgmt
            config["wep-key0"] = pwdStr
            config["wep-key-type"] = root.pwdFlays
            config["auth-alg"] = root.configWSecurity["auth-alg"]
            break
        case "wpa-psk":
        case "sae":
            config["key-mgmt"] = root.keyMgmt
            config["psk"] = pwdStr
            config["psk-flags"] = root.pwdFlays
            break
        case "wpa-eap":
            config["key-mgmt"] = root.keyMgmt
            break
        default:
            return undefined
        }
        return config
    }

    function setConfig802_1x(c) {
        errorKey = ""
        if (root.keyMgmt === "" && c !== undefined) {
            root.keyMgmt = "wpa-eap"
        }
        switch (root.keyMgmt) {
        case "wpa-eap":
            root.config802_1x = c ? c : {}
            if (root.config802_1x && root.config802_1x.hasOwnProperty("eap") && root.config802_1x.eap.length > 0) {
                root.eapType = root.config802_1x["eap"][0]
                console.log("sss===root.eapType=", root.eapType)
            }
            switch (root.eapType) {
            case "tls":
                pwdStr = root.config802_1x.hasOwnProperty("private-key-password") ? root.config802_1x["private-key-password"] : ""
                root.pwdFlays = root.config802_1x.hasOwnProperty("private-key-password-flags") ? root.config802_1x["private-key-password-flags"] : 0
                break
            case "md5":
            case "leap":
            case "fast":
            case "ttls":
            case "peap":
                pwdStr = root.config802_1x.hasOwnProperty("password") ? root.config802_1x["password"] : ""
                root.pwdFlays = root.config802_1x.hasOwnProperty("password-flags") ? root.config802_1x["password-flags"] : 0
                break
            }
            break
        case "":
            // 无密码
            root.config802_1x = {}
            root.eapType = "tls"
            pwdStr = ""
            root.pwdFlays = 0
            break
        default:
            root.config802_1x = {}
            root.eapType = "tls"
            break
        }
    }
    function getConfig802_1x() {
        let saveConfig = {}
        if (root.keyMgmt === "wpa-eap") {
            saveConfig.eap = [root.eapType]
            switch (root.eapType) {
            case "tls":
                saveConfig["identity"] = root.config802_1x["identity"]
                saveConfig["private-key-password-flags"] = root.pwdFlays
                saveConfig["private-key-password"] = pwdStr
                // 文件类型需要以\0结尾
                if (root.config802_1x.hasOwnProperty("private-key") && root.config802_1x["private-key"] !== "") {
                    saveConfig["private-key"] = NetUtils.strToByteArray(root.config802_1x["private-key"])
                }
                if (root.config802_1x.hasOwnProperty("ca-cert") && root.config802_1x["ca-cert"] !== "") {
                    saveConfig["ca-cert"] = NetUtils.strToByteArray(root.config802_1x["ca-cert"])
                }
                if (root.config802_1x.hasOwnProperty("client-cert") && root.config802_1x["client-cert"] !== "") {
                    saveConfig["client-cert"] = NetUtils.strToByteArray(root.config802_1x["client-cert"])
                }
                break
            case "md5":
            case "leap":
                saveConfig["identity"] = root.config802_1x["identity"]
                saveConfig["password-flags"] = root.pwdFlays
                saveConfig["password"] = pwdStr
                break
            case "fast":
                saveConfig["identity"] = root.config802_1x["identity"]
                saveConfig["password-flags"] = root.pwdFlays
                saveConfig["password"] = pwdStr
                saveConfig["anonymous-identity"] = root.config802_1x["anonymous-identity"]
                saveConfig["phase1-fast-provisioning"] = root.config802_1x["phase1-fast-provisioning"]
                if (root.config802_1x.hasOwnProperty("pac-file") && root.config802_1x["pac-file"] !== "") {
                    saveConfig["pac-file"] = NetUtils.strToByteArray(root.config802_1x["pac-file"])
                }
                saveConfig["phase2-auth"] = root.config802_1x["phase2-auth"]
                break
            case "ttls":
                saveConfig["identity"] = root.config802_1x["identity"]
                saveConfig["password-flags"] = root.pwdFlays
                saveConfig["password"] = pwdStr
                saveConfig["anonymous-identity"] = root.config802_1x["anonymous-identity"]
                if (root.config802_1x.hasOwnProperty("ca-cert") && root.config802_1x["ca-cert"] !== "") {
                    saveConfig["ca-cert"] = NetUtils.strToByteArray(root.config802_1x["ca-cert"])
                }
                saveConfig["phase2-auth"] = root.config802_1x["phase2-auth"]
                break
            case "peap":
                saveConfig["identity"] = root.config802_1x["identity"]
                saveConfig["password-flags"] = root.pwdFlays
                saveConfig["password"] = pwdStr
                saveConfig["anonymous-identity"] = root.config802_1x["anonymous-identity"]
                if (root.config802_1x.hasOwnProperty("ca-cert") && root.config802_1x["ca-cert"] !== "") {
                    saveConfig["ca-cert"] = NetUtils.strToByteArray(root.config802_1x["ca-cert"])
                }
                saveConfig["phase1-peapver"] = root.config802_1x["phase1-peapver"]
                saveConfig["phase2-auth"] = root.config802_1x["phase2-auth"]
                break
            }
        } else {
            // delete saveConfig
            return undefined
        }

        return saveConfig
    }
    function checkInput() {
        errorKey = ""
        switch (root.keyMgmt) {
        case "none":
            if (root.pwdFlays !== 2 && !oceanuiData.CheckPasswordValid("wep-key0", pwdStr)) {
                errorKey = "password"
                return false
            }
            break
        case "wpa-psk":
        case "sae":
            if (root.pwdFlays !== 2 && !oceanuiData.CheckPasswordValid("psk", pwdStr)) {
                errorKey = "password"
                return false
            }
            break
        case "wpa-eap":
            console.log("identity", root.config802_1x["identity"], typeof root.config802_1x["identity"])
            if (!root.config802_1x["identity"] || root.config802_1x["identity"] === "") {
                errorKey = "identity"
                return false
            }
            if (root.pwdFlays !== 2 && !oceanuiData.CheckPasswordValid("password", pwdStr)) {
                errorKey = "password"
                return false
            }
            switch (root.eapType) {
            case "tls":
                if (!root.config802_1x["private-key"] || root.config802_1x["private-key"] === "") {
                    errorKey = "private-key"
                    return false
                }
                if (!root.config802_1x["client-cert"] || root.config802_1x["client-cert"] === "") {
                    errorKey = "client-cert"
                    return false
                }
                break
            default:
                break
            }
            break
        default:
            return true
        }
        return true
    }

    function removeTrailingNull(str) {
        return str ? str.toString().replace(/\0+$/, '') : ""
    }

    name: "secretTitle"
    displayName: qsTr("Security")

    ListModel {
        id: eapModelWired
        ListElement {
            text: qsTr("TLS")
            value: "tls"
        }
        ListElement {
            text: qsTr("MD5")
            value: "md5"
        }
        ListElement {
            text: qsTr("FAST")
            value: "fast"
        }
        ListElement {
            text: qsTr("Tunneled TLS")
            value: "ttls"
        }
        ListElement {
            text: qsTr("Protected EAP")
            value: "peap"
        }
    }
    ListModel {
        id: eapModelWireless
        ListElement {
            text: qsTr("TLS")
            value: "tls"
        }
        ListElement {
            text: qsTr("LEAP")
            value: "leap"
        }
        ListElement {
            text: qsTr("FAST")
            value: "fast"
        }
        ListElement {
            text: qsTr("Tunneled TLS")
            value: "ttls"
        }
        ListElement {
            text: qsTr("Protected EAP")
            value: "peap"
        }
    }
    ListModel {
        id: fastAuthModel
        ListElement {
            text: qsTr("GTC")
            value: "gtc"
        }
        ListElement {
            text: qsTr("MSCHAPV2")
            value: "mschapv2"
        }
    }
    ListModel {
        id: ttlsAuthModel
        ListElement {
            text: qsTr("PAP")
            value: "pap"
        }
        ListElement {
            text: qsTr("MSCHAP")
            value: "mschap"
        }
        ListElement {
            text: qsTr("MSCHAPV2")
            value: "mschapv2"
        }
        ListElement {
            text: qsTr("CHAP")
            value: "chap"
        }
    }
    ListModel {
        id: peapAuthModel
        ListElement {
            text: qsTr("GTC")
            value: "gtc"
        }
        ListElement {
            text: qsTr("MD5")
            value: "md5"
        }
        ListElement {
            text: qsTr("MSCHAPV2")
            value: "mschapv2"
        }
    }
    OceanUIObject {
        name: "secretGroup"
        parentName: root.parentName
        weight: root.weight + 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "securityType"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Security")
            weight: 10
            visible: root.type === NetType.WirelessItem
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("None"),
                        "value": ""
                    }, {
                        "text": qsTr("WEP"),
                        "value": "none"
                    }, {
                        "text": qsTr("WPA/WPA2 Personal"),
                        "value": "wpa-psk"
                    }, {
                        "text": qsTr("WPA/WPA2 Enterprise"),
                        "value": "wpa-eap"
                    }, {
                        "text": qsTr("WPA3 Personal"),
                        "value": "sae"
                    }]
                currentIndex: indexOfValue(root.keyMgmt)
                onActivated: {
                    root.keyMgmt = currentValue
                    root.editClicked()
                }
                Component.onCompleted: {
                    currentIndex = indexOfValue(root.keyMgmt)
                }
            }
        }
        OceanUIObject {
            name: "security"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Security")
            weight: 20
            visible: root.type === NetType.WiredItem
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.keyMgmt === "wpa-eap"
                onClicked: {
                    if ((root.keyMgmt === "wpa-eap") !== checked) {
                        root.keyMgmt = checked ? "wpa-eap" : ""
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "eapAuth"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("EAP Auth")
            weight: 30
            visible: root.keyMgmt === "wpa-eap"
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: indexOfValue(root.eapType)
                onActivated: {
                    root.eapType = currentValue
                    root.editClicked()
                }
                model: root.type === NetType.WiredItem ? eapModelWired : eapModelWireless
                Component.onCompleted: {
                    currentIndex = indexOfValue(root.eapType)
                }
            }
        }
        OceanUIObject {
            name: "identity"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Identity")
            weight: 40
            visible: root.keyMgmt === "wpa-eap"
            pageType: OceanUIObject.Editor
            page: D.LineEdit {
                placeholderText: qsTr("Required")
                text: root.config802_1x && root.config802_1x.hasOwnProperty("identity") ? root.config802_1x.identity : ""
                onTextChanged: {
                    if (showAlert) {
                        errorKey = ""
                    }
                    if (root.config802_1x.identity !== text) {
                        root.config802_1x.identity = text
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
            name: "pwdOptions"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Pwd Options")
            weight: 50
            visible: root.keyMgmt !== ""
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Save password for this user"),
                        "value": 1
                    }, {
                        "text": qsTr("Save password for all users"),
                        "value": 0
                    }, {
                        "text": qsTr("Ask me always"),
                        "value": 2
                    }]
                currentIndex: indexOfValue(root.pwdFlays)
                onActivated: {
                    root.pwdFlays = currentValue
                    root.editClicked()
                }
                Component.onCompleted: {
                    currentIndex = indexOfValue(root.pwdFlays)
                }
            }
        }
        OceanUIObject {
            name: "password"
            parentName: root.parentName + "/secretGroup"
            displayName: root.keyMgmt === "none" ? qsTr("Key") : (root.eapType === "tls" ? qsTr("Private Pwd") : qsTr("Password"))
            weight: 60
            visible: root.keyMgmt !== "" && root.pwdFlays !== 2
            pageType: OceanUIObject.Editor
            page: NetPasswordEdit {
                text: root.pwdStr
                dataItem: root
                onTextUpdated: root.pwdStr = text
            }
        }
        OceanUIObject {
            name: "Authentication"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Authentication")
            weight: 70
            visible: root.keyMgmt === "none"
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Shared key"),
                        "value": "shared"
                    }, {
                        "text": qsTr("Open system"),
                        "value": "open"
                    }]
                currentIndex: root.configWSecurity && root.configWSecurity.hasOwnProperty("auth-alg") ? indexOfValue(root.configWSecurity["auth-alg"]) : 0
                onActivated: {
                    root.configWSecurity["auth-alg"] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: {
                    currentIndex = root.configWSecurity && root.configWSecurity.hasOwnProperty("auth-alg") ? indexOfValue(root.configWSecurity["auth-alg"]) : 0
                }
            }
        }
        OceanUIObject {
            name: "private-key"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Private Key")
            weight: 80
            visible: root.keyMgmt === "wpa-eap" && root.eapType === "tls"
            pageType: OceanUIObject.Editor
            page: NetFileChooseEdit {
                dataItem: root
                placeholderText: qsTr("Required")
                text: root.config802_1x && root.config802_1x.hasOwnProperty("private-key") ? removeTrailingNull(root.config802_1x["private-key"]) : ""
                onTextUpdated: root.config802_1x["private-key"] = text
            }
        }
        OceanUIObject {
            name: "anonymousID"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Anonymous ID")
            weight: 90
            visible: root.keyMgmt === "wpa-eap" && (root.eapType === "fast" || root.eapType === "ttls" || root.eapType === "peap")
            pageType: OceanUIObject.Editor
            page: D.LineEdit {
                text: root.config802_1x && root.config802_1x.hasOwnProperty("anonymous-identity") ? root.config802_1x["anonymous-identity"] : ""
                onTextChanged: {
                    if (root.config802_1x["anonymous-identity"] !== text) {
                        root.config802_1x["anonymous-identity"] = text
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "provisioning"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Provisioning")
            weight: 100
            visible: root.keyMgmt === "wpa-eap" && root.eapType === "fast"
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: (root.config802_1x && root.config802_1x.hasOwnProperty("phase1-fast-provisioning")) ? indexOfValue(root.config802_1x["phase1-fast-provisioning"]) : 0
                onActivated: {
                    root.config802_1x["phase1-fast-provisioning"] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: {
                    currentIndex = (root.config802_1x && root.config802_1x.hasOwnProperty("phase1-fast-provisioning")) ? indexOfValue(root.config802_1x["phase1-fast-provisioning"]) : 0
                    if (!root.config802_1x.hasOwnProperty("phase1-fast-provisioning")) {
                        root.config802_1x["phase1-fast-provisioning"] = currentValue
                    }
                }
                model: [{
                        "text": qsTr("Disabled"),
                        "value": 0
                    }, {
                        "text": qsTr("Anonymous"),
                        "value": 1
                    }, {
                        "text": qsTr("Authenticated"),
                        "value": 2
                    }, {
                        "text": qsTr("Both"),
                        "value": 3
                    }]
            }
        }
        OceanUIObject {
            name: "pac-file"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("PAC file")
            visible: root.keyMgmt === "wpa-eap" && root.eapType === "fast"
            weight: 110
            pageType: OceanUIObject.Editor
            page: NetFileChooseEdit {
                dataItem: root
                text: root.config802_1x && root.config802_1x.hasOwnProperty(oceanuiObj.name) ? removeTrailingNull(root.config802_1x[oceanuiObj.name]) : ""
                onTextUpdated: root.config802_1x[oceanuiObj.name] = text
            }
        }
        OceanUIObject {
            name: "ca-cert"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("CA Cert")
            weight: 120
            visible: root.keyMgmt === "wpa-eap" && (root.eapType === "tls" || root.eapType === "ttls" || root.eapType === "peap")
            pageType: OceanUIObject.Editor
            page: NetFileChooseEdit {
                dataItem: root
                text: root.config802_1x && root.config802_1x.hasOwnProperty(oceanuiObj.name) ? removeTrailingNull(root.config802_1x[oceanuiObj.name]) : ""
                onTextUpdated: root.config802_1x[oceanuiObj.name] = text
            }
        }
        OceanUIObject {
            name: "client-cert"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("User Cert")
            weight: 130
            visible: root.keyMgmt === "wpa-eap" && root.eapType === "tls"
            pageType: OceanUIObject.Editor
            page: NetFileChooseEdit {
                dataItem: root
                placeholderText: qsTr("Required")
                text: root.config802_1x && root.config802_1x.hasOwnProperty(oceanuiObj.name) ? removeTrailingNull(root.config802_1x[oceanuiObj.name]) : ""
                onTextUpdated: root.config802_1x[oceanuiObj.name] = text
            }
        }
        OceanUIObject {
            name: "peapVersion"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("PEAP Version")
            weight: 140
            visible: root.keyMgmt === "wpa-eap" && root.eapType === "peap"
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: (root.config802_1x && root.config802_1x.hasOwnProperty("phase1-peapver")) ? indexOfValue(root.config802_1x["phase1-peapver"]) : 0
                onActivated: {
                    root.config802_1x["phase1-peapver"] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: {
                    currentIndex = (root.config802_1x && root.config802_1x.hasOwnProperty("phase1-peapver")) ? indexOfValue(root.config802_1x["phase1-peapver"]) : 0
                    if (!root.config802_1x.hasOwnProperty("phase1-peapver")) {
                        root.config802_1x["phase1-peapver"] = currentValue
                    }
                }
                model: [{
                        "text": qsTr("Automatic"),
                        "value": undefined
                    }, {
                        "text": qsTr("Version 0"),
                        "value": 0
                    }, {
                        "text": qsTr("Version 1"),
                        "value": 1
                    }]
            }
        }
        OceanUIObject {
            name: "innerAuth"
            parentName: root.parentName + "/secretGroup"
            displayName: qsTr("Inner Auth")
            weight: 150
            visible: root.keyMgmt === "wpa-eap" && (root.eapType === "fast" || root.eapType === "ttls" || root.eapType === "peap")
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: (root.config802_1x && root.config802_1x.hasOwnProperty("phase2-auth")) ? indexOfValue(root.config802_1x["phase2-auth"]) : 0
                onActivated: {
                    root.config802_1x["phase2-auth"] = currentValue
                    root.editClicked()
                }
                model: root.eapType === "peap" ? peapAuthModel : (root.eapType === "fast" ? fastAuthModel : ttlsAuthModel)
                Component.onCompleted: {
                    currentIndex = (root.config802_1x && root.config802_1x.hasOwnProperty("phase2-auth")) ? indexOfValue(root.config802_1x["phase2-auth"]) : 0
                    if (!root.config802_1x.hasOwnProperty("phase2-auth")) {
                        root.config802_1x["phase2-auth"] = currentValue
                    }
                }
            }
        }
    }
}
