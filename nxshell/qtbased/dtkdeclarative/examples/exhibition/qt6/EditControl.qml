// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtCore
import QtQuick
import QtQuick.Dialogs
import org.deepin.dtk 1.0

Column {
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "各种输入框，包括搜索框、单行输入框、多行输入框、密码输入框和数字输入框，他们都可以在指定条件下弹出警告框。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 20

    readonly property int editInputMinWidth: 304

    Flow {
        spacing: 20
        width: parent.width

        LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "shule"
            alertText: qsTr("长度大于等于3")
            showAlert: text.length >= 3
        }

        LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "editting"
        }

        LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            placeholderText: "必填"
            alertText: qsTr("不能为空")
            showAlert: true
        }

        Item {width: parent.width; height: 1}

        SearchEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            placeholder: "搜索"
            alertText: qsTr("alert tips")
            showAlert: text.length >= 3
        }

        SearchEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "deepin"
            editting: true
        }

        SearchEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            placeholder: "showAlert"
            alertText: qsTr("不能包含非法字符")
            showAlert: true
        }

        Item {width: parent.width; height: 1}

        IpV4LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "192.168.0.1"
        }

        IpV4LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "192.168.0.1"
        }

        IpV4LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "192.168.0.1"
            alertText: qsTr("ip地址格式不正确")
            showAlert: true
        }

        Item {width: parent.width; height: 1}

        PasswordEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "123456"
        }

        PasswordEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "123456"
            validator: RegularExpressionValidator {
                regularExpression: /[0-9]+/
            }
        }

        PasswordEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "12345a"
            alertText: "密码错误"
            showAlert: /[^0-9]+/.test(text)
        }

        Item {width: parent.width; height: 1}

        LineEdit {
            id: keyLineEdit
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "~/.ssh/ssh_keygen.key"
            ActionButton {
                anchors {
                    right: keyLineEdit.clearButton.left
                    rightMargin: 5
                    verticalCenter: parent.verticalCenter
                }
                icon.name: "entry_loadfile"
                onClicked: {
                    keyFileDialog.open()
                }
            }
            FileDialog {
                id: keyFileDialog
                title: "Please choose a file"
                currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
                onAccepted: {
                    keyLineEdit.text = keyFileDialog.urlToPath(keyFileDialog.selectedFile)
                }
            }
        }

        LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "~/.ssh/ssh_keygen.key"
            ActionButton {
                anchors {
                    right: parent.clearButton.left
                    rightMargin: 5
                    verticalCenter: parent.verticalCenter
                }
                icon.name: "entry_loadfile"
            }
        }

        LineEdit {
            width: Math.max(implicitWidth, editInputMinWidth)
            text: "~/.ssh/ssh_keygen.key"
            alertText: "文件非法"
            showAlert: true
            ActionButton {
                anchors {
                    right: parent.clearButton.left
                    rightMargin: 5
                    verticalCenter: parent.verticalCenter
                }
                icon.name: "entry_loadfile"

            }
        }

        Item {width: parent.width; height: 1}

        PlusMinusSpinBox {
            width: Math.max(implicitWidth, editInputMinWidth)
            spinBox.from: 0
            spinBox.to: 10
            spinBox.editable: true
            spinBox.alertText: qsTr("Can't include 1、3、5、7、9 number")
            spinBox.alertDuration: 2000  // millisecond
            spinBox.showAlert: /[13579]+/.test(spinBox.value)
        }

        PlusMinusSpinBox {
            width: Math.max(implicitWidth, editInputMinWidth)
            spinBox.from: 0
            spinBox.to: 10
            spinBox.editable: true
        }

        PlusMinusSpinBox {
            spinBox.editable: true
            spinBox.alertText: "只能输入数字"
            spinBox.showAlert: true
        }

        Item {width: parent.width; height: 1}

        PlusMinusSpinBox {
            spinBox.editable: true
            Component.onCompleted: forceActiveFocus()
        }

        PlusMinusSpinBox {
            spinBox.editable: true
            resetButtonVisible: true
            Component.onCompleted: forceActiveFocus()
        }

        Item {width: parent.width; height: 1}

        SpinBox { }

        Row {
            spacing: 10
            SpinBox {
                width: 100
                from: 0
                to: 23
                value: 8
                font.family: "DigifaceWide"
                font.pointSize: 44
                textFromValue: function (value, locale) {
                    return value < 10 ? "0" + Number(value) : value
                }
            }
            Label {
                text: ":"
                font.pointSize: 40
                verticalAlignment: Text.AlignVCenter
            }
            SpinBox {
                width: 100
                value: 24
                from: 0
                to: 59
                font.family: "DigifaceWide"
                font.pointSize: 44
                textFromValue: function (value, locale) {
                    return value < 10 ? "0" + Number(value) : value
                }
            }
        }

        Item {width: parent.width; height: 1}

        ListModel {
            id: mountPointModel
            ListElement {device: "/home"}
            ListElement {device: "/tmp"}
        }
        ComboBox {
            editable: true
            model: mountPointModel
        }

        ComboBox {
            editable: true
            model: mountPointModel
        }

        ComboBox {
            editable: true
            model: mountPointModel
            alertText: "不能包含非法字符"
            showAlert: true
        }

        Item {width: parent.width; height: 1}

        TextArea {
            text: "你好，这里是多行输入框"
            width: Math.max(implicitWidth, editInputMinWidth)
            height: 144
        }

        TextArea {
            text: "你好，这里是多行输入框"
            width: Math.max(implicitWidth, editInputMinWidth)
            height: 144
        }
    }
}
