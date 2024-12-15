// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

Item {
    id: fadeInoutAnimation
    signal stoped()
    property bool show: true
    property string idName: ""
    property real normalDuration: GStatus.animationDuration

    onOpacityChanged: {
        if ((opacity === 1 || opacity === 0) && !GStatus.loading) {
            stoped()
        }
    }

    // 大图预览退出/进入动效时间翻倍，需要慢速播放，才能清晰显示大图缩小动画
    onShowChanged: {
        if ((idName === "mainAlbumView")
                || (idName === "mainStack")) {
            normalDuration = GStatus.largeImagePreviewAnimationDuration
        } else {
            normalDuration = GStatus.animationDuration
        }
    }

    state: "hide"
    states: [
        State {
            name: "show"
            PropertyChanges {
                target: fadeInoutAnimation
                opacity: 1
                visible: true
            }
            when: show
        },
        State {
            name: "hide"
            PropertyChanges {
                target: fadeInoutAnimation
                opacity: 0
                visible: false
            }
            when: !show
        }
    ]

    transitions:
        Transition {
        enabled: GStatus.enableFadeInoutAnimation
        NumberAnimation{properties: "opacity,visible"; easing.type: Easing.OutExpo; duration: normalDuration
        }
    }
}
