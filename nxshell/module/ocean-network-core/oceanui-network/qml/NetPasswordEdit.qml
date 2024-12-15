// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0

D.PasswordEdit {
    property var dataItem
    property bool newInput: false
    property bool initialized: false
    signal textUpdated

    placeholderText: qsTr("Required")
    echoButtonVisible: newInput
    showAlert: dataItem.errorKey === oceanuiObj.name
    onTextChanged: {
        if (!initialized) {
            return
        }
        if (showAlert) {
            dataItem.errorKey = ""
        }
        if (text.length === 0) {
            newInput = true
        }
        textUpdated()
        dataItem.editClicked()
    }
    alertDuration: 2000
    onShowAlertChanged: {
        if (showAlert) {
            OceanUIApp.showPage(oceanuiObj)
            forceActiveFocus()
        }
    }
    Component.onCompleted: {
        newInput = text.length === 0
        initialized = true
    }
}
