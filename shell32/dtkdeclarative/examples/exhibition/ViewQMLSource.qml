// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.deepin.dtk 1.0

ColumnLayout {
    property url url

    onUrlChanged: {
        edit.text = globalObject.readFile(url)
    }

    Text {
        id: errorMessage
        visible: text
        color: "#ff5736"
        font: DTK.fontManager.t6
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        horizontalAlignment: Qt.AlignCenter
        Layout.fillWidth: true
    }

    RowLayout {
        id: layout
        Layout.fillWidth: true
        Layout.fillHeight: true

        ScrollView {
            id: codePreview
            Layout.fillHeight:  true
            Layout.preferredWidth: parent.width * 0.6
            clip: true
            Item {id: guest;}
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                id: edit

                property Item lastPreview: guest

                width: parent.width
                selectByMouse: true
                selectByKeyboard: true

                onTextChanged: {
                    try {
                        var obj = Qt.createQmlObject(edit.text, codePreview,  url.toString().replace(".qml", "_temporary.qml"))
                        globalObject.replace(lastPreview, obj)
                        lastPreview = obj
                        obj.width = codePreview.width
                        errorMessage.text = ""
                    } catch (error) {
                        errorMessage.text = String(error.lineNumber ? error.lineNumber : "未知") + "行，"
                                + String(error.columnNumber ? error.columnNumber : "未知") + "列：" + error.message
                    }
                }
            }
        }
    }
}
