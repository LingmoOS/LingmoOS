// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import "../Utils"

BaseImageDelegate {
    id: delegate

    property bool needInit: true

    inputHandler: imageInput
    status: image.status
    targetImage: image

    AnimatedImage {
        id: image

        asynchronous: true
        cache: false
        clip: true
        fillMode: Image.PreserveAspectFit
        height: delegate.height
        scale: 1.0
        smooth: true
        source: delegate.source
        width: delegate.width
    }

    ImageInputHandler {
        id: imageInput

        anchors.fill: parent
        targetImage: image.status === Image.Ready ? image : null
    }

    // 动图在首次加载，状态变更为 Ready 时，paintedWidth 可能未更新，为0
    // 手动复位图片状态，调整缩放比例
    Connections {
        function onPaintedWidthChanged() {
            if (image.paintedWidth > 0) {
                needInit = false;
                delegate.reset();
            }
        }

        enabled: needInit
        target: image
    }
}
