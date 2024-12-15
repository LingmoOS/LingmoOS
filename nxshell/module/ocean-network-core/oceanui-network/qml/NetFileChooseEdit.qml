// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.lingmo.dtk 1.0 as D

RowLayout {
    id: root
    property var dataItem
    property var nameFilters: [qsTr("All files (*)")]
    property alias placeholderText: edit.placeholderText
    property alias text: edit.text
    property bool initialized: false
    signal textUpdated

    Component {
        id: fileDialog
        FileDialog {
            visible: false
            nameFilters: root.nameFilters
            onAccepted: {
                edit.text = currentFile.toString().replace("file://", "")
                dataItem.editClicked()
                this.destroy(10)
            }
            onRejected: this.destroy(10)
        }
    }
    D.LineEdit {
        id: edit
        onTextChanged: {
            if (!initialized) {
                return
            }
            if (showAlert) {
                dataItem.errorKey = ""
            }
            root.textUpdated()
            dataItem.editClicked()
        }
        showAlert: dataItem.errorKey === oceanuiObj.name
        alertDuration: 2000
        onShowAlertChanged: {
            if (showAlert) {
                OceanUIApp.showPage(oceanuiObj)
                forceActiveFocus()
            }
        }
        Component.onCompleted: initialized = true
    }
    NetButton {
        text: "..."
        onClicked: {
            fileDialog.createObject(this).open()
        }
    }
}
