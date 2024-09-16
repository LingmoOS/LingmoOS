// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS

BaseImageDelegate {
    id: delegate

    status: Image.Error

    Label {
        // 提示没有权限查看图片
        id: noPermissionLabel

        // 不同的前景色
        color: DS.Style.control.selectColor(undefined, Qt.rgba(0, 0, 0, 0.6), Qt.rgba(1, 1, 1, 0.6))
        font: DTK.fontManager.t6
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("You have no permission to view the image")
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignVCenter

        anchors.centerIn: parent
    }
}
