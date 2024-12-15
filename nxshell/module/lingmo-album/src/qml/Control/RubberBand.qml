// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls

//橡皮筋控件
//此处仅预留控制接口，具体控制逻辑由调用方编写

Item {

    signal rectSelChanged()
    //选择框当前的大小
    property int m_width: 0
    property int m_height: 0

    //上一次的坐标，外部调用者根据需要自行设置
    property int m_lastX: 0
    property int m_lastY: 0
    property int m_lastWidth: 0
    property int m_lastHeight: 0

    property int x1: -1
    property int y1: -1
    property int x2: -1
    property int y2: -1

    visible: false

    onX2Changed: {
        updateRect()
    }

    onY2Changed: {
        updateRect()
    }

    function isValid() {
        return x1 !== -1 && y1 !== -1 && x2 !== -1 && y2 !== -1
    }

    function _left() {
        return Math.min(x1, x2)
    }

    function _top() {
        return Math.min(y1, y2)
    }

    function _right() {
        return Math.max(x1, x2)
    }

    function _bottom() {
        return Math.max(y1, y2)
    }

    function updateRect() {
        if (x1 > x2)  {
            selArea.x = x2
        }
        else
            selArea.x = x1

        if (y1 > y2)
            selArea.y = y2
        else
            selArea.y = y1

        m_width = Math.abs(x1 - x2)
        m_height = Math.abs(y1 - y2)

        if (isValid())
            rectSelChanged()
    }

    function clearRect() {
        x1 = -1
        y1 = -1
        x2 = -1
        y2 = -1
    }

    Rectangle {
        id: selArea
        width: m_width
        height: m_height

        //TODO：颜色参数和透明度需要和UI确认好，目前DTK没有提供相关的控件，也无法获取系统设置的参数
        color: "#1E90FF"
        opacity: 0.4
    }
}
