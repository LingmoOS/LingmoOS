// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.network 1.0

OceanUITitleObject {
    id: root
    property var config: new Object()
    property string settingsID: ""

    property string errorKey: ""
    signal editClicked

    function setConfig(c) {
        errorKey = ""
        root.config = c
        settingsID = root.config.hasOwnProperty("id") ? root.config.id : ""
        root.configChanged()
    }
    function getConfig() {
        return root.config
    }
    function checkInput() {
        root.config.id = settingsID
        errorKey = ""
        console.log("root.config.id.length", root.config.id, root.config.id.length)
        if (root.config.id.length === 0) {
            errorKey = "id"
        }

        return errorKey.length === 0
    }
    onErrorKeyChanged: console.log("generic errorKey", errorKey)
    name: "genericTitle"
    displayName: qsTr("General")
    OceanUIObject {
        name: "genericGroup"
        parentName: root.parentName
        weight: root.weight + 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "name"
            parentName: root.parentName + "/genericGroup"
            displayName: root.config.type === "802-11-wireless" ? qsTr("Name (SSID)") : qsTr("Name")
            weight: 10
            enabled: !root.config.hasOwnProperty("id") || root.config.id.length === 0
            pageType: OceanUIObject.Editor
            page: D.LineEdit {
                text: settingsID
                placeholderText: qsTr("Required")
                onTextChanged: {
                    if (showAlert) {
                        errorKey = ""
                    }
                    if (settingsID !== text) {
                        root.editClicked()
                        settingsID = text
                    }
                }
                showAlert: errorKey === "id"
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
            name: "autoConnect"
            parentName: root.parentName + "/genericGroup"
            displayName: qsTr("Auto Connect")
            weight: 20
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: !root.config.hasOwnProperty("autoconnect") || root.config.autoconnect
                onClicked: {
                    root.config.autoconnect = checked
                    root.editClicked()
                }
            }
        }
    }
}
