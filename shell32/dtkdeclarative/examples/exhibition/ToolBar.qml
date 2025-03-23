// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import ".."

Column {
    id: control
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "包括标题栏和普通工具栏，是要是一系列关联的动作集合。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    property int leftAreaWidth: 300
    property int rightAreaWidth: 600
    function toPercent(value) {
        return Number(value * 100).toFixed(0) + "%"
    }

    Flow {
        spacing: 30
        width: parent.width

        ImageCell {
            source: "qrc:/assets/toolbar/toolbar-1.svg"
            ApplicationWindow {
                DWindow.enabled: true
                flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
                width: 1000
                height: 140
                header: TitleBar {
                    icon.name: "music"
                    leftContent: ActionButton {
                        icon.name: "music"
                    }
                    content: RowLayout {
                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: 10
                            rightMargin: 10
                        }
                        Row {
                            spacing: 10
                            IconButton { icon.name: "arrow_ordinary_left" }
                            IconButton { icon.name: "arrow_ordinary_right" }
                        }
                        SearchEdit {
                            Layout.preferredWidth: 300
                            Layout.alignment: Qt.AlignRight
                            placeholder: "搜索"
                        }
                    }
                }
                Rectangle {
                    width: 800
                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                    }
                    color: Qt.rgba(0, 0, 0, 0.1)
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/toolbar/toolbar-2.svg"
            ApplicationWindow {
                DWindow.enabled: true
                flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
                width: 1000
                height: 140
                header: TitleBar {
                    icon.name: "music"
                    leftContent: ActionButton {
                        icon.name: "music"
                    }
                    content: Item {
                        SearchEdit {
                            anchors.centerIn: parent
                            width: 300
                            placeholder: "搜索"
                        }
                    }
                }
                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(0, 0, 0, 0.1)
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/toolbar/toolbar-3.svg"
            ApplicationWindow {
                DWindow.enabled: true
                flags: Qt.Window | Qt.WindowMinimizeButtonHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
                width: 1000
                height: 140
                header: TitleBar {
                    icon.name: "music"
                    leftContent: ActionButton {
                        icon.name: "music"
                    }
                }
                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(0, 0, 0, 0.1)
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/toolbar/toolbar-4.png"
            imageScale: 0.5
            ApplicationWindow {
                DWindow.enabled: true
                flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
                width: 1000
                height: 140
                header: TitleBar {
                    icon.name: "music"
                    title: "Title"
                    leftContent: ActionButton {
                        icon.name: "music"
                    }
                    textColor: Palette {
                        normal {
                            common: Qt.rgba(1, 1, 1, 0.7)
                            crystal: Qt.rgba(1, 1, 1, 0.9)
                        }
                        pressed {
                            common: DTK.makeColor(Color.Highlight)
                            crystal: DTK.makeColor(Color.Highlight)
                        }
                    }
                }
                Image {
                    width: parent.width
                    height: Window.height
                    anchors.bottom: parent.bottom
                    source: "qrc:/assets/toolbar/toolbar-4-2.png"
                }
            }
        }

        Item { width: parent.width; height: 1}

        FloatingPanel {
            contentItem: RowLayout {
                spacing: 10
                ToolButton { icon.name: "action_newfolder"; text: "文件夹"; ColorSelector.hovered: true }
                ToolButton { icon.name: "action_copy"; text: "复制" }
                ToolButton { icon.name: "action_share"; text: "分享" }
                ToolButton { icon.name: "action_compress"; text: "压缩" }
            }
        }

        FloatingPanel {
            contentItem: RowLayout {
                spacing: 20

                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}

                RowLayout {
                    spacing: 12
                    Slider {
                        id: toolBarSlider
                        property int totalValue: 7*60 + 56
                        property int currentValue: 3 * 60 + 25

                        Layout.preferredWidth: 230
                        highlightedPassedGroove: true
                        handleType: Slider.HandleType.NoArrowHorizontal
                        from: 0
                        to: msToValue(totalValue)
                        value: msToValue(currentValue)
                        stepSize: 1
                        function msToValue(ms) { return ms}
                        function msToText(ms) {
                            return String("%1:%2").arg(Math.floor(ms / 60)).arg(ms % 60)
                        }
                    }
                    Text {
                        Layout.preferredWidth: 50
                        text: String("%1/%2").arg(toolBarSlider.msToText(toolBarSlider.value)).arg(toolBarSlider.msToText(toolBarSlider.to))
                    }
                }

                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
            }
        }

        FloatingPanel {
            contentItem: RowLayout {
                spacing: 5
                ToolButton { icon.name: "search"; ColorSelector.hovered: true }
                ToolButton { icon.name: "search"; checked: true }
                ToolButton { icon.name: "search" }
                ToolButton { icon.name: "search" }
                ToolButton { icon.name: "search" }
                ToolButton { icon.name: "search" }
            }
        }
    }
}
