// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

Item {
    property url imageSource
    property url imageSourceTemp

    function restart() {
        destroyAnimation.complete()
        createAnimation.start()
    }

    onImageSourceChanged: {
        destroyAnimation.start()
        createAnimation.start()
    }

    Image {
        id: toBeDeleted

        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        source: imageSourceTemp

        NumberAnimation on opacity {
            id: destroyAnimation

            from: 1
            to: 0
            duration: 2000
            easing.type: Easing.InOutQuad
        }
    }

    Image {
        id: toBeCreated

        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        source: imageSource

        NumberAnimation on opacity {
            id: createAnimation

            from: 0
            to: 1
            duration: 2000
            easing.type: Easing.InOutQuad

            onRunningChanged: {
                if (!running) {
                    imageSourceTemp = imageSource
                }
            }
        }
    }
}
