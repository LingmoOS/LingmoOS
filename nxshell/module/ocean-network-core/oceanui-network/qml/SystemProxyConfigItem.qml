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
    property var config: null
    property bool hasAuth: config.auth
    property bool hasUrl: false

    property string errorKey: ""
    signal editClicked

    function checkInput() {
        errorKey = ""
        if (hasAuth) {
            if (!root.config.user || root.config.user.length === 0) {
                errorKey = "user"
                return false
            }
            if (!root.config.password || root.config.password.length === 0) {
                errorKey = "password"
                return false
            }
        }
        return true
    }
    pageType: OceanUIObject.Item
    page: OceanUIGroupView {}
    OceanUIObject {
        name: "url"
        parentName: root.parentName + "/" + root.name
        displayName: root.displayName
        weight: 10
        pageType: OceanUIObject.Editor
        page: D.LineEdit {
            topInset: 4
            bottomInset: 4
            text: root.config.url
            placeholderText: qsTr("Optional")
            onTextChanged: {
                hasUrl = text.length !== 0
                if (root.config.url !== text) {
                    root.config.url = text
                }
            }
        }
    }
    OceanUIObject {
        name: "port"
        parentName: root.parentName + "/" + root.name
        displayName: qsTr("Port")
        weight: 20
        pageType: OceanUIObject.Editor
        page: D.LineEdit {
            topInset: 4
            bottomInset: 4
            validator: IntValidator {
                bottom: 0
                top: 65535
            }
            text: root.config.port
            placeholderText: qsTr("Optional")
            onTextChanged: {
                if (root.config.port !== text) {
                    root.config.port = text
                }
            }
        }
    }
    OceanUIObject {
        name: "auth"
        parentName: root.parentName + "/" + root.name
        displayName: qsTr("Authentication is required")
        weight: 30
        pageType: OceanUIObject.Editor
        page: Switch {
            checked: root.config.auth
            onClicked: {
                if (root.config.auth !== checked) {
                    root.config.auth = checked
                    hasAuth = root.config.auth
                }
            }
        }
    }
    OceanUIObject {
        id: user
        name: "user"
        parentName: root.parentName + "/" + root.name
        displayName: qsTr("Username")
        visible: hasAuth
        weight: 40
        pageType: OceanUIObject.Editor
        page: D.LineEdit {
            topInset: 4
            bottomInset: 4
            text: root.config.user
            placeholderText: qsTr("Required")
            showAlert: errorKey === oceanuiObj.name
            onTextChanged: {
                if (showAlert) {
                    errorKey = ""
                }
                if (root.config.user !== text) {
                    root.config.user = text
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
        name: "password"
        parentName: root.parentName + "/" + root.name
        displayName: qsTr("Password")
        visible: hasAuth
        weight: 50
        pageType: OceanUIObject.Editor
        page: NetPasswordEdit {
            dataItem: root
            text: root.config.password
            onTextUpdated: root.config.password = text
        }
    }
}
