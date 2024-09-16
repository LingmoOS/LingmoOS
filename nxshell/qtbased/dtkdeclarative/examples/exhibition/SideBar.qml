// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import ".."

Column {
    id: control
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "侧边栏用在一些不常用的功能上，多数情况下隐藏，在需要时才展示出来。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    Flow {
        spacing: 30
        width: parent.width

        Pane {
            width: 220
            height: 830
            topPadding: 50
            ColumnLayout {
                spacing: 20
                ButtonGroup { id: paneListGroup }

                SideBarItem {
                    title: "资料库"
                    group: paneListGroup
                    model: ListModel {
                        ListElement { checked: false; icon: "music"; text: "专辑"}
                        ListElement { checked: true; icon: "music"; text: "演唱者"}
                        ListElement { checked: false; icon: "music"; text: "所有音乐"}
                        ListElement { checked: false; icon: "music"; text: "我的收藏"}
                    }
                }

                SideBarItem {
                    id: playList
                    title: "播放列表"
                    group: paneListGroup
                    action: FloatingButton {
                        width: 20
                        height: 20
                        icon.name: "list-add"
                        onClicked: {
                            playList.model.append({checked: false, icon: "music", text: "自定义"})
                        }
                    }
                    model: ListModel {
                        ListElement { checked: false; icon: "music"; text: "校园民谣"}
                        ListElement { checked: false; icon: "music"; text: "怀旧经典音乐"}
                    }
                }
            }
            background: Rectangle {
                implicitWidth: 220
                implicitHeight: 400
                border.width: 1
                border.color: "#979797"
                color: "transparent"
            }
        }

        Pane {
            width: 220
            height: 830
            topPadding: 50
            ColumnLayout {
                spacing: 20
                ButtonGroup { id: paneListGroup2 }

                SideBarItem {
                    title: "收藏"
                    group: paneListGroup2
                    spacing: 5
                    itemSpacing: 5
                    itemFont: DTK.fontManager.t6
                    model: ListModel {
                        ListElement { checked: false; icon: "music"; text: "最近访问"}
                        ListElement { checked: true; icon: "music"; text: "主目录"}
                        ListElement { checked: false; icon: "music"; text: "桌面"}
                        ListElement { checked: false; icon: "music"; text: "文档"}
                        ListElement { checked: false; icon: "music"; text: "图片"}
                        ListElement { checked: false; icon: "music"; text: "视频"}
                        ListElement { checked: false; icon: "music"; text: "音乐"}
                        ListElement { checked: false; icon: "music"; text: "下载"}
                        ListElement { checked: false; icon: "music"; text: "回收站"}
                    }
                }

                SideBarItem {
                    title: "磁盘"
                    group: paneListGroup2
                    spacing: 5
                    itemSpacing: 5
                    itemFont: DTK.fontManager.t6
                    model: ListModel {
                        ListElement { checked: false; icon: "music"; text: "计算机"}
                        ListElement { checked: false; icon: "music"; text: "系统盘"}
                        ListElement { checked: false; icon: "music"; text: "百度云盘"}
                        ListElement { checked: false; icon: "music"; text: "Google Drive"}
                    }
                }

                SideBarItem {
                    title: "书签"
                    group: paneListGroup2
                    spacing: 5
                    itemSpacing: 5
                    itemFont: DTK.fontManager.t6
                    model: ListModel {
                        ListElement { checked: false; icon: "music"; text: "学习资料"}
                        ListElement { checked: false; icon: "music"; text: "V23设计"}
                    }
                }

                SideBarItem {
                    title: "标签"
                    group: paneListGroup2
                    spacing: 5
                    itemSpacing: 5
                    itemFont: DTK.fontManager.t6
                    model: ListModel {
                        ListElement { checked: false; icon: "music"; text: "设计文稿"}
                        ListElement { checked: false; icon: "music"; text: "V23设计"}
                    }
                }
            }
            background: Rectangle {
                implicitWidth: 220
                implicitHeight: 400
                border.width: 1
                border.color: "#979797"
                color: "transparent"
            }
        }
    }
}
