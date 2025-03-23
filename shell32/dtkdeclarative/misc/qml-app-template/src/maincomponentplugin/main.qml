// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

/**
        AppLoader 用于动态创建组件，它在 Preaload 创建显示
    出来后再进行创建操作，并将创建出来的组件放置在 Preload 的
    窗口内部。开发过程中不需要处理 AppLoader 的处理过程，对于
    需要使用到 Window 属性的情况，请使用其属性 AppLoader.window。

        AppLoader 中请将相互独立的组件放置在不同 Component
    中，将相互引用的组件放在在一个 Component 内，将全局属性或
    者创建逻辑不需要太复杂的控件放置在 AppLoader 的全局区域。

        AppLoader 能够配合 Loader 等动态创建的控件一起使用，
    在使用时，Loader 的 active 属性可以配合 AppLoader 的
    loaded 属性达到控件加载先后顺序的效果。
 */
AppLoader {
    Component {
        Row {
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            width: 500
            height: 50
            spacing: 5

            Repeater {
                id: repeater
                model: 10
                Rectangle {
                    color: (index == 0 || index == repeater.count - 1) ? "blue" : "cyan"
                    width: 50
                    height: 50
                    radius: Style.control.radius
                }
            }
        }
    }

    Component {
        Button {
            anchors.centerIn: parent
            text: qsTr("Button")
        }
    }

    Component {
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            width: 500
            height: 50
            spacing: 5

            Repeater {
                id: repeater
                model: 10
                Rectangle {
                    color: (index == 0 || index == repeater.count - 1) ? "red" : "cyan"
                    width: 50
                    height: 50
                    radius: Style.control.radius
                }
            }
        }
    }
}
