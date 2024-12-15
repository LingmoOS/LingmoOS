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
    property bool proxyEnable: item.isEnabled
    property var config: item.config
    property bool urlAlert: false
    property bool portAlert: false

    function checkInput() {
        if (proxyEnable) {
            urlAlert = false
            var ipv4Pattern = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
            if (!ipv4Pattern.test(root.config.url)) {
                urlAlert = true
                return false
            }
            portAlert = false
            let port = parseInt(root.config.port, 10)
            if (isNaN(port) || port <= 0 || port > 65535) {
                portAlert = true
                return false
            }
        }
        return true
    }

    visible: item
    displayName: qsTr("Application Proxy")
    description: qsTr("Set up proxy servers")
    icon: "oceanui_app_agent"
    page: OceanUISettingsView {
        Component.onDestruction: {
            urlAlert = false
            portAlert = false
            proxyEnable = item.isEnabled
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
            icon: "oceanui_app_agent"
            weight: 10
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: proxyEnable
                enabled: item.enabledable
                onClicked: {
                    if (checked != proxyEnable) {
                        proxyEnable = checked
                    }
                    if (!proxyEnable) {
                        oceanuiData.exec(NetManager.SetConnectInfo, item.id, {
                                         "enable": false
                                     })
                    }
                }
            }
        }
        OceanUIObject {
            id: proxyConfig
            name: "config"
            parentName: root.name + "/body"
            weight: 20
            backgroundType: OceanUIObject.Normal
            visible: proxyEnable
            pageType: OceanUIObject.Item
            page: OceanUIGroupView {}
            OceanUIObject {
                name: "type"
                parentName: proxyConfig.parentName + "/" + proxyConfig.name
                displayName: qsTr("Proxy Type")
                weight: 10
                pageType: OceanUIObject.Editor
                page: ComboBox {
                    textRole: "text"
                    valueRole: "value"
                    currentIndex: indexOfValue(root.config.type)
                    onActivated: root.config.type = currentValue
                    model: [{
                            "value": "http",
                            "text": qsTr("http")
                        }, {
                            "value": "socks4",
                            "text": qsTr("socks4")
                        }, {
                            "value": "socks5",
                            "text": qsTr("socks5")
                        }]
                    Component.onCompleted: {
                        root.config = item.config
                    }
                    Connections {
                        target: root
                        function onConfigChanged() {
                            currentIndex = indexOfValue(root.config.type)
                        }
                    }
                }
            }
            OceanUIObject {
                name: "url"
                parentName: proxyConfig.parentName + "/" + proxyConfig.name
                displayName: qsTr("IP Address")
                weight: 20
                pageType: OceanUIObject.Editor
                page: D.LineEdit {
                    topInset: 4
                    bottomInset: 4
                    validator: RegularExpressionValidator {
                        regularExpression: /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
                    }
                    text: root.config.url
                    placeholderText: qsTr("Required")
                    showAlert: urlAlert
                    alertDuration: 2000
                    alertText: qsTr("Invalid IP address")
                    onShowAlertChanged: {
                        if (showAlert) {
                            OceanUIApp.showPage(oceanuiObj)
                            forceActiveFocus()
                        }
                    }
                    onTextChanged: {
                        if (root.config.url !== text) {
                            root.config.url = text
                        }
                    }
                }
            }
            OceanUIObject {
                name: "port"
                parentName: proxyConfig.parentName + "/" + proxyConfig.name
                displayName: qsTr("Port")
                weight: 30
                pageType: OceanUIObject.Editor
                page: D.LineEdit {
                    topInset: 4
                    bottomInset: 4
                    validator: IntValidator {
                        bottom: 0
                        top: 65535
                    }
                    text: root.config.port
                    placeholderText: qsTr("Required")
                    showAlert: portAlert
                    alertDuration: 2000
                    alertText: qsTr("Invalid port")
                    onShowAlertChanged: {
                        if (showAlert) {
                            OceanUIApp.showPage(oceanuiObj)
                            forceActiveFocus()
                        }
                    }
                    onTextChanged: {
                        if (root.config.port !== text) {
                            root.config.port = text
                        }
                    }
                }
            }
            OceanUIObject {
                name: "user"
                parentName: proxyConfig.parentName + "/" + proxyConfig.name
                displayName: qsTr("Username")
                weight: 40
                pageType: OceanUIObject.Editor
                page: D.LineEdit {
                    topInset: 4
                    bottomInset: 4
                    text: root.config.user
                    placeholderText: qsTr("Optional")
                    onTextChanged: {
                        if (root.config.user !== text) {
                            root.config.user = text
                        }
                    }
                }
            }
            OceanUIObject {
                name: "password"
                parentName: proxyConfig.parentName + "/" + proxyConfig.name
                displayName: qsTr("Password")
                weight: 50
                pageType: OceanUIObject.Editor
                page: D.PasswordEdit {
                    topInset: 4
                    bottomInset: 4
                    text: root.config.password
                    placeholderText: qsTr("Optional")
                    onTextChanged: {
                        if (root.config.password !== text) {
                            root.config.password = text
                        }
                    }
                }
            }
        }
        OceanUIObject {
            name: "ignoreHostsTips"
            parentName: root.name + "/body"
            displayName: qsTr("Check \"Use a proxy\" in application context menu in Launcher after configured")
            weight: 90
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
            visible: proxyEnable
            weight: 20
            pageType: OceanUIObject.Item
            page: Item {
                Layout.fillWidth: true
            }
        }
        OceanUIObject {
            name: "cancel"
            parentName: root.name + "/footer"
            visible: proxyEnable
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
                text: qsTr("Cancel")
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    proxyEnable = item.isEnabled
                    root.config = root.item.config
                }
            }
        }
        OceanUIObject {
            name: "Save"
            parentName: root.name + "/footer"
            visible: proxyEnable
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
                text: qsTr("Save")
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    if (!checkInput()) {
                        return
                    }
                    let config = {}
                    if (proxyEnable) {
                        config = root.config
                    }
                    config.enable = proxyEnable

                    oceanuiData.exec(NetManager.SetConnectInfo, item.id, config)
                }
            }
        }
    }
}
