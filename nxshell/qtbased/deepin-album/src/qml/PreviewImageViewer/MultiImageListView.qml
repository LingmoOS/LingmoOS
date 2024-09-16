// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtGraphicalEffects 1.0

/**
 * @brief 用于 *.tif 等多页图的详细缩略图展示列表，在缩略图栏拓展展示
 *      宽度默认前后需要保留一份其它图片的缩略图，最大宽度不超过619px
 *      多页图列表项最小宽度10px,显示不全的图片将被隐藏
 */
Item {
    id: multiImage

    // 图片帧索引
    property alias frameIndex: listView.currentIndex
    // 图片信息
    property var source

    onSourceChanged: {
        console.log("multiImage source:", source)
    }

    // 通过信号抛出点击切换多页图帧号，不使用 frameIndex 变更信号，避免循环调用
    signal switchFrameIndex(var switchIndex)

    width: parent.width
    height: parent.height

    ListView {
        id: listView
        anchors.fill: parent

        height: multiImage.height
        width: multiImage.width
        // 计算调整的 item 宽度，item 宽度允许范围内处于 10px ~ 30px, coun一定 >=2
        property int preferredItemWidth: Math.min(Math.max(10, (width - 30) / (count - 1)), 30)

        // 使用范围模式，允许高亮缩略图在preferredHighlightBegin~End的范围外，使缩略图填充空白区域
        highlightRangeMode: ListView.ApplyRange
        highlightFollowsCurrentItem: true
        // 使焦点图片尽量显示在缩略图栏中部
        preferredHighlightBegin: width / 2 - 25
        preferredHighlightEnd: width / 2 + 25

        // 上层存在另一 ListView 底部缩略图栏，此 ListView 设置拖动不可超过边界，不影响二者的拖拽效果
        boundsBehavior: Flickable.StopAtBounds

        clip: true
        spacing: 1
        focus: true

        orientation: Qt.Horizontal
        cacheBuffer: 200

        // 根据当前多页图页数设置
        model: FileControl.getImageCount(source)
        // 子图片项
        delegate: Item {
            id: delegateItem

            height: listView.height
            width: listView.preferredItemWidth

            Image {
                id: img
                height: parent.height
                width: parent.width
                asynchronous: true
                // 多页图使用特定加载类
                source: "image://multiimage/" + multiImage.source + "#frame_" + index + "_thumbnail"
            }

            // 焦点图片边框
            Rectangle {
                id: imgBorder
                anchors.fill: parent
                color: listView.currentIndex === index ? "transparent" : Qt.rgba(0, 0, 0, 0.3)
                border.width: 0
                border.color: "white"
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    // 向外发送多页图帧号切换事件
                    switchFrameIndex(index)
                }
            }

            // 焦点状态的图片单独展示
            states: State {
                name: "active"
                when: listView.currentIndex === index

                PropertyChanges {
                    target: delegateItem
                    width: 30
                }

                PropertyChanges {
                    target: imgBorder
                    border.width: 1
                }
            }
        }

        // 滑动联动列表
        onCurrentIndexChanged: {
            // 特殊处理，防止默认显示首个缩略图时采用Center的策略会被遮挡部分
            if (0 == currentIndex) {
                positionViewAtBeginning()
            } else {
                // 尽可能将高亮缩略图显示在列表中
                positionViewAtIndex(currentIndex, ListView.Center)
            }
        }
    }
}
