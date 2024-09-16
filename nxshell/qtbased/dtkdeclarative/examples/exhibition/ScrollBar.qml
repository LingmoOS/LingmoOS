// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0

Column {
    id: panel
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "所有产生滚动的地方。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    Flow {
        spacing: 30
        width: parent.width

        Rectangle {
            border.color: "#cacaca"
            width: 300
            height: 213
            Flickable {
                width: 300
                height: 213
                contentWidth: 500
                contentHeight: 500
                clip: true

                ScrollBar.vertical: ScrollBar { }
                ScrollBar.horizontal: ScrollBar { }

                contentX: -width / 2
                contentY: -height / 2

                Label {
                    text: "Normal"
                    font: DTK.fontManager.t4
                }
            }
        }

        Item { width: parent.width; height: 1}

        Rectangle {
            id: frame
            clip: true
            width: 1000
            height: 278
            border.color: "black"
            Rectangle {
                id: content
                width: 450
                height: 1000
                x: (frame.width - content.width) / 2
                y: -vbar.position * content.height

                color: Qt.rgba(150 / 255.0, 255 / 255.0, 112 / 255.0, 0.3)

                Label {
                    anchors.centerIn: parent
                    text: "内容区域"
                }
            }

            ScrollBar {
                id: vbar
                orientation: Qt.Vertical
                size: frame.height / content.height
                position: 0.4
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
            }
        }
    }
}
