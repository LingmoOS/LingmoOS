// Copyright (C) 2023 pengwenhao <pengwenhao@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS

ApplicationWindow {
    id: window
    visible: true
    width: 765
    height: 750
    title: qsTr("Wallpaper Settings")
    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    header: D.TitleBar {
        windowButtonGroup: Row {
            D.WindowButton {
                icon.name: "window_close"
                textColor: DS.Style.button.text
                onClicked: Qt.quit()
            }
        }
        DragHandler {
            onActiveChanged: if (active) window.startSystemMove()
            target: null
        }
    }

    Wallpaper {
        anchors.fill: parent
        anchors.leftMargin: 32
        anchors.rightMargin: 32
        anchors.topMargin: 16
        anchors.bottomMargin: 16
    }
}
