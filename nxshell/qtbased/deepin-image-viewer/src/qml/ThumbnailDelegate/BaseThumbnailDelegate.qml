// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV

Item {
    id: thumbnailDelegate

    property int imgRadius: 3
    property bool isCurrentItem: parent.ListView.isCurrentItem
    property alias shader: enterShader
    property var source

    height: 40
    width: 30
    y: 20

    transitions: Transition {
        reversible: true

        // 保留最明显的宽度变更动画，其它属性变更忽略
        NumberAnimation {
            // 调整不同宽度下的动画时间，最少366ms
            duration: Math.max(width / 2, IV.GStatus.animationDefaultDuration)
            easing.type: Easing.OutExpo
            properties: "width, height, x, y"
        }

        ParallelAnimation {
            NumberAnimation {
                duration: Math.max(width / 2, IV.GStatus.animationDefaultDuration) + 100
                easing.type: Easing.OutExpo
                properties: "opacity, border.width"
                target: enterShader
            }

            ScriptAction {
                script: {
                    enterShader.aniamtionOffset = enterShader.visible ? 0 : -2;
                }
            }
        }
    }

    Rectangle {
        id: enterShader

        // 动画边框偏移，用于遮盖图片缩放时的白边
        property real aniamtionOffset: -2

        anchors.centerIn: parent

        // 如需改为匹配高亮色 palette.highlight
        border.color: "#0081FF"
        // 动画效果更新
        // border.width: imgRadius
        color: "transparent"
        height: parent.height + (2 * border.width) + aniamtionOffset
        opacity: 0
        radius: imgRadius * 2
        visible: false
        width: parent.width + (2 * border.width) + aniamtionOffset
    }
}
