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
    property var config: null
    property var dataMap: new Object
    property var secretMap: new Object
    property int vpnType: 1

    property string errorKey: ""
    signal editClicked

    // 控件变化
    property bool mppe: false
    property string currentMppeMethod: "require-mppe"
    property bool ipsecEnabled: false
    property string currentEncryption: "secure"

    function setConfig(c) {
        errorKey = ""
        root.config = c !== undefined ? c : {}
        root.dataMap = root.config.hasOwnProperty("data") ? oceanuiData.toMap(root.config.data) : {}
        root.secretMap = root.config.hasOwnProperty("secrets") ? oceanuiData.toMap(root.config.secrets) : {}

        const mppeList = ["require-mppe", "require-mppe-40", "require-mppe-128"]
        mppe = false
        currentMppeMethod = "require-mppe"
        for (let mppeMethod of mppeList) {
            if (root.dataMap.hasOwnProperty(mppeMethod) && root.dataMap[mppeMethod] === "yes") {
                mppe = true
                currentMppeMethod = mppeMethod
            }
        }
        ipsecEnabled = root.dataMap["ipsec-enabled"] === "yes"
        // 防止密码项没设置对应项显示错误，先给默认值
        const pwdKeys = ["password-flags", "Xauth password-flags", "IPSec secret-flags", "cert-pass-flags"]
        for (let pwdKey of pwdKeys) {
            if (!root.dataMap.hasOwnProperty(pwdKey)) {
                root.dataMap[pwdKey] = "0"
            }
        }
        if (!root.dataMap.hasOwnProperty("method")) {
            root.dataMap["method"] = "key"
        }
        if (!root.dataMap.hasOwnProperty("connection-type")) {
            root.dataMap["connection-type"] = "tls"
        }
        console.log("proxy-type:  ", root.dataMap["proxy-type"], root.dataMap["proxy-type"] !== "none")
        if (!root.dataMap.hasOwnProperty("proxy-type")) {
            root.dataMap["proxy-type"] = "none"
        }
        console.log("proxy-type:  ", root.dataMap["proxy-type"], root.dataMap["proxy-type"] !== "none")
        switch (root.vpnType) {
        case NetUtils.VpnTypeEnum["vpnc"]:
            if (root.dataMap["Enable no encryption"] === "yes") {
                currentEncryption = "none"
            } else if (root.dataMap["Enable Single DES"] === "yes") {
                currentEncryption = "weak"
            } else {
                currentEncryption = "secure"
            }
            break
        }
        dataMapChanged()
    }
    function getConfig() {
        if (mppe) {
            switch (currentMppeMethod) {
            case "require-mppe":
                root.dataMap["require-mppe"] = "yes"
                delete root.dataMap["require-mppe-40"]
                delete root.dataMap["require-mppe-128"]
                break
            case "require-mppe-40":
                root.dataMap["require-mppe-40"] = "yes"
                delete root.dataMap["require-mppe"]
                delete root.dataMap["require-mppe-128"]
                break
            case "require-mppe-128":
                root.dataMap["require-mppe-128"] = "yes"
                delete root.dataMap["require-mppe-40"]
                delete root.dataMap["require-mppe"]
                break
            }
        } else {
            delete root.dataMap["require-mppe"]
            delete root.dataMap["require-mppe-40"]
            delete root.dataMap["require-mppe-128"]
            delete root.config["mppe-stateful"]
        }
        // vpn配置全在这，防止不同vpn的配置都保存，这里用key过虑下
        let dataKeys = []
        let secretKeys = []
        const pppKeys = ["require-mppe", "require-mppe-40", "require-mppe-128", "mppe-stateful", "refuse-eap", "refuse-pap", "refuse-chap", "refuse-mschap", "refuse-mschapv2", "nobsdcomp", "nodeflate", "no-vj-comp", "nopcomp", "noaccomp", "lcp-echo-failure", "lcp-echo-interval"]
        const ipsecKeys = ["ipsec-enabled", "ipsec-group-name", "ipsec-gateway-id", "ipsec-psk", "ipsec-ike", "ipsec-esp"]
        const vpncAdvKeys = ["Domain", "Vendor", "Application Version", "Enable Single DES", "Enable no encryption", "NAT Traversal Mode", "IKE DH Group", "Perfect Forward Secrecy", "Local Port", "DPD idle timeout (our side)"]
        switch (root.vpnType) {
        case NetUtils.VpnTypeEnum["l2tp"]:
            dataKeys = ["gateway", "user", "password-flags", "domain"]
            dataKeys = dataKeys.concat(pppKeys)
            dataKeys = dataKeys.concat(ipsecKeys)
            if (root.dataMap["password-flags"] === "0") {
                secretKeys = ["password"]
            }
            break
        case NetUtils.VpnTypeEnum["pptp"]:
            dataKeys = ["gateway", "user", "password-flags", "domain"]
            dataKeys = dataKeys.concat(pppKeys)
            if (root.dataMap["password-flags"] === "0") {
                secretKeys = ["password"]
            }
            break
        case NetUtils.VpnTypeEnum["vpnc"]:
            dataKeys = ["IPSec gateway", "Xauth username", "Xauth password-flags", "xauth-password-type", "IPSec ID", "IPSec secret-flags", "ipsec-secret-type", "IKE Authmode", "CA-File"]
            dataKeys = dataKeys.concat(vpncAdvKeys)
            if (root.dataMap["Xauth password-flags"] === "0") {
                secretKeys.push("Xauth password")
            }
            if (root.dataMap["IPSec secret-flags"] === "0") {
                secretKeys.push("IPSec secret")
            }
            switch (currentEncryption) {
            case "none":
                delete root.dataMap["Enable Single DES"]
                root.dataMap["Enable no encryption"] = "yes"
                break
            case "weak":
                delete root.dataMap["Enable no encryption"]
                root.dataMap["Enable Single DES"] = "yes"
                break
            case "secure":
                delete root.dataMap["Enable Single DES"]
                delete root.dataMap["Enable no encryption"]
                break
            }
            switch (root.dataMap["Xauth password-flags"]) {
            case "0":
                root.dataMap["xauth-password-type"] = "save"
                break
            case "1":
                root.dataMap["xauth-password-type"] = "ask"
                break
            case "2":
                root.dataMap["xauth-password-type"] = "unused"
                break
            }
            switch (root.dataMap["IPSec secret-flags"]) {
            case "0":
                root.dataMap["ipsec-secret-type"] = "save"
                break
            case "1":
                root.dataMap["ipsec-secret-type"] = "ask"
                break
            case "2":
                root.dataMap["ipsec-secret-type"] = "unused"
                break
            }
            break
        case NetUtils.VpnTypeEnum["openconnect"]:
            dataKeys = ["gateway", "cacert", "proxy", "enable_csd_trojan", "csd_wrapper", "usercert", "userkey", "pem_passphrase_fsid", "cookie-flags"]
            root.dataMap["cookie-flags"] = "2"
            break
        case NetUtils.VpnTypeEnum["strongswan"]:
            dataKeys = ["address", "certificate", "method", "usercert", "userkey", "user", "virtual", "encap", "ipcomp", "proposal", "ike", "esp"]
            secretKeys = ["password"]
            break
        case NetUtils.VpnTypeEnum["openvpn"]:
            dataKeys = ["remote", "connection-type", "port", "reneg-seconds", "comp-lzo", "proto-tcp", "dev-type", "tunnel-mtu", "fragment-size", "mssfix", "remote-random", "tls-remote", "remote-cert-tls", "ta", "ta-dir"]
            switch (root.dataMap["connection-type"]) {
            case "tls":
                dataKeys = dataKeys.concat(["ca", "cert", "key", "cert-pass-flags"])
                if (root.dataMap["cert-pass-flags"] === "0") {
                    secretKeys.push("cert-pass")
                }
                break
            case "password":
                dataKeys = dataKeys.concat(["ca", "username", "password-flags"])
                if (root.dataMap["password-flags"] === "0") {
                    secretKeys.push("password")
                }
                break
            case "password-tls":
                dataKeys = dataKeys.concat(["ca", "username", "password-flags", "cert", "key", "cert-pass-flags"])
                if (root.dataMap["password-flags"] === "0") {
                    secretKeys.push("password")
                }
                if (root.dataMap["cert-pass-flags"] === "0") {
                    secretKeys.push("cert-pass")
                }
                break
            case "static-key":
                dataKeys = dataKeys.concat(["static-key", "static-key-direction", "remote-ip", "local-ip"])
                break
            }
            if (root.dataMap["cipher"] !== "default") {
                dataKeys.push("cipher")
            }
            if (root.dataMap["auth"] !== "default") {
                dataKeys.push("auth")
            }

            switch (root.dataMap["proxy-type"]) {
            case "http":
                dataKeys = dataKeys.concat(["proxy-type", "proxy-server", "proxy-port", "proxy-retry", "http-proxy-username", "http-proxy-password-flags"])
                root.dataMap["http-proxy-password-flags"] = "0"
                secretKeys.push("http-proxy-password")
                break
            case "socks":
                dataKeys = dataKeys.concat(["proxy-type", "proxy-server", "proxy-port", "proxy-retry"])
                break
            case "none":
                break
            }
            break
        }
        let tmpDataMap = {}
        for (var key of dataKeys) {
            if (root.dataMap.hasOwnProperty(key)) {
                tmpDataMap[key] = root.dataMap[key]
            }
        }
        let tmpSecretMap = {}
        for (key of secretKeys) {
            if (root.secretMap.hasOwnProperty(key)) {
                tmpSecretMap[key] = root.secretMap[key]
            }
        }

        root.config.data = oceanuiData.toStringMap(tmpDataMap)
        root.config.secrets = oceanuiData.toStringMap(tmpSecretMap)
        return root.config
    }
    function checkInput() {
        errorKey = ""
        let checkKeys = []
        switch (root.vpnType) {
        case NetUtils.VpnTypeEnum["pptp"]:
        case NetUtils.VpnTypeEnum["l2tp"]:
            // ipv6 不支持，这里过滤掉，不然后面host 会反向解析浪费时间。
            if (!root.dataMap.hasOwnProperty("gateway") || root.dataMap["gateway"].length === 0 || NetUtils.ipv6RegExp.test(root.dataMap["gateway"])) {
                errorKey = "gateway"
                return false
            }
            checkKeys = [[root.dataMap, "gateway"], [root.dataMap, "user"]]
            console.log("password-flags", root.dataMap["password-flags"])
            if (!root.dataMap.hasOwnProperty("password-flags") || root.dataMap["password-flags"] === "0") {
                checkKeys.push([root.secretMap, "password"])
            }
            console.log("length", checkKeys.length)
            break
        case NetUtils.VpnTypeEnum["vpnc"]:
            checkKeys = [[root.dataMap, "IPSec gateway"], [root.dataMap, "Xauth username"]]
            if (!root.dataMap.hasOwnProperty("Xauth password-flags") || root.dataMap["Xauth password-flags"] === "0") {
                checkKeys.push([root.secretMap, "Xauth password"])
            }
            checkKeys.push([root.dataMap, "IPSec ID"])
            if (!root.dataMap.hasOwnProperty("IPSec secret-flags") || root.dataMap["IPSec secret-flags"] === "0") {
                checkKeys.push([root.secretMap, "IPSec secret"])
            }
            if (root.dataMap["IKE Authmode"] === "hybrid") {
                checkKeys.push([root.dataMap, "CA-File"])
            }
            break
        case NetUtils.VpnTypeEnum["openconnect"]:
            checkKeys = [[root.dataMap, "gateway"], [root.dataMap, "usercert"], [root.dataMap, "userkey"]]
            break
        case NetUtils.VpnTypeEnum["strongswan"]:
            checkKeys = [[root.dataMap, "address"]]
            break
        case NetUtils.VpnTypeEnum["openvpn"]:
            checkKeys = [[root.dataMap, "remote"]]
            const openvpnTlsKeys = [[root.dataMap, "cert"], [root.dataMap, "key"]]
            const openvpnPwdKeys = [[root.dataMap, "username"]]
            switch (root.dataMap["connection-type"]) {
            case "tls":
                checkKeys.push([root.dataMap, "ca"])
                checkKeys = checkKeys.concat(openvpnTlsKeys)
                if (root.dataMap["cert-pass-flags"] === "0") {
                    checkKeys.push([root.secretMap, "cert-pass"])
                }
                break
            case "password":
                checkKeys.push([root.dataMap, "ca"])
                checkKeys = checkKeys.concat(openvpnPwdKeys)
                if (root.dataMap["password-flags"] === "0") {
                    checkKeys.push([root.secretMap, "password"])
                }
                break
            case "password-tls":
                checkKeys.push([root.dataMap, "ca"])
                checkKeys = checkKeys.concat(openvpnPwdKeys)
                checkKeys = checkKeys.concat(openvpnTlsKeys)
                if (root.dataMap["password-flags"] === "0") {
                    checkKeys.push([root.secretMap, "password"])
                }
                if (root.dataMap["cert-pass-flags"] === "0") {
                    checkKeys.push([root.secretMap, "cert-pass"])
                }
                break
            case "static-key":
                checkKeys = checkKeys.concat([[root.dataMap, "static-key"], [root.dataMap, "remote-ip"], [root.dataMap, "local-ip"]])
                break
            }
            switch (root.dataMap["proxy-type"]) {
            case "http":
                checkKeys = checkKeys.concat([[root.dataMap, "proxy-server"], [root.dataMap, "http-proxy-username"], [root.secretMap, "http-proxy-password"]])
                break
            case "socks":
                checkKeys.push([root.dataMap, "proxy-server"])
                break
            case "none":
                break
            }
            break
        }
        for (var key of checkKeys) {
            console.log("err:", key[1], key[0][key[1]])
            if (!key[0].hasOwnProperty(key[1]) || key[0][key[1]].length === 0) {
                errorKey = key[1]
                return false
            }
        }
        return true
    }

    name: "VPNTitle"
    displayName: qsTr("VPN")
    Component {
        id: fileDialog
        FileDialog {
            property var selectEdit: null
            visible: false
            nameFilters: [qsTr("All files (*)")]
            onAccepted: {
                selectEdit.text = currentFile.toString().replace("file://", "")
                this.destroy(10)
            }
            onRejected: this.destroy(10)
        }
    }
    Component {
        id: fileLineEdit
        RowLayout {
            D.LineEdit {
                id: fileEdit
                text: root.dataMap.hasOwnProperty(oceanuiObj.name) ? NetUtils.removeTrailingNull(root.dataMap[oceanuiObj.name]) : ""
                onTextChanged: {
                    if (showAlert) {
                        errorKey = ""
                    }
                    if (root.dataMap[oceanuiObj.name] !== text) {
                        root.dataMap[oceanuiObj.name] = text
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
            NetButton {
                text: "..."
                onClicked: {
                    fileDialog.createObject(this, {
                                                "selectEdit": fileEdit
                                            }).open()
                }
            }
        }
    }
    Component {
        id: requiredFileLineEdit
        RowLayout {
            D.LineEdit {
                id: fileEdit
                placeholderText: qsTr("Required")
                text: root.dataMap.hasOwnProperty(oceanuiObj.name) ? NetUtils.removeTrailingNull(root.dataMap[oceanuiObj.name]) : ""
                onTextChanged: {
                    if (showAlert) {
                        errorKey = ""
                    }
                    if (root.dataMap[oceanuiObj.name] !== text) {
                        root.dataMap[oceanuiObj.name] = text
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
            NetButton {
                text: "..."
                onClicked: {
                    fileDialog.createObject(this, {
                                                "selectEdit": fileEdit
                                            }).open()
                }
            }
        }
    }
    Component {
        id: gatewayLineEdit
        D.LineEdit {
            placeholderText: qsTr("Required")
            text: root.dataMap.hasOwnProperty(oceanuiObj.name) ? root.dataMap[oceanuiObj.name] : ""
            onTextChanged: {
                if (showAlert) {
                    errorKey = ""
                }
                if (root.dataMap[oceanuiObj.name] !== text) {
                    root.dataMap[oceanuiObj.name] = text
                    root.editClicked()
                }
            }
            showAlert: errorKey === oceanuiObj.name
            alertDuration: 2000
            alertText: qsTr("Invalid gateway")
            onShowAlertChanged: {
                if (showAlert) {
                    OceanUIApp.showPage(oceanuiObj)
                    forceActiveFocus()
                }
            }
        }
    }
    Component {
        id: requiredLineEdit
        D.LineEdit {
            placeholderText: qsTr("Required")
            text: root.dataMap.hasOwnProperty(oceanuiObj.name) ? root.dataMap[oceanuiObj.name] : ""
            onTextChanged: {
                if (showAlert) {
                    errorKey = ""
                }
                if (root.dataMap[oceanuiObj.name] !== text) {
                    root.dataMap[oceanuiObj.name] = text
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
    Component {
        id: lineEdit
        D.LineEdit {
            text: root.dataMap.hasOwnProperty(oceanuiObj.name) ? root.dataMap[oceanuiObj.name] : ""
            onTextChanged: {
                if (showAlert) {
                    errorKey = ""
                }
                if (root.dataMap[oceanuiObj.name] !== text) {
                    root.dataMap[oceanuiObj.name] = text
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
    Component {
        id: switchItem
        D.Switch {
            checked: root.dataMap.hasOwnProperty(oceanuiObj.name) && root.dataMap[oceanuiObj.name] === "yes"
            onClicked: {
                if (checked) {
                    root.dataMap[oceanuiObj.name] = "yes"
                } else {
                    delete root.dataMap[oceanuiObj.name]
                }
                dataMapChanged()
                root.editClicked()
            }
        }
    }

    OceanUIObject {
        name: "vpnGroup"
        parentName: root.parentName
        weight: root.weight + 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        // l2tp/pptp
        OceanUIObject {
            name: "gateway"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Gateway")
            weight: 10
            visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"] | NetUtils.VpnTypeEnum["openconnect"])
            pageType: OceanUIObject.Editor
            page: gatewayLineEdit
        }
        OceanUIObject {
            name: "cacert"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("CA Cert")
            weight: 20
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openconnect"])
            pageType: OceanUIObject.Editor
            page: fileLineEdit
        }
        OceanUIObject {
            name: "proxy"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Proxy")
            weight: 30
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openconnect"])
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "enable_csd_trojan"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Allow Cisco Secure Desktop Trojan")
            weight: 40
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openconnect"])
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "csd_wrapper"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("CSD Script")
            weight: 50
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openconnect"])
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "address"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Gateway")
            weight: 60
            visible: root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])
            pageType: OceanUIObject.Editor
            page: gatewayLineEdit
        }
        OceanUIObject {
            name: "certificate"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("CA Cert")
            weight: 70
            visible: root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])
            pageType: OceanUIObject.Editor
            page: fileLineEdit
        }
        OceanUIObject {
            name: "method"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Auth Type")
            weight: 80
            visible: root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Private Key"),
                        "value": "key"
                    }, {
                        "text": qsTr("SSH Agent"),
                        "value": "agent"
                    }, {
                        "text": qsTr("Smart Card"),
                        "value": "smartcard"
                    }, {
                        "text": qsTr("EAP"),
                        "value": "eap"
                    }, {
                        "text": qsTr("Pre-Shared Key"),
                        "value": "psk"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    dataMapChanged()
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "usercert"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("User Cert")
            weight: 90
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openconnect"])) || ((root.vpnType & NetUtils.VpnTypeEnum["strongswan"]) && (root.dataMap["method"] === "key" || root.dataMap["method"] === "agent"))
            pageType: OceanUIObject.Editor
            page: RowLayout {
                D.LineEdit {
                    id: usercertEdit
                    placeholderText: root.vpnType === NetUtils.VpnTypeEnum["strongswan"] ? "" : qsTr("Required")
                    text: root.dataMap.hasOwnProperty(oceanuiObj.name) ? NetUtils.removeTrailingNull(root.dataMap[oceanuiObj.name]) : ""
                    onTextChanged: {
                        if (showAlert) {
                            errorKey = ""
                        }
                        if (root.dataMap[oceanuiObj.name] !== text) {
                            root.dataMap[oceanuiObj.name] = text
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
                NetButton {
                    text: "..."
                    onClicked: {
                        fileDialog.createObject(this, {
                                                    "selectEdit": usercertEdit
                                                }).open()
                    }
                }
            }
        }
        OceanUIObject {
            name: "userkey"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Private Key")
            weight: 100
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openconnect"])) || ((root.vpnType & NetUtils.VpnTypeEnum["strongswan"]) && (root.dataMap["method"] === "key"))
            pageType: OceanUIObject.Editor
            page: RowLayout {
                D.LineEdit {
                    id: userkeyEdit
                    placeholderText: root.vpnType === NetUtils.VpnTypeEnum["strongswan"] ? "" : qsTr("Required")
                    text: root.dataMap.hasOwnProperty(oceanuiObj.name) ? NetUtils.removeTrailingNull(root.dataMap[oceanuiObj.name]) : ""
                    onTextChanged: {
                        if (showAlert) {
                            errorKey = ""
                        }
                        if (root.dataMap[oceanuiObj.name] !== text) {
                            root.dataMap[oceanuiObj.name] = text
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
                NetButton {
                    text: "..."
                    onClicked: {
                        fileDialog.createObject(this, {
                                                    "selectEdit": userkeyEdit
                                                }).open()
                    }
                }
            }
        }
        OceanUIObject {
            name: "user"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Username")
            weight: 110
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"])) || ((root.vpnType & NetUtils.VpnTypeEnum["strongswan"]) && (root.dataMap["method"] === "eap" || root.dataMap["method"] === "psk"))
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "pem_passphrase_fsid"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Use FSID for Key Passphrase")
            weight: 120
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openconnect"])
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "remote"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Gateway")
            weight: 130
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
            pageType: OceanUIObject.Editor
            page: gatewayLineEdit
        }
        OceanUIObject {
            name: "connection-type"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Auth Type")
            weight: 140
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Certificates (TLS)"),
                        "value": "tls"
                    }, {
                        "text": qsTr("Password"),
                        "value": "password"
                    }, {
                        "text": qsTr("Certificates with Password (TLS)"),
                        "value": "password-tls"
                    }, {
                        "text": qsTr("Static Key"),
                        "value": "static-key"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    dataMapChanged()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "ca"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("CA Cert")
            weight: 150
            visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"]) && (root.dataMap["connection-type"] === "tls" || root.dataMap["connection-type"] === "password" || root.dataMap["connection-type"] === "password-tls")
            pageType: OceanUIObject.Editor
            page: requiredFileLineEdit
        }
        OceanUIObject {
            name: "username"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Username")
            weight: 160
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "password" || root.dataMap["connection-type"] === "password-tls")
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "password-flags"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Pwd Options")
            weight: 170
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"])) || (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "password" || root.dataMap["connection-type"] === "password-tls")
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Saved"),
                        "value": "0"
                    }, {
                        "text": qsTr("Ask"),
                        "value": "2"
                    }, {
                        "text": qsTr("Not Required"),
                        "value": "4"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    dataMapChanged()
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "password"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Password")
            weight: 180
            visible: ((root.vpnType & NetUtils.VpnTypeEnum["strongswan"]) && (root.dataMap["method"] === "eap" || root.dataMap["method"] === "psk")) || (root.dataMap["password-flags"] === "0" && ((root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"])) || (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "password" || root.dataMap["connection-type"] === "password-tls")))
            pageType: OceanUIObject.Editor
            page: NetPasswordEdit {
                dataItem: root
                text: root.secretMap.hasOwnProperty(oceanuiObj.name) ? root.secretMap[oceanuiObj.name] : ""
                onTextUpdated: root.secretMap[oceanuiObj.name] = text
            }
        }
        OceanUIObject {
            name: "cert"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("User Cert")
            weight: 190
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "password-tls" || root.dataMap["connection-type"] === "tls")
            pageType: OceanUIObject.Editor
            page: fileLineEdit
        }
        OceanUIObject {
            name: "key"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Private Key")
            weight: 200
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "password-tls" || root.dataMap["connection-type"] === "tls")
            pageType: OceanUIObject.Editor
            page: fileLineEdit
        }
        OceanUIObject {
            name: "cert-pass-flags"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Pwd Options")
            weight: 210
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "password-tls" || root.dataMap["connection-type"] === "tls")
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Saved"),
                        "value": "0"
                    }, {
                        "text": qsTr("Ask"),
                        "value": "2"
                    }, {
                        "text": qsTr("Not Required"),
                        "value": "4"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    dataMapChanged()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "cert-pass"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Private Pwd")
            weight: 220
            visible: root.dataMap["cert-pass-flags"] === "0" && (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "password-tls" || root.dataMap["connection-type"] === "tls")
            pageType: OceanUIObject.Editor
            page: NetPasswordEdit {
                dataItem: root
                text: root.secretMap.hasOwnProperty(oceanuiObj.name) ? root.secretMap[oceanuiObj.name] : ""
                onTextUpdated: root.secretMap[oceanuiObj.name] = text
            }
        }
        OceanUIObject {
            name: "static-key"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Static Key")
            weight: 230
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "static-key")
            pageType: OceanUIObject.Editor
            page: requiredFileLineEdit
        }
        OceanUIObject {
            name: "has_static-key-direction"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Customize Key Direction")
            weight: 240
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "static-key")
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty("static-key-direction")
                onClicked: {
                    if (checked) {
                        root.dataMap["static-key-direction"] = "0"
                    } else {
                        delete root.dataMap["static-key-direction"]
                    }
                    dataMapChanged()
                }
            }
        }
        OceanUIObject {
            name: "static-key-direction"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Key Direction")
            weight: 250
            visible: root.dataMap.hasOwnProperty(this.name) && (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "static-key")
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("0"),
                        "value": "0"
                    }, {
                        "text": qsTr("1"),
                        "value": "1"
                    }]
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                Connections {
                    target: root
                    function onDataMapChanged() {
                        currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                    }
                }
            }
        }
        OceanUIObject {
            name: "remote-ip"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Remote IP")
            weight: 260
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "static-key")
            pageType: OceanUIObject.Editor
            page: requiredFileLineEdit
        }
        OceanUIObject {
            name: "local-ip"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Local IP")
            weight: 270
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] === "static-key")
            pageType: OceanUIObject.Editor
            page: requiredFileLineEdit
        }
        OceanUIObject {
            name: "domain"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("NT Domain")
            weight: 280
            visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"])
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "virtual"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Request an Inner IP Address")
            weight: 290
            visible: root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "encap"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Enforce UDP Encapsulation")
            weight: 300
            visible: root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "ipcomp"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Use IP Compression")
            weight: 310
            visible: root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "proposal"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Enable Custom Cipher Proposals")
            weight: 320
            visible: root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty(oceanuiObj.name) ? root.dataMap[oceanuiObj.name] === "yes" : false
                onClicked: {
                    root.dataMap[oceanuiObj.name] = checked ? "yes" : "no"
                    dataMapChanged()
                }
            }
        }
        OceanUIObject {
            name: "ike"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("IKE")
            weight: 330
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])) && root.dataMap["proposal"] === "yes"
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "esp"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("ESP")
            weight: 340
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["strongswan"])) && root.dataMap["proposal"] === "yes"
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        // vpnc
        OceanUIObject {
            name: "IPSec gateway"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Gateway")
            weight: 350
            visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
            pageType: OceanUIObject.Editor
            page: gatewayLineEdit
        }
        OceanUIObject {
            name: "Xauth username"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Username")
            weight: 360
            visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "Xauth password-flags"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Pwd Options")
            weight: 370
            visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Saved"),
                        "value": "0"
                    }, {
                        "text": qsTr("Ask"),
                        "value": "2"
                    }, {
                        "text": qsTr("Not Required"),
                        "value": "4"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    dataMapChanged()
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "Xauth password"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Password")
            weight: 380
            visible: root.dataMap["Xauth password-flags"] === "0" && (root.vpnType & (NetUtils.VpnTypeEnum["vpnc"]))
            pageType: OceanUIObject.Editor
            page: NetPasswordEdit {
                dataItem: root
                text: root.secretMap.hasOwnProperty(oceanuiObj.name) ? root.secretMap[oceanuiObj.name] : ""
                onTextUpdated: root.secretMap[oceanuiObj.name] = text
            }
        }
        OceanUIObject {
            name: "IPSec ID"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Group Name")
            weight: 390
            visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "IPSec secret-flags"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Pwd Options")
            weight: 400
            visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Saved"),
                        "value": "0"
                    }, {
                        "text": qsTr("Ask"),
                        "value": "2"
                    }, {
                        "text": qsTr("Not Required"),
                        "value": "4"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    dataMapChanged()
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "IPSec secret"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Group Pwd")
            weight: 410
            visible: root.dataMap["IPSec secret-flags"] === "0" && (root.vpnType & (NetUtils.VpnTypeEnum["vpnc"]))
            pageType: OceanUIObject.Editor
            page: NetPasswordEdit {
                dataItem: root
                text: root.secretMap.hasOwnProperty(oceanuiObj.name) ? root.secretMap[oceanuiObj.name] : ""
                onTextUpdated: root.secretMap[oceanuiObj.name] = text
            }
        }
        OceanUIObject {
            name: "IKE Authmode"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("Use Hybrid Authentication")
            weight: 420
            visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty(oceanuiObj.name) && root.dataMap[oceanuiObj.name] === "hybrid"
                onClicked: {
                    if (checked) {
                        root.dataMap[oceanuiObj.name] = "hybrid"
                    } else {
                        delete root.dataMap[oceanuiObj.name]
                    }
                    dataMapChanged()
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "CA-File"
            parentName: root.parentName + "/vpnGroup"
            displayName: qsTr("CA File")
            weight: 430
            visible: (root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])) && root.dataMap["IKE Authmode"] === "hybrid"
            pageType: OceanUIObject.Editor
            page: fileLineEdit
        }
    }

    // VPN PPP
    OceanUITitleObject {
        name: "vpnPPPTitle"
        parentName: root.parentName
        displayName: qsTr("VPN PPP")
        weight: root.weight + 100
        visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"])
    }
    OceanUIObject {
        name: "vpnPPPMPPEGroup"
        parentName: root.parentName
        weight: root.weight + 110
        visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"])
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "require-mppe"
            parentName: root.parentName + "/vpnPPPMPPEGroup"
            displayName: qsTr("Use MPPE")
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: mppe
                onClicked: {
                    mppe = checked
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "security"
            parentName: root.parentName + "/vpnPPPMPPEGroup"
            displayName: qsTr("Security")
            weight: 20
            visible: mppe
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: indexOfValue(currentMppeMethod)
                model: [{
                        "text": qsTr("All Available (default)"),
                        "value": "require-mppe"
                    }, {
                        "text": qsTr("40-bit (less secure)"),
                        "value": "require-mppe-40"
                    }, {
                        "text": qsTr("128-bit (most secure)"),
                        "value": "require-mppe-128"
                    }]
                onActivated: {
                    currentMppeMethod = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = indexOfValue(currentMppeMethod)
            }
        }
        OceanUIObject {
            name: "mppe-stateful"
            parentName: root.parentName + "/vpnPPPMPPEGroup"
            displayName: qsTr("Stateful MPPE")
            weight: 30
            visible: mppe
            pageType: OceanUIObject.Editor
            page: switchItem
        }
    }
    OceanUIObject {
        name: "vpnPPPGroup"
        parentName: root.parentName
        weight: root.weight + 120
        visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"] | NetUtils.VpnTypeEnum["pptp"])
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "refuse-eap"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("Refuse EAP Authentication")
            weight: 40
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-pap"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("Refuse PAP Authentication")
            weight: 50
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-chap"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("Refuse CHAP Authentication")
            weight: 60
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-mschap"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("Refuse MSCHAP Authentication")
            weight: 70
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "refuse-mschapv2"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("Refuse MSCHAPv2 Authentication")
            weight: 80
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "nobsdcomp"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("No BSD Data Compression")
            weight: 90
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "nodeflate"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("No Deflate Data Compression")
            weight: 100
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "no-vj-comp"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("No TCP Header Compression")
            weight: 110
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "nopcomp"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("No Protocol Field Compression")
            weight: 120
            visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"])
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "noaccomp"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("No Address/Control Compression")
            weight: 130
            visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"])
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "lcpEchoInterval"
            parentName: root.parentName + "/vpnPPPGroup"
            displayName: qsTr("Send PPP Echo Packets")
            weight: 140
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap["lcp-echo-failure"] === "5" && root.dataMap["lcp-echo-interval"] === "30"
                onClicked: {
                    if (checked) {
                        root.dataMap["lcp-echo-failure"] = "5"
                        root.dataMap["lcp-echo-interval"] = "30"
                    } else {
                        delete root.dataMap["lcp-echo-failure"]
                        delete root.dataMap["lcp-echo-interval"]
                    }
                    root.editClicked()
                }
            }
        }
    }
    // VPN IPsec
    OceanUITitleObject {
        name: "vpnIPsecTitle"
        parentName: root.parentName
        displayName: qsTr("VPN IPsec")
        weight: root.weight + 200
        visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"])
    }
    OceanUIObject {
        name: "vpnIPsecGroup"
        parentName: root.parentName
        weight: root.weight + 210
        visible: root.vpnType & (NetUtils.VpnTypeEnum["l2tp"])
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "ipsec-enabled"
            parentName: root.parentName + "/vpnIPsecGroup"
            displayName: qsTr("Enable IPsec")
            weight: 10
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "ipsec-group-name"
            parentName: root.parentName + "/vpnIPsecGroup"
            displayName: qsTr("Group Name")
            weight: 20
            visible: root.dataMap["ipsec-enabled"] === "yes"
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "ipsec-gateway-id"
            parentName: root.parentName + "/vpnIPsecGroup"
            displayName: qsTr("Group ID")
            weight: 30
            visible: root.dataMap["ipsec-enabled"] === "yes"
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "ipsec-psk"
            parentName: root.parentName + "/vpnIPsecGroup"
            displayName: qsTr("Pre-Shared Key")
            weight: 40
            visible: root.dataMap["ipsec-enabled"] === "yes"
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "ipsec-ike"
            parentName: root.parentName + "/vpnIPsecGroup"
            displayName: qsTr("Phase1 Algorithms")
            weight: 50
            visible: root.dataMap["ipsec-enabled"] === "yes"
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "ipsec-esp"
            parentName: root.parentName + "/vpnIPsecGroup"
            displayName: qsTr("Phase2 Algorithms")
            weight: 60
            visible: root.dataMap["ipsec-enabled"] === "yes"
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
    }
    // VPN Advanced
    OceanUITitleObject {
        name: "vpnAdvancedTitle"
        parentName: root.parentName
        displayName: qsTr("VPN Advanced")
        weight: root.weight + 300
        visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
    }
    OceanUIObject {
        name: "vpnAdvancedGroup"
        parentName: root.parentName
        weight: root.weight + 310
        visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "has_port"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Customize Gateway Port")
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty("port")
                onClicked: {
                    if (checked) {
                        root.dataMap["port"] = "1194"
                    } else {
                        delete root.dataMap["port"]
                    }
                    dataMapChanged()
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "port"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Gateway Port")
            weight: 20
            visible: root.dataMap.hasOwnProperty("port")
            pageType: OceanUIObject.Editor
            page: D.SpinBox {
                value: root.dataMap.hasOwnProperty(oceanuiObj.name) ? parseInt(root.dataMap[oceanuiObj.name], 10) : 1194
                from: 0
                to: 65535
                onValueChanged: {
                    if (root.dataMap[oceanuiObj.name] !== value) {
                        root.dataMap[oceanuiObj.name] = value
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "has_reneg-seconds"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Customize Renegotiation Interval")
            weight: 30
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty("reneg-seconds")
                onClicked: {
                    if (checked) {
                        root.dataMap["reneg-seconds"] = "0"
                    } else {
                        delete root.dataMap["reneg-seconds"]
                    }
                    dataMapChanged()
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "reneg-seconds"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Renegotiation Interval")
            weight: 40
            visible: root.dataMap.hasOwnProperty("reneg-seconds")
            pageType: OceanUIObject.Editor
            page: D.SpinBox {
                value: root.dataMap.hasOwnProperty(oceanuiObj.name) ? parseInt(root.dataMap[oceanuiObj.name], 10) : 0
                from: 0
                to: 65535
                onValueChanged: {
                    if (root.dataMap[oceanuiObj.name] !== value) {
                        root.dataMap[oceanuiObj.name] = value
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "comp-lzo"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Use LZO Data Compression")
            weight: 50
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap["comp-lzo"] === "yes" || root.dataMap["comp-lzo"] === "adaptive"
                onClicked: {
                    if (checked) {
                        root.dataMap[oceanuiObj.name] = "yes"
                    } else {
                        delete root.dataMap[oceanuiObj.name]
                    }
                    dataMapChanged()
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "proto-tcp"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Use TCP Connection")
            weight: 60
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "dev-type"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Use TAP Device")
            weight: 70
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "has_tunnel-mtu"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Customize Tunnel MTU")
            weight: 80
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty("tunnel-mtu")
                onClicked: {
                    if (checked) {
                        root.dataMap["tunnel-mtu"] = "1500"
                    } else {
                        delete root.dataMap["tunnel-mtu"]
                    }
                    dataMapChanged()
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "tunnel-mtu"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("MTU")
            weight: 90
            visible: root.dataMap.hasOwnProperty("tunnel-mtu")
            pageType: OceanUIObject.Editor
            page: D.SpinBox {
                value: root.dataMap.hasOwnProperty(oceanuiObj.name) ? parseInt(root.dataMap[oceanuiObj.name], 10) : 1500
                from: 0
                to: 65535
                onValueChanged: {
                    if (root.dataMap[oceanuiObj.name] !== value) {
                        root.dataMap[oceanuiObj.name] = value
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "has_fragment-size"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Customize UDP Fragment Size")
            weight: 100
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty("fragment-size")
                onClicked: {
                    if (checked) {
                        root.dataMap["fragment-size"] = "1300"
                    } else {
                        delete root.dataMap["fragment-size"]
                    }
                    dataMapChanged()
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "fragment-size"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("UDP Fragment Size")
            weight: 110
            visible: root.dataMap.hasOwnProperty("fragment-size")
            pageType: OceanUIObject.Editor
            page: D.SpinBox {
                value: root.dataMap.hasOwnProperty(oceanuiObj.name) ? parseInt(root.dataMap[oceanuiObj.name], 10) : 1300
                from: 0
                to: 65535
                onValueChanged: {
                    if (root.dataMap[oceanuiObj.name] !== value) {
                        root.dataMap[oceanuiObj.name] = value
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "mssfix"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Restrict Tunnel TCP MSS")
            weight: 120
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "remote-random"
            parentName: root.parentName + "/vpnAdvancedGroup"
            displayName: qsTr("Randomize Remote Hosts")
            weight: 130
            pageType: OceanUIObject.Editor
            page: switchItem
        }
    }
    // VPN Advanced vpn vpnc
    OceanUITitleObject {
        name: "vpncAdvancedTitle"
        parentName: root.parentName
        displayName: qsTr("VPN Advanced")
        weight: root.weight + 350
        visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
    }
    OceanUIObject {
        name: "vpncAdvancedGroup"
        parentName: root.parentName
        weight: root.weight + 360
        visible: root.vpnType & (NetUtils.VpnTypeEnum["vpnc"])
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "Domain"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("Domain")
            weight: 10
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "Vendor"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("Vendor")
            weight: 20
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Cisco (default)"),
                        "value": "cisco"
                    }, {
                        "text": qsTr("Netscreen"),
                        "value": "netscreen"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "Application Version"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("Version")
            weight: 30
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "encryption"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("Encryption")
            weight: 40
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Secure (default)"),
                        "value": "secure"
                    }, {
                        "text": qsTr("Weak"),
                        "value": "weak"
                    }, {
                        "text": qsTr("None"),
                        "value": "none"
                    }]
                currentIndex: indexOfValue(currentEncryption)
                onActivated: {
                    currentEncryption = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = indexOfValue(currentEncryption)
            }
        }
        OceanUIObject {
            name: "NAT Traversal Mode"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("NAT Traversal Mode")
            weight: 50
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("NAT-T When Available (default)"),
                        "value": "natt"
                    }, {
                        "text": qsTr("NAT-T Always"),
                        "value": "force-natt"
                    }, {
                        "text": qsTr("Cisco UDP"),
                        "value": "cisco-udp"
                    }, {
                        "text": qsTr("Disabled"),
                        "value": "none"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "IKE DH Group"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("IKE DH Group")
            weight: 60
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("DH Group 1"),
                        "value": "dh1"
                    }, {
                        "text": qsTr("DH Group 2 (default)"),
                        "value": "dh2"
                    }, {
                        "text": qsTr("DH Group 5"),
                        "value": "dh5"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "Perfect Forward Secrecy"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("Forward Secrecy")
            weight: 70
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Server (default)"),
                        "value": "server"
                    }, {
                        "text": qsTr("None"),
                        "value": "nopfs"
                    }, {
                        "text": qsTr("DH Group 1"),
                        "value": "dh1"
                    }, {
                        "text": qsTr("DH Group 2"),
                        "value": "dh2"
                    }, {
                        "text": qsTr("DH Group 5"),
                        "value": "dh5"
                    }]
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "Local Port"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("Local Port")
            weight: 80
            pageType: OceanUIObject.Editor
            page: D.SpinBox {
                value: root.dataMap.hasOwnProperty(oceanuiObj.name) ? parseInt(root.dataMap[oceanuiObj.name], 10) : 0
                from: 0
                to: 65535
                onValueChanged: {
                    if (root.dataMap[oceanuiObj.name] !== value) {
                        root.dataMap[oceanuiObj.name] = value
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "DPD idle timeout (our side)"
            parentName: root.parentName + "/vpncAdvancedGroup"
            displayName: qsTr("Disable Dead Peer Detection")
            weight: 90
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap[oceanuiObj.name] === "0"
                onClicked: {
                    if (checked) {
                        root.dataMap[oceanuiObj.name] = "0"
                    } else {
                        delete root.dataMap[oceanuiObj.name]
                    }
                    root.editClicked()
                }
            }
        }
    }
    // VPN Security
    OceanUITitleObject {
        name: "vpnSecurityTitle"
        parentName: root.parentName
        displayName: qsTr("VPN Security")
        weight: root.weight + 400
        visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
    }
    OceanUIObject {
        name: "vpnSecurityGroup"
        parentName: root.parentName
        weight: root.weight + 410
        visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "cipher"
            parentName: root.parentName + "/vpnSecurityGroup"
            displayName: qsTr("Cipher")
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: root.dataMap.hasOwnProperty("cipher") ? indexOfValue(root.dataMap["cipher"]) : 0
                model: [{
                        "text": qsTr("Default"),
                        "value": "default"
                    }, {
                        "text": qsTr("None"),
                        "value": "none"
                    }, {
                        "text": qsTr("DES-CBC"),
                        "value": "DES-CBC"
                    }, {
                        "text": qsTr("RC2-CBC"),
                        "value": "RC2-CBC"
                    }, {
                        "text": qsTr("DES-EDE-CBC"),
                        "value": "DES-EDE-CBC"
                    }, {
                        "text": qsTr("DES-EDE3-CBC"),
                        "value": "DES-EDE3-CBC"
                    }, {
                        "text": qsTr("DESX-CBC"),
                        "value": "DESX-CBC"
                    }, {
                        "text": qsTr("BF-CBC"),
                        "value": "BF-CBC"
                    }, {
                        "text": qsTr("RC2-40-CBC"),
                        "value": "RC2-40-CBC"
                    }, {
                        "text": qsTr("CAST5-CBC"),
                        "value": "CAST5-CBC"
                    }, {
                        "text": qsTr("RC2-64-CBC"),
                        "value": "RC2-64-CBC"
                    }, {
                        "text": qsTr("AES-128-CBC"),
                        "value": "AES-128-CBC"
                    }, {
                        "text": qsTr("AES-192-CBC"),
                        "value": "AES-192-CBC"
                    }, {
                        "text": qsTr("AES-256-CBC"),
                        "value": "AES-256-CBC"
                    }, {
                        "text": qsTr("CAMELLIA-128-CBC"),
                        "value": "CAMELLIA-128-CBC"
                    }, {
                        "text": qsTr("CAMELLIA-192-CBC"),
                        "value": "CAMELLIA-192-CBC"
                    }, {
                        "text": qsTr("CAMELLIA-256-CBC"),
                        "value": "CAMELLIA-256-CBC"
                    }, {
                        "text": qsTr("SEED-CBC"),
                        "value": "SEED-CBC"
                    }]
                onActivated: {
                    root.dataMap["cipher"] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty("cipher") ? indexOfValue(root.dataMap["cipher"]) : 0
            }
        }
        OceanUIObject {
            name: "auth"
            parentName: root.parentName + "/vpnSecurityGroup"
            displayName: qsTr("HMAC Auth")
            weight: 20
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: root.dataMap.hasOwnProperty("auth") ? indexOfValue(root.dataMap["auth"]) : 0
                model: [{
                        "text": qsTr("Default"),
                        "value": "default"
                    }, {
                        "text": qsTr("None"),
                        "value": "none"
                    }, {
                        "text": qsTr("RSA MD-4"),
                        "value": "RSA-MD4"
                    }, {
                        "text": qsTr("MD-5"),
                        "value": "MD5"
                    }, {
                        "text": qsTr("SHA-1"),
                        "value": "SHA1"
                    }, {
                        "text": qsTr("SHA-224"),
                        "value": "SHA224"
                    }, {
                        "text": qsTr("SHA-256"),
                        "value": "SHA256"
                    }, {
                        "text": qsTr("SHA-384"),
                        "value": "SHA384"
                    }, {
                        "text": qsTr("SHA-512"),
                        "value": "SHA512"
                    }, {
                        "text": qsTr("RIPEMD-160"),
                        "value": "RIPEMD160"
                    }]
                onActivated: {
                    root.dataMap["auth"] = currentValue
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty("auth") ? indexOfValue(root.dataMap["auth"]) : 0
            }
        }
    }
    // VPN Proxy
    OceanUITitleObject {
        name: "vpnProxyTitle"
        parentName: root.parentName
        displayName: qsTr("VPN Proxy")
        weight: root.weight + 500
        visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
    }
    OceanUIObject {
        name: "vpnProxyGroup"
        parentName: root.parentName
        weight: root.weight + 510
        visible: root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "proxy-type"
            parentName: root.parentName + "/vpnProxyGroup"
            displayName: qsTr("Proxy Type")
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: root.dataMap.hasOwnProperty("proxy-type") ? indexOfValue(root.dataMap["proxy-type"]) : 0
                model: [{
                        "text": qsTr("Not Required"),
                        "value": "none"
                    }, {
                        "text": qsTr("HTTP"),
                        "value": "http"
                    }, {
                        "text": qsTr("SOCKS"),
                        "value": "socks"
                    }]
                onActivated: {
                    root.dataMap["proxy-type"] = currentValue
                    dataMapChanged()
                    root.editClicked()
                }
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty("proxy-type") ? indexOfValue(root.dataMap["proxy-type"]) : 0
            }
        }
        OceanUIObject {
            name: "proxy-server"
            parentName: root.parentName + "/vpnProxyGroup"
            displayName: qsTr("Server IP")
            weight: 20
            visible: root.dataMap["proxy-type"] !== "none"
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "proxy-port"
            parentName: root.parentName + "/vpnProxyGroup"
            displayName: qsTr("Port")
            weight: 30
            visible: root.dataMap["proxy-type"] !== "none"
            pageType: OceanUIObject.Editor
            page: D.SpinBox {
                value: root.dataMap.hasOwnProperty(oceanuiObj.name) ? parseInt(root.dataMap[oceanuiObj.name], 10) : 0
                from: 0
                to: 65535
                onValueChanged: {
                    if (root.dataMap[oceanuiObj.name] !== value) {
                        root.dataMap[oceanuiObj.name] = value
                        root.editClicked()
                    }
                }
                Component.onCompleted: {
                    console.log("onCompleted-type:  ", root.dataMap["proxy-type"], root.dataMap["proxy-type"] !== "none", oceanuiObj.visible)
                }
            }
        }
        OceanUIObject {
            name: "proxy-retry"
            parentName: root.parentName + "/vpnProxyGroup"
            displayName: qsTr("Retry Indefinitely When Failed")
            weight: 40
            visible: root.dataMap["proxy-type"] !== "none"
            pageType: OceanUIObject.Editor
            page: switchItem
        }
        OceanUIObject {
            name: "http-proxy-username"
            parentName: root.parentName + "/vpnProxyGroup"
            displayName: qsTr("Username")
            weight: 50
            visible: root.dataMap["proxy-type"] === "http"
            pageType: OceanUIObject.Editor
            page: requiredLineEdit
        }
        OceanUIObject {
            name: "http-proxy-password"
            parentName: root.parentName + "/vpnProxyGroup"
            displayName: qsTr("Password")
            weight: 60
            visible: root.dataMap["proxy-type"] === "http"
            pageType: OceanUIObject.Editor
            page: NetPasswordEdit {
                text: root.secretMap.hasOwnProperty(oceanuiObj.name) ? root.secretMap[oceanuiObj.name] : ""
                dataItem: root
                onTextUpdated: root.secretMap[oceanuiObj.name] = text
            }
        }
    }
    // VPN TLS Authentication
    OceanUITitleObject {
        name: "vpnTLSAuthenticationTitle"
        parentName: root.parentName
        displayName: qsTr("VPN TLS Authentication")
        weight: 2000
        visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] !== "static-key")
    }
    OceanUIObject {
        name: "vpnTLSAuthenticationGroup"
        parentName: root.parentName
        weight: 2010
        visible: (root.vpnType & (NetUtils.VpnTypeEnum["openvpn"])) && (root.dataMap["connection-type"] !== "static-key")
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "tls-remote"
            parentName: root.parentName + "/vpnTLSAuthenticationGroup"
            displayName: qsTr("Subject Match")
            weight: 10
            pageType: OceanUIObject.Editor
            page: lineEdit
        }
        OceanUIObject {
            name: "remote-cert-tls"
            parentName: root.parentName + "/vpnTLSAuthenticationGroup"
            displayName: qsTr("Remote Cert Type")
            weight: 20
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    if (currentValue === "default") {
                        delete root.dataMap[oceanuiObj.name]
                    } else {
                        root.dataMap[oceanuiObj.name] = currentValue
                    }
                    root.editClicked()
                }
                model: [{
                        "text": qsTr("Default"),
                        "value": "default"
                    }, {
                        "text": qsTr("Client"),
                        "value": "client"
                    }, {
                        "text": qsTr("Server"),
                        "value": "server"
                    }]
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
            }
        }
        OceanUIObject {
            name: "ta"
            parentName: root.parentName + "/vpnTLSAuthenticationGroup"
            displayName: qsTr("Key File")
            weight: 30
            pageType: OceanUIObject.Editor
            page: fileLineEdit
        }
        OceanUIObject {
            name: "has_ta-dir"
            parentName: root.parentName + "/vpnTLSAuthenticationGroup"
            displayName: qsTr("Customize Key Direction")
            weight: 40
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: root.dataMap.hasOwnProperty("ta-dir")
                onClicked: {
                    if (checked) {
                        root.dataMap["ta-dir"] = "0"
                    } else {
                        delete root.dataMap["ta-dir"]
                    }
                    dataMapChanged()
                    root.editClicked()
                }
            }
        }
        OceanUIObject {
            name: "ta-dir"
            parentName: root.parentName + "/vpnTLSAuthenticationGroup"
            displayName: qsTr("Key Direction")
            weight: 50
            visible: root.dataMap.hasOwnProperty(this.name)
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                onActivated: {
                    root.dataMap[oceanuiObj.name] = currentValue
                    root.editClicked()
                }
                model: [{
                        "text": qsTr("0"),
                        "value": "0"
                    }, {
                        "text": qsTr("1"),
                        "value": "1"
                    }]
                Component.onCompleted: currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                Connections {
                    target: root
                    function onDataMapChanged() {
                        currentIndex = root.dataMap.hasOwnProperty(oceanuiObj.name) ? indexOfValue(root.dataMap[oceanuiObj.name]) : 0
                    }
                }
            }
        }
    }
}
