// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQml.Models
import org.deepin.dtk 1.0
//import ".."

FloatingPanel {
    //原生属性-开始
    id: idWindow
    width: 480; height: 93
    opacity: 0.9
    visible: false
    //原生属性-结束

    //自定义属性-开始
    //自定义属性-结束

    //窗口布局-开始
    ColumnLayout {
        anchors {
            fill: parent   //填充到父项
            topMargin: 12
            bottomMargin: 17
            leftMargin: 40
            rightMargin: 40
        }
        spacing: 0
        //标签-标题
        Label {
            id: labelTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            Layout.preferredHeight: 24
            height: 24
            font: DTK.fontManager.t5
            text: ""
            color: Qt.rgba(0,0,0)
        }

        Rectangle {
            Layout.fillWidth: true   //可拉伸
            Layout.fillHeight: true   //可拉伸
            Layout.preferredWidth: 0
            Layout.preferredHeight: 0
            opacity: 0
        }

        //标签-内容
        Label {
            id: labelContent
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            Layout.preferredHeight: 20
            font: DTK.fontManager.t6
            text: ""
            color: "#424241"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true   //可拉伸
            Layout.preferredWidth: 0
            Layout.preferredHeight: 5
            opacity: 0
        }

        //进度条
        ProgressBar {
            id: idProgressBar
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            Layout.preferredHeight: 8
            from: 0
            to: 100
            value: 0
        }
    }
    //窗口布局-结束

    //事件处理-开始
    MouseArea {
        id: idMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton

        //鼠标按下起始位置
        property point mousePosStart: "0,0"

        onPressed: (mouse)=> {
            mousePosStart = Qt.point(mouse.x, mouse.y)
        }

        onPositionChanged: (mouse)=> {
            if (idMouseArea.pressedButtons & Qt.LeftButton) {
                //计算位移
                var difference = Qt.point(mouse.x - mousePosStart.x, mouse.y - mousePosStart.y)
                var x = idWindow.x + difference.x
                var y = idWindow.y + difference.y
                //限制在父窗口内
                x = Math.max(0, x)
                y = Math.max(50, y)//50是AlbumtTitle的高度
                x = Math.min(x, idWindow.parent.width - idWindow.width)
                y = Math.min(y, idWindow.parent.height - idWindow.height)
                idWindow.x = x
                idWindow.y = y
            }
        }
    }
    //事件处理-结束

    //自定义函数-开始
    //显示
    function show() {
        idWindow.visible = true
        var x = (parent.width - idWindow.width) / 2
        var y = (parent.height - idWindow.height) / 2
        idWindow.x = x
        idWindow.y = y
    }

    //关闭
    function close() {
        idWindow.visible = false
    }

    //设置标题
    function setTitle(text) {
        labelTitle.text = text
    }

    //设置内容
    function setContent(text) {
        labelContent.text = text
    }

    //设置进度
    function setProgress(value, max) {
        idProgressBar.to = max
        idProgressBar.value = value
    }

    //清空显示内容
    function clear() {
        labelTitle.text = ""
        labelContent.text = ""
        idProgressBar.to = 100
        idProgressBar.value = 0
    }
    //自定义函数-结束
}
