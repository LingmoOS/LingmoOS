// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQml

import org.deepin.dtk 1.0
//视频时长标签
Label {
    property string displayStr: ""

    text: displayStr

    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    property Palette textColor: Palette {
        normal: ("black")
        normalDark: ("white")
    }
    color: ColorSelector.textColor
    background: Rectangle {
        color: DTK.themeType === ApplicationHelper.LightType ? "#EEEEEE" : "#111111"
        radius: 20
        anchors.centerIn: parent
        width: parent.width + 10
        height: parent.height
    }
}


