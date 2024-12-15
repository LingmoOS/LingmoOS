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
    property int method: NetType.None
    property bool autoUrlAlert: false
    property int inputItem: 0

    function setItem(netItem) {
        item = netItem
        method = item.method
        autoUrlAlert = false
        // methodChanged()
    }

    visible: item
    displayName: qsTr("System Proxy")
    description: qsTr("Set up proxy servers")
    icon: "oceanui_system_agent"
    page: OceanUISettingsView {}

    OceanUIObject {
        name: "body"
        parentName: root.name
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "title"
            parentName: root.name + "/body"
            displayName: root.displayName
            icon: "oceanui_system_agent"
            weight: 10
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: item.isEnabled
                enabled: item.enabledable
                onClicked: {
                    if (checked) {
                        root.method = NetType.Auto
                    } else {
                        root.method = NetType.None
                        oceanuiData.exec(NetManager.SetConnectInfo, item.id, {
                                         "method": method
                                     })
                    }
                }
            }
            Connections {
                target: item
                function onMethodChanged(m) {
                    method = m
                }
            }
        }
        OceanUIObject {
            id: methodObj

            name: "method"
            parentName: root.name + "/body"
            displayName: qsTr("Proxy Type")
            weight: 20
            backgroundType: OceanUIObject.Normal
            visible: method !== NetType.None
            pageType: OceanUIObject.Editor
            page: ComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: indexOfValue(method)
                onActivated: {
                    method = currentValue
                }
                model: [{
                        "value": NetType.Auto,
                        "text": qsTr("Auto")
                    }, {
                        "value": NetType.Manual,
                        "text": qsTr("Manual")
                    }]
                Component.onCompleted: {
                    currentIndex = indexOfValue(method)
                }
                Connections {
                    target: root
                    function onMethodChanged() {
                        currentIndex = indexOfValue(method)
                    }
                }
            }
        }
        OceanUIObject {
            id: autoUrl
            property var config: item.autoProxy
            name: "autoUrl"
            parentName: root.name + "/body"
            displayName: qsTr("Configuration URL")
            weight: 30
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            visible: method === NetType.Auto
            page: D.LineEdit {
                topInset: 4
                bottomInset: 4
                placeholderText: qsTr("Required")
                text: oceanuiObj.config
                Layout.fillWidth: true
                showAlert: autoUrlAlert
                onTextChanged: {
                    autoUrlAlert = false
                    if (text.length === 0) {
                        inputItem &= 0x0f
                    } else {
                        inputItem |= 0x10
                    }

                    if (oceanuiObj.config !== text) {
                        oceanuiObj.config = text
                    }
                }
                onShowAlertChanged: {
                    if (showAlert) {
                        this.forceActiveFocus()
                    }
                }
            }
        }
        SystemProxyConfigItem {
            id: http
            name: "http"
            parentName: root.name + "/body"
            displayName: qsTr("HTTP Proxy")
            visible: method === NetType.Manual
            weight: 40
            config: root.item.manualProxy.http
            onHasUrlChanged: {
                if (hasUrl) {
                    inputItem |= 0x01
                } else {
                    inputItem &= ~0x01
                }
            }
        }
        SystemProxyConfigItem {
            id: https
            name: "https"
            parentName: root.name + "/body"
            displayName: qsTr("HTTPS Proxy")
            visible: method === NetType.Manual
            weight: 50
            config: root.item.manualProxy.https
            onHasUrlChanged: {
                if (hasUrl) {
                    inputItem |= 0x02
                } else {
                    inputItem &= ~0x02
                }
            }
        }
        SystemProxyConfigItem {
            id: ftp
            name: "ftp"
            parentName: root.name + "/body"
            displayName: qsTr("FTP Proxy")
            visible: method === NetType.Manual
            weight: 60
            config: root.item.manualProxy.ftp
            onHasUrlChanged: {
                if (hasUrl) {
                    inputItem |= 0x04
                } else {
                    inputItem &= ~0x04
                }
            }
        }
        SystemProxyConfigItem {
            id: socks
            name: "socks"
            parentName: root.name + "/body"
            displayName: qsTr("SOCKS Proxy")
            visible: method === NetType.Manual
            weight: 70
            config: root.item.manualProxy.socks
            onHasUrlChanged: {
                if (hasUrl) {
                    inputItem |= 0x08
                } else {
                    inputItem &= ~0x08
                }
            }
        }
        OceanUIObject {
            id: ignoreHosts
            property var config: root.item.manualProxy.ignoreHosts

            name: "ignoreHosts"
            parentName: root.name + "/body"
            visible: method === NetType.Manual
            weight: 80
            pageType: OceanUIObject.Item
            page: TextArea {
                wrapMode: TextEdit.WordWrap
                text: oceanuiObj.config
                onTextChanged: {
                    if (oceanuiObj.config !== text) {
                        oceanuiObj.config = text
                    }
                }
            }
        }
        OceanUIObject {
            name: "ignoreHostsTips"
            parentName: root.name + "/body"
            displayName: qsTr("Ignore the proxy configurations for the above hosts and domains")
            weight: 90
            visible: ignoreHosts.visibleToApp
            pageType: OceanUIObject.Item
            page: Label {
                text: oceanuiObj.displayName
                wrapMode: Text.WordWrap
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
            visible: method !== NetType.None
            weight: 20
            pageType: OceanUIObject.Item
            page: Item {
                Layout.fillWidth: true
            }
        }
        OceanUIObject {
            name: "cancel"
            parentName: root.name + "/footer"
            visible: method !== NetType.None
            displayName: qsTr("Cancel")
            weight: 30
            pageType: OceanUIObject.Item
            page: Button {
                implicitHeight: implicitContentHeight + 10
                implicitWidth: implicitContentWidth + 10
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                spacing: 0
                text: oceanuiObj.displayName
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    method = root.item.method
                    autoUrl.config = root.item.autoProxy
                    http.config = root.item.manualProxy.http
                    https.config = root.item.manualProxy.https
                    ftp.config = root.item.manualProxy.ftp
                    socks.config = root.item.manualProxy.socks
                    ignoreHosts.config = root.item.manualProxy.ignoreHosts
                }
            }
        }
        OceanUIObject {
            name: "Save"
            parentName: root.name + "/footer"
            displayName: qsTr("Save")
            visible: method !== NetType.None
            weight: 40
            pageType: OceanUIObject.Item
            page: Button {
                implicitHeight: implicitContentHeight + 10
                implicitWidth: implicitContentWidth + 10
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                spacing: 0
                enabled: (method === NetType.Auto && (inputItem & 0xf0)) || (method === NetType.Manual && (inputItem & 0x0f))
                text: oceanuiObj.displayName
                Layout.alignment: Qt.AlignRight
                function printfObj(obj) {
                    for (let k in obj) {
                        console.log(k, obj[k])
                    }
                }

                onClicked: {
                    console.log("save:", method)
                    console.log("autoUrl.config :", autoUrl.config)
                    console.log("http.config:", http.config)
                    printfObj(http.config)
                    console.log("https.config:", https.config)
                    printfObj(https.config)
                    console.log("ftp.config:", ftp.config)
                    printfObj(ftp.config)
                    console.log("socks.config:", socks.config)
                    printfObj(socks.config)
                    console.log("ignoreHosts.config:", ignoreHosts.config)
                    let config = {}
                    switch (method) {
                    case NetType.None:
                        break
                    case NetType.Auto:
                        autoUrlAlert = false
                        if (autoUrl.config.length === 0) {
                            autoUrlAlert = true
                            return
                        }

                        config["autoUrl"] = autoUrl.config
                        break
                    case NetType.Manual:
                        if (!http.checkInput() || !https.checkInput() || !ftp.checkInput() || !socks.checkInput()) {
                            return
                        }
                        config["httpAddr"] = http.config["url"]
                        config["httpPort"] = http.config["port"]
                        config["httpAuth"] = http.config["auth"]
                        config["httpUser"] = http.config["user"]
                        config["httpPassword"] = http.config["password"]

                        config["httpsAddr"] = https.config["url"]
                        config["httpsPort"] = https.config["port"]
                        config["httpsAuth"] = https.config["auth"]
                        config["httpsUser"] = https.config["user"]
                        config["httpsPassword"] = https.config["password"]

                        config["ftpAddr"] = ftp.config["url"]
                        config["ftpPort"] = ftp.config["port"]
                        config["ftpAuth"] = ftp.config["auth"]
                        config["ftpUser"] = ftp.config["user"]
                        config["ftpPassword"] = ftp.config["password"]

                        config["socksAddr"] = socks.config["url"]
                        config["socksPort"] = socks.config["port"]
                        config["socksAuth"] = socks.config["auth"]
                        config["socksUser"] = socks.config["user"]
                        config["socksPassword"] = socks.config["password"]
                        config["ignoreHosts"] = ignoreHosts.config
                        break
                    }
                    config["method"] = method
                    oceanuiData.exec(NetManager.SetConnectInfo, item.id, config)
                }
            }
        }
    }
}
