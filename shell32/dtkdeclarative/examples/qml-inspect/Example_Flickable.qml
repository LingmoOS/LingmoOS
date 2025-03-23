// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Flow {
    id: control
    Row {
        spacing: 20
        ScrollView {
            id: scrollView
            width: 200
            height: 400
            clip: true
            ListView {
                model: 20
                delegate: Button {
                    text: "index" + String(modelData)
                }
            }
            background: Rectangle {
                border.color: "green"
            }
        }

        Column {
            Row {
                spacing: 20
                ProgressBar {
                    from: 0
                    to: 100
                    formatText: ("已下载" + (value / to * 100).toFixed() + "%（点击暂停）")
                    NumberAnimation on value {
                        from: 0
                        to: 100
                        duration: 1000
                        loops: Animation.Infinite
                    }
                }

                ProgressBar {
                    from: 0
                    to: 100
                    height: 8

                    NumberAnimation on value {
                        from: 0
                        to: 100
                        duration: 1000
                        loops: Animation.Infinite
                    }
                }

                EmbeddedProgressBar {
                    from: 0
                    to: 100
                    NumberAnimation on value {
                        from: 0
                        to: 100
                        duration: 1000
                        loops: Animation.Infinite
                    }
                }

                WaterProgressBar {
                    NumberAnimation on value {
                        id: valueAnimation
                        loops: Animation.Infinite
                        from: 0
                        to: 100
                        duration: 10000
                    }
                    running: valueAnimation.running
                }
            }

            Row {
                spacing: 20
                ProgressBar {
                    from: 0
                    to: 100
                    height: 18
                    indeterminate: true
                }

                ProgressBar {
                    from: 0
                    to: 100
                    indeterminate: true
                    formatText: "正在准备（点击终止）"

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            parent.animationStop = !parent.animationStop
                        }
                    }
                }
            }

            Column {
                spacing: 10
                IpV4LineEdit {

                }
                IpV4LineEdit {
                    width: 300
                    height: 40
                    showAlert: focus
                    alertText: "alert tips"
                }
                IpV4LineEdit {
                    width: 300
                    height: 40
                    text: "10.20.52.57"
                }

                Row {
                    spacing: 10
                    IpV4LineEdit {
                        id: idLineEditSetValueByText
                        width: 300
                        height: 40
                    }

                    Button {
                        text: "set IP by Text"
                        onClicked: idLineEditSetValueByText.text = "10.20.52.57"
                    }
                    Text {
                        text: idLineEditSetValueByText.text
                    }
                }
            }
        }
    }

    Column {
        width: 340
        height: 280

        RowLayout {
            TextField {
                id: nameFilter
                placeholderText: qsTr("Search by name...")
                Layout.fillWidth: true
                onTextChanged: sortFilterModel.update()
            }

            Column {
                RadioButton {
                    id: sortByName
                    checked: true
                    text: qsTr("Sort by name")
                    onCheckedChanged: sortFilterModel.update()
                }
                RadioButton {
                    text: qsTr("Sort by team")
                    onCheckedChanged: sortFilterModel.update()
                }
            }
        }

        SortFilterModel {
            id: sortFilterModel
            model: ListModel {
                ListElement { name: "Alice"; team: "Crypto" }
                ListElement { name: "Bob"; team: "Crypto" }
                ListElement { name: "Jane"; team: "QA" }
                ListElement { name: "Victor"; team: "QA" }
                ListElement { name: "Wendy"; team: "Graphics" }
            }
            delegate: Text {
                text: name + " (" + team + ")"
            }
            filterAcceptsItem: function(item) {
                return item.name.includes(nameFilter.text)
            }
            lessThan: function(left, right) {
                var leftVal = sortByName.checked ? left.name : left.team;
                var rightVal = sortByName.checked ? right.name : right.team;
                return leftVal < rightVal ? -1 : 1;
            }
        }

        ListView {
            height: 100
            width: parent.width
            model: sortFilterModel
        }
    }

    Column {
        KeySequenceEdit {
            width: 504
            height: 36
            text: "切换键盘布局"
            placeholderText: "请重新输入快捷键"
            keys: ["CTRL", "SHIFT"]
        }

        KeySequenceEdit {
            width: 504
            height: 36
            text: "关闭窗口"
            placeholderText: "请重新输入快捷键"
            keys: ["ALT", "F4"]
        }

        KeySequenceEdit {
            width: 504
            height: 36
            text: "关闭窗口"
            placeholderText: "请重新输入快捷键"
        }
    }
}
