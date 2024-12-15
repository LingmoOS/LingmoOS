// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Window
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui.fcitx5configtool 1.0

Loader {
    id: loader
    active: false
    required property var viewModel
    signal selected(string data)

    sourceComponent: D.DialogWindow {
        id: imDialog
        width: 420
        height: 550
        icon: "preferences-system"
        modality: Qt.WindowModal
        ColumnLayout {
            spacing: 10
            width: parent.width
            Label {
                Layout.alignment: Qt.AlignHCenter
                font.bold: true
                text: qsTr("Add input method")
            }

            D.SearchEdit {
                id: searchEdit
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                placeholder: qsTr("Search")
                onTextChanged: {
                    console.log("search: ", text)
                    viewModel.filterText = text
                }
                onEditingFinished: {
                    console.log("search: ", text)
                    viewModel.filterText = text
                }
            }

            ListView {
                id: itemsView
                property string checkedIM
                property int checkedIndex: -1
                Layout.fillWidth: true
                height: 330
                clip: true
                model: loader.viewModel
                spacing: 10
                ButtonGroup {
                    id: langGroup
                }

                section.property: "language"
                section.criteria: ViewSection.FullString
                section.delegate: Rectangle {
                    width: ListView.view.width
                    height: childrenRect.height + 10
                    color: D.DTK.palette.window

                    required property string section

                    Text {
                        text: parent.section
                        x: 10
                        y: 5
                    }
                }

                delegate: Rectangle {
                    width: itemsView.width
                    height: checkDelegate.height
                    color: D.DTK.palette.base

                    D.CheckDelegate {
                        id: checkDelegate
                        width: parent.width
                        text: model.name
                        hoverEnabled: true
                        ButtonGroup.group: langGroup
                        onCheckedChanged: {
                            if (checked) {
                                console.log("checked", model.name, model.index)
                                itemsView.checkedIM = model.name
                                itemsView.checkedIndex = model.index
                            }
                        }
                    }
                }
                ScrollBar.vertical: ScrollBar {}
                boundsBehavior: Flickable.StopAtBounds
            }

            D.Button {
                text: qsTr("Find more in App Store")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                background: null
                textColor: D.Palette {
                    normal: D.DTK.makeColor(D.Color.Highlight)
                }

                onClicked: {
                    console.log("Find more in App Store button clicked")
                    oceanuiData.openLingmoAppStore()
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10
                Button {
                    Layout.bottomMargin: 10
                    text: qsTr("Cancel")
                    onClicked: {
                        imDialog.close()
                    }
                }
                Button {
                    Layout.bottomMargin: 10
                    text: qsTr("Add")
                    enabled: itemsView.checkedIM.length > 0
                    onClicked: {
                        console.log("add im: ", itemsView.checkedIM,
                                    itemsView.checkedIndex)
                        selected(itemsView.checkedIndex)
                        imDialog.close()
                    }
                }
            }
        }
        onClosing: {
            loader.active = false
            viewModel.filterText = ""
        }
    }

    onLoaded: {
        item.show()
    }
}
