// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls 2.3
import QtQuick.Layouts
import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    // 键盘布局列表抬头
    OceanUIObject {
        id: keyboardLayoutTitle
        property bool isEditing: false
        name: "KeyboardLayoutTitle"
        parentName: "KeyboardLayout"
        displayName: qsTr("Keyboard layout")
        weight: 10
        pageType: OceanUIObject.Item
        page: RowLayout {
            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.leftMargin: 10
                font.bold: true
                font.pointSize: 14
                text: oceanuiObj.displayName
            }
            D.Button {
                id: button
                checkable: true
                checked: keyboardLayoutTitle.isEditing
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                text: keyboardLayoutTitle.isEditing ? qsTr("done") : qsTr("edit")
                font.pointSize: 13
                background: null
                textColor: D.Palette {
                    normal {
                        common: D.DTK.makeColor(D.Color.Highlight)
                        crystal: D.DTK.makeColor(D.Color.Highlight)
                    }
                }
                onCheckedChanged: {
                    keyboardLayoutTitle.isEditing = button.checked
                }
            }
        }
    }

    OceanUIObject {
        name: "KeyboardLayoutTips"
        parentName: "KeyboardLayout"
        displayName: qsTr("Add the corresponding input method in <a style='text-decoration: none;' href='Manage Input Methods'>Input Method Management</a> to ensure the keyboard layout works when aoceand or switched.")
        weight: 12
        pageType: OceanUIObject.Item
        page: D.Label {
            textFormat: Text.RichText
            text: oceanuiObj.displayName
            leftPadding: 10
            rightPadding: 10
            wrapMode: Text.WordWrap
            onLinkActivated: link => OceanUIApp.showPage(link)
        }
    }

    OceanUIObject {
        name: "KeyboardLayoutGroup"
        parentName: "KeyboardLayout"
        weight: 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIRepeater {
            model: oceanuiData.layoutCount
            delegate: OceanUIObject {
                name: "KeyboardLayoutItem" + index
                parentName: "KeyboardLayoutGroup"
                weight: 10 + index * 2
                pageType: OceanUIObject.Item
                backgroundType: OceanUIObject.Normal
                page: ItemDelegate {
                    id: itemDelegate
                    property bool isCurrentLang: oceanuiData.currentLayout === oceanuiData.userLayoutAt(index, true)
                    visible: oceanuiObj
                    hoverEnabled: true
                    implicitHeight: 40
                    // icon.name: oceanuiObj.icon
                    checkable: false

                    Label {
                        id: itemName
                        text: oceanuiData.userLayoutAt(index, true)
                        elide: Text.ElideRight
                        anchors {
                            left: itemDelegate.left
                            leftMargin: 10
                            top: itemDelegate.top
                            topMargin: (itemDelegate.height - itemName.height) / 2
                        }
                    }

                    D.IconButton {
                        id: removeButton
                        visible: itemDelegate.isCurrentLang || keyboardLayoutTitle.isEditing
                        icon.name: itemDelegate.isCurrentLang ? "sp_ok" : "list_delete"
                        icon.width: 24
                        icon.height: 24
                        implicitWidth: 36
                        implicitHeight: 36
                        anchors {
                            right: itemDelegate.right
                            rightMargin: 10
                            top: itemDelegate.top
                            topMargin: (itemDelegate.height - removeButton.height) / 2
                        }
                        background: null
                        onClicked: {
                            if (!keyboardLayoutTitle.isEditing || itemDelegate.isCurrentLang)
                                return

                            oceanuiData.deleteUserLayout(itemName.text)
                        }
                    }

                    background: OceanUIItemBackground {
                        separatorVisible: true
                    }

                    onClicked: {
                        if (keyboardLayoutTitle.isEditing)
                            return

                        // console.log("set current layout", itemName.text, oceanuiData.userLayoutAt(index, false))
                        oceanuiData.currentLayout = oceanuiData.userLayoutAt(index, false)
                    }
                }
            }
        }

        OceanUIObject {
            name: "KeyboardLayoutLastItem"
            parentName: "KeyboardLayoutGroup"
            weight: 999
            pageType: OceanUIObject.Item
            backgroundType: OceanUIObject.Normal
            page: Item {
                implicitHeight: 40
                D.Button {
                    id: addButton
                    implicitHeight: 40
                    text: qsTr("Add new keyboard layout...")
                    background: null
                    textColor: D.Palette {
                        normal {
                            common: D.DTK.makeColor(D.Color.Highlight)
                            crystal: D.DTK.makeColor(D.Color.Highlight)
                        }
                    }
                    LayoutsChooser {
                        id: layoutsChooser
                        viewModel: oceanuiData.layoutSearchModel()
                        onSelected: function (data) {
                            // console.log("selected data...", data)
                            oceanuiData.addUserLayout(data)
                        }
                    }

                    onClicked: {
                        keyboardLayoutTitle.isEditing = false
                        layoutsChooser.active = true
                    }
                }
            }
        }
    }
}
