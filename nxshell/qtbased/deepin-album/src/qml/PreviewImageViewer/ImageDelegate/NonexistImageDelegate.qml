// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import org.deepin.image.viewer 1.0 as IV

BaseImageDelegate {
    id: delegate

    paintedPaddingWidth: (width - notExistImage.width) / 2
    status: notExistImage.status

    Component.onCompleted: {
        if (delegate.targetImageInfo.hasCachedThumbnail) {
            notExistImage.source = "image://ThumbnailLoad/" + delegate.source;
        } else {
            notExistImage.source = "qrc:/res/icon_import_photo.svg";
        }
    }

    Image {
        id: notExistImage

        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        height: 100
        mipmap: true
        smooth: true
        width: 100
    }

    Label {
        // 提示图片未找到信息
        id: notExistLabel

        // 不同的前景色
        color: DS.Style.control.selectColor(undefined, Qt.rgba(0, 0, 0, 0.6), Qt.rgba(1, 1, 1, 0.6))
        font: DTK.fontManager.t9
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Image file not found")
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignVCenter

        anchors {
            horizontalCenter: notExistImage.horizontalCenter
            top: notExistImage.bottom
            topMargin: 20
        }
    }
}
