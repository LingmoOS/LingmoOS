// SPDX-FileCopyrightText: 2020 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import Qt.labs.platform 1.0
//sudo apt install qml-module-qt-labs-platform

Image {
    property string imagePath: StandardPaths.standardLocations(StandardPaths.HomeLocation) + "/.config/deepin/deepin-screensaver/saverpic/defaultpic.jpg"

    id: root
    anchors.fill: parent

    // If the source URL indicates a non-existing local file or resource, the Image element attempts to auto-detect the file extension.
    // If an existing file can be found by appending any of the supported image file extensions to the source URL, then that file will be loaded.
    source: imagePath

    // 图像将缩放以适应大小
    fillMode: Image.Stretch

    // 同步加载资源
    asynchronous: false

    // 忽略EXIF信息
    autoTransform: false

    // 居中显示
    horizontalAlignment: Image.AlignHCenter

    Component.onCompleted: {
        console.log(imagePath)
    }

    onStatusChanged: {
        if (root.status == Image.Null)
            console.log("no image has been set.")
        else if (root.status == Image.Ready)
            console.log("the image has been loaded.")
        else if (root.status == Image.Loading)
            console.log("the image is currently being loaded.")
        else if (root.status == Image.Error)
            console.log("an error occurred while loading the image.")
    }
}
