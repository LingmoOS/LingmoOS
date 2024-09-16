// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

Item {
    property real prefferImageScale: 0
    property bool running: fitImageAniamtion.running
    property Item targetImage: null

    // 缩放动画
    function scaleAnime(to) {
        prefferImageScale = to;
        moveAnimation.to = 0;
        scaleAnimation.to = to;
        fitImageAniamtion.start();
    }

    // 图像匹配动画效果
    ParallelAnimation {
        id: fitImageAniamtion

        NumberAnimation {
            id: moveAnimation

            duration: 200
            easing.type: Easing.OutExpo
            properties: "x,y"
            target: targetImage
        }

        NumberAnimation {
            id: scaleAnimation

            duration: 366
            easing.type: Easing.OutExpo
            properties: "scale"
            target: targetImage
        }
    }
}
