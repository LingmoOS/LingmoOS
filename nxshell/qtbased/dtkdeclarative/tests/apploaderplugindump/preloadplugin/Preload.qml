// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.0
import org.deepin.dtk 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 100; height: 100

    DWindow.loadingOverlay: Rectangle {
        color: palette.window
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
