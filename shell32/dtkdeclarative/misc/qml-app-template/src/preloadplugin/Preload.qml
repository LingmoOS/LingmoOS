// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Window 2.0
import org.deepin.dtk 1.0
import "."

/**
        Preload 作为程序的主窗口被 AppLoader 提前加载，用于达到快速
    启动的功能。 在使用时，可以配合 DWindow 的 overlayExited属性，
    进行流畅化动画设计。

        Preload 中可以使用 ApplicationWindow; Window 和 DialogWindow
    作为主窗口，但请注意，每个程序只有一个 Preload 入口，而这个 Preload 入口
    将作为程序的主界面显示，当程序存在多个 Window 窗口时，请将主窗口作为 Preload
    窗口。

        Preload 中 loadingOverlay 作为流畅化过渡的组件属性，请不要在程序加载
    完成之后使用它，也不要管理它的生命周期，其生命周期仅会在窗口过渡阶段。 loadingOverlay
    属性除了可以使用一般控件之外，也能使用静态图，动态图等内容。当不指定其大小和位置时，
    默认情况进行主窗口填充。当指定大小后，将按照控件大小和位置进行布局。
 */
ApplicationWindow {
    id: window
    visible: true
    width: 900
    height: 700
    title: qsTr("qml-demo")
    flags: Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    header: TitleBar {}

    DWindow.enabled: true
    DWindow.loadingOverlay: Rectangle {
        color: palette.window
        BusyIndicator {
            id: indicator
            anchors.centerIn: parent
            running: true
            width: 64
            height: 64
        }
    }

    DWindow.overlayExited: Transition {
        NumberAnimation {
            properties: "scale"
            from: 1
            to: 0
            easing.type: Easing.InBack
        }
    }
}
