// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
// 确保dtk的模块在最后被引入
import org.deepin.dtk 1.0

Column {
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "应用设置和控制中心部分设置快捷键的地方。"
        horizontalAlignment: Qt.AlignHCenter
    }

    Flow {
        spacing: 10
        width: parent.width

        KeySequenceEdit {
            width: 504
            height: 36
            text: "切换键盘布局"
            placeholderText: "请重新输入快捷键"
            keys: ["CTRL", "SHIFT"]
        }

        KeySequenceEdit {
            width: 504
            height: 36
            text: "关闭窗口"
            placeholderText: "请重新输入快捷键"
            keys: ["ALT", "F4"]
        }

        KeySequenceEdit {
            width: 504
            height: 36
            text: "关闭窗口"
            placeholderText: "请重新输入快捷键"
        }
    }
}
