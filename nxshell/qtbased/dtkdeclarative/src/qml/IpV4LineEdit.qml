// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

FocusScope {
    id: control
    property string text
    property string alertText
    property int alertDuration
    property bool showAlert
    property D.Palette backgroundColor: DS.Style.edit.background

    width: impl.width
    height: impl.height

    Control {
        id: impl
        anchors.fill: parent
        contentItem: RowLayout {
            Repeater {
                id: fields
                model: 4
                Layout.fillWidth: true
                delegate: TextInput {
                    KeyNavigation.right: index < fields.count - 1 ? fields.itemAt(index + 1) : null
                    KeyNavigation.left: index > 0 ? fields.itemAt(index - 1) : null
                    selectByMouse: true
                    color: impl.palette.text
                    selectionColor: impl.palette.highlight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: modelData
                    Layout.preferredWidth: DS.Style.ipEdit.fieldWidth
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    validator: RegExpValidator {
                        regExp: /^(([0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])?)$/
                    }
                    Loader {
                        active: index < fields.count - 1

                        sourceComponent: Label {
                            text: "."
                            anchors {
                                left: parent.right
                            }
                        }
                    }

                    onTextEdited: fields.updateText()
                }
                function updateText() {
                    var text = ""
                    for (var i = 0; i < fields.count; ++i) {
                        text += fields.itemAt(i).text
                        if (i < fields.count - 1)
                            text += "."
                    }
                    control.text = text == "..." ? "" : text
                }
                function clearText() {
                    for (var i = 0; i < fields.count; ++i) {
                        fields.itemAt(i).text = ""
                    }
                    control.text = ""
                }
                function updateByText() {
                    if (control.text === "")
                        clearText()
                    var arrs = control.text.split(".")
                    if (arrs.length != 4)
                        return
                    fields.model = arrs
                }

                Component.onCompleted: {
                    updateByText()
                    control.textChanged.connect(updateByText)
                }
            }

            Loader {
                id: clearBtn
                active: control.activeFocus && control.text
                width: height
                height: 36
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 8

                sourceComponent: ActionButton {
                    icon.name: "entry_clear"
                    focusPolicy: Qt.NoFocus
                    onClicked: fields.clearText()
                }
            }
        }

        background: EditPanel {
            control: impl
            alertText: control.alertText
            alertDuration: control.alertDuration
            showAlert: control.showAlert
            showBorder: control.activeFocus
            backgroundColor: control.backgroundColor
            implicitWidth: DS.Style.edit.width
            implicitHeight: DS.Style.edit.textFieldHeight
        }
    }
}
