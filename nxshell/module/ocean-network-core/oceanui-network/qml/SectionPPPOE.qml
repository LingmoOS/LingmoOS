// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0

OceanUITitleObject {
    id: root
    property var config: null

    property string errorKey: ""
    signal editClicked

    function setConfig(c) {
        errorKey = ""
        root.config = c !== undefined ? c : {}
    }
    function getConfig() {
        return root.config
    }
    function checkInput() {
        if (!root.config.hasOwnProperty("username") || root.config.username.trim().length === 0) {
            errorKey = "username"
            return false
        }
        if (!root.config.hasOwnProperty("password") || root.config.password.length === 0) {
            errorKey = "password"
            return false
        }
        return true
    }

    name: "pppoeTitle"
    displayName: qsTr("PPPoE")
    OceanUIObject {
        name: "pppoeGroup"
        parentName: root.parentName
        weight: root.weight + 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "username"
            parentName: root.parentName + "/pppoeGroup"
            displayName: qsTr("Username")
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.LineEdit {
                placeholderText: qsTr("Required")
                text: root.config.hasOwnProperty("username") ? root.config.username : ""
                showAlert: errorKey === oceanuiObj.name
                alertDuration: 2000
                onTextChanged: {
                    if (showAlert) {
                        errorKey = ""
                    }
                    if (root.config.username !== text) {
                        root.config.username = text
                        root.editClicked()
                    }
                }
                onShowAlertChanged: {
                    if (showAlert) {
                        OceanUIApp.showPage(oceanuiObj)
                        forceActiveFocus()
                    }
                }
            }
        }
        OceanUIObject {
            name: "service"
            parentName: root.parentName + "/pppoeGroup"
            displayName: qsTr("Service")
            weight: 20
            pageType: OceanUIObject.Editor
            page: D.LineEdit {
                text: root.config.hasOwnProperty("service") ? root.config.service : ""
                onTextChanged: {
                    if (root.config.service !== text) {
                        root.config.service = text
                        root.editClicked()
                    }
                }
            }
        }
        OceanUIObject {
            name: "password"
            parentName: root.parentName + "/pppoeGroup"
            displayName: qsTr("Password")
            weight: 30
            pageType: OceanUIObject.Editor
            page: NetPasswordEdit {
                dataItem: root
                text: root.config.hasOwnProperty("password") ? root.config.password : ""
                onTextUpdated: root.config.password = text
            }
        }
    }
}
