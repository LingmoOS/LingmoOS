// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Column {
    id: panel
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "各种列表项展示，包括背景、单选复选、内容单行多行、图标。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    readonly property int itemDelegateMinWidth: 204
    readonly property int itemDelegateMaxWidth: 600
    readonly property int itemDelegateHeight: 40

    Flow {
        spacing: 10
        width: parent.width

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: 4
            delegate: ItemDelegate {
                text: "Nth " + index
                icon.name: "music"
                checked: index === ListView.view.count - 1
                backgroundVisible: index % 2 === 0
            }
        }



        ListView {
            id: subView
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: ListModel {
                ListElement { leftNode: true; }
                ListElement { leftNode: true; }
                ListElement { leftNode: true; }
                ListElement { leftNode: false; }
            }
            delegate: ItemDelegate {
                    text: "Nth " + index
                    icon.name: "music"
                    checked: index === ListView.view.count - 1
                    cascadeSelected: !leftNode
                    backgroundVisible: index % 2 === 0
                    Loader {
                        active: !leftNode

                        anchors {
                            left: parent.right
                            bottom: parent.bottom
                        }
                        sourceComponent: ListView {
                            implicitHeight: contentHeight
                            width: itemDelegateMinWidth
                            model: 4
                            delegate: ItemDelegate {
                                text: "Nth " + index
                                icon.name: "music"
                                checked: index === ListView.view.count - 1
                                backgroundVisible: index % 2 === 0
                            }
                        }
                }
            }
        }

        Item {width: parent.width; height: 40}

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: 4
            spacing: 2
            delegate: ItemDelegate {
                text: "Nth " + index
                icon.name: "music"
                checked: index === ListView.view.count - 1
                indicatorVisible: true
                backgroundVisible: true
                corners: getCornersForBackground(index, ListView.view.count)
            }
        }

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: 3
            spacing: 10
            delegate: ItemDelegate {
                text: "Nth " + index
                icon.name: "music"
                checked: index === ListView.view.count - 1
                indicatorVisible: true
                backgroundVisible: index % 2 === 0
                corners: getCornersForBackground(index, ListView.view.count)
            }
        }

        Item {width: parent.width; height: 40}

        ListModel {
            id: propertyModel
            ListElement { iconName: "music"; fileName: "fileName"; createDate: "2021.07.31 上午09：34"; fileSize: "3.6M"; fileFormat: "图像" }
            ListElement { iconName: "music"; fileName: "fileName"; createDate: "2021.07.31 上午09：34"; fileSize: "3.6M"; fileFormat: "图像" }
            ListElement { iconName: "music"; fileName: "fileName"; createDate: "2021.07.31 上午09：34"; fileSize: "3.6M"; fileFormat: "图像" }
            ListElement { iconName: "music"; fileName: "fileName"; createDate: "2021.07.31 上午09：34"; fileSize: "3.6M"; fileFormat: "图像" }
        }

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMaxWidth
            model: propertyModel

            delegate: ItemDelegate {
                width: itemDelegateMaxWidth
                text: fileName
                icon.name: iconName
                checked: index === ListView.view.count - 1
                backgroundVisible: index % 2 === 0

                contentFlow: true
                content: RowLayout {
                    Label { text: createDate }
                    Label { text: fileSize }
                    Label { text: fileFormat; Layout.fillWidth: true }
                }
            }
        }

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMaxWidth
            model: propertyModel

            delegate: ItemDelegate {
                width: itemDelegateMaxWidth
                text: fileName
                icon.name: iconName
                checked: index === ListView.view.count - 1
                backgroundVisible: index % 2 === 0

                content: RowLayout {
                    Label { text: createDate }
                    Label { text: fileSize }
                    Label { text: fileFormat }
                }
            }
        }

        Item {width: parent.width; height: 40}

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: 4
            spacing: 10

            ButtonGroup { id: singleExclusiveGroup }
            delegate: CheckDelegate {
                ButtonGroup.group: singleExclusiveGroup
                text: "标题"
                icon.name: "music"
                checked: index === ListView.view.count - 1
            }
        }

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: 4
            spacing: 10

            delegate: CheckDelegate {
                text: "标题"
                icon.name: "music"
                checked: index === ListView.view.count - 1
            }
        }

        Item {width: parent.width; height: 40}

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: 4
            spacing: 10
            ButtonGroup { id: doubleExclusiveGroup }
            delegate: CheckDelegate {
                icon.name: "music"
                checked: index === ListView.view.count - 1
                ButtonGroup.group: doubleExclusiveGroup
                content: Column {
                    Label {
                        text: "标题"
                    }
                    Label {
                        text: "描述信息"
                        font: DTK.fontManager.t8
                    }
                    Layout.fillWidth: true
                }
            }
        }

        ListView {
            implicitHeight: contentHeight
            width: itemDelegateMinWidth
            model: 4
            spacing: 10
            delegate: CheckDelegate {
                icon.name: "music"
                checked: index < ListView.view.count / 2
                content: Column {
                    Label {
                        text: "标题"
                    }
                    Label {
                        text: "描述信息"
                        font: DTK.fontManager.t8
                    }
                    Layout.fillWidth: true
                }
            }
        }
    }
}
