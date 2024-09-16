// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk 1.0

RowLayout {
    id: control
    spacing: 0

    property D.Palette textColor: DS.Style.button.text
    property bool fullScreenButtonVisible: true
    property bool embedMode: false
    signal maxOrWinded()

    property var __dwindow: Window.window.D.DWindow
    property bool __forceHind: !__dwindow.enabled || embedMode || Window.window.visibility === Window.FullScreen

    onMaxOrWinded: {
        if (!(__dwindow.motifFunctions & D.WindowManagerHelper.FUNC_MAXIMIZE)) {
            return
        }

        if (Window.window.visibility === Window.Maximized) {
            Window.window.visibility = Window.Windowed
        } else if (Window.window.visibility !== Window.FullScreen &&
                   maxOrWindedBtn.visible) {
            Window.window.visibility = Window.Maximized
        }
    }

    WindowButton {
        icon.name: "window_minimize"
        textColor: control.textColor
        property bool hasWindowFlag/*: (Window.window.flags & Qt.WindowMinimizeButtonHint)*/
        Component.onCompleted: hasWindowFlag = (Window.window.flags & Qt.WindowMinimizeButtonHint)

        visible: hasWindowFlag &&  !__forceHind
        enabled: (__dwindow.motifFunctions & D.WindowManagerHelper.FUNC_MINIMIZE)

        onClicked: {
            console.log("window minimize..")
            Window.window.visibility = Window.Minimized
        }
    }

    WindowButton {
        icon.name: "window_quit_full"
        textColor: control.textColor
        visible: !(!control.fullScreenButtonVisible ||
                    !__dwindow.enabled ||
                    Window.window.visibility !== Window.FullScreen)

        onClicked: {
            if (Window.window.visibility === Window.FullScreen) {
                Window.window.visibility = Window.Windowed
            } else {
                Window.window.visibility = Window.FullScreen
            }
        }
    }

    WindowButton {
        id: maxOrWindedBtn
        property bool isMaximized: Window.window.visibility === Window.Maximized
        icon.name: isMaximized ? "window_restore" : "window_maximize"

        textColor: control.textColor
        onClicked: maxOrWinded()

        property bool hasWindowFlag/*: (Window.window.flags & Qt.WindowMaximizeButtonHint)*/
        Component.onCompleted: hasWindowFlag = (Window.window.flags & Qt.WindowMaximizeButtonHint)

        readonly property size maxSize: Qt.size(Window.window.maximumWidth, Window.window.maximumHeight)
        readonly property size minSize: Qt.size(Window.window.minimumWidth, Window.window.minimumHeight)
        visible: (hasWindowFlag && !__forceHind &&
                    (__dwindow.motifFunctions & D.WindowManagerHelper.FUNC_RESIZE) &&
                    maxSize != minSize)

        enabled: ((__dwindow.motifFunctions & D.WindowManagerHelper.FUNC_MAXIMIZE) &&
                    (__dwindow.motifFunctions & D.WindowManagerHelper.FUNC_RESIZE))
    }

    WindowButton {
        icon.name: "window_close"
        textColor: control.textColor
        property bool hasWindowFlag/*: (Window.window.flags & Qt.WindowCloseButtonHint)*/
        Component.onCompleted: hasWindowFlag = (Window.window.flags & Qt.WindowCloseButtonHint)

        visible: hasWindowFlag && __dwindow.enabled
        enabled: __dwindow.motifFunctions & D.WindowManagerHelper.FUNC_CLOSE
        onClicked: {
            if (!FileControl.isAlbum()) {
                Window.window.close()
            }
            else {
                GStatus.stackControlCurrent = 0
                window.title = ""
            }
        }
    }
}
