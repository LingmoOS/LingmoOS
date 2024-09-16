// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0

import org.deepin.album 1.0 as Album

Item {
    property var selectedPaths: GStatus.selectedPaths // 已选路径

    function objIsEmpty(obj) {
        var ret = (String(obj) === "undefined" || String(obj) === "null")
        //console.log("obj is", ret ? "empty." : "not empty.", "objStr:", String(obj))
        return ret
    }

    function isDrag(fromX, fromY, toX, toY) {
        var length = Math.abs(fromX - toX) + Math.abs(fromY - toY);
        return length >= Qt.styleHints.startDragDistance;
    }

    // 导入重复文件提示
    Connections {
        target: albumControl
        function onSigRepeatUrls(urls) {
            if (urls.length > 0)
                DTK.sendMessage(stackControl, qsTr("The photo/video already exists"), "warning")
        }
    }

    // 导入不支持文件提示
    Connections {
        target: albumControl
        function onSigInvalidFormat() {
            DTK.sendMessage(stackControl, qsTr("The file format is not supported"), "warning")
        }
    }

    // 导入不支持文件提示
    Connections {
        target: FileControl
        function onInvalidFormat() {
            DTK.sendMessage(stackControl, qsTr("The file format is not supported"), "warning")
        }
    }

    onSelectedPathsChanged: {
        menuItemStates.updateMenuItemStates()
    }
}
