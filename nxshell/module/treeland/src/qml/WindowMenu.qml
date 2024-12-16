// Copyright (C) 2024 pengwenhao <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS

D.Menu {
    id: menu

    signal requestMove
    signal requestMinimize
    signal requestMaximize(var max)
    signal requestClose
    signal requestResize(var edges)

    D.MenuItem {
        text: qsTr("Minimize")
        onTriggered: requestMinimize()
    }

    D.MenuItem {
        text: qsTr("Maximize")
        onTriggered: requestMaximize(true)
    }

    D.MenuItem {
        text: qsTr("Unmaximize")
        onTriggered: requestMaximize(false)
    }

    D.MenuItem {
        text: qsTr("Move")
        onTriggered: requestMove()
    }

    D.MenuItem {
        text: qsTr("Resize")
        onTriggered: requestResize(Qt.BottomEdge | Qt.RightEdge)
    }

    D.MenuItem {
        text: qsTr("Alway's on Top")
    }

    D.MenuItem {
        text: qsTr("Alway's on Visible Workspace")
    }

    D.MenuItem {
        text: qsTr("Move to Work Space Left")
        onTriggered: QmlHelper.shortcutManager.moveToNeighborWorkspace(-1)
    }

    D.MenuItem {
        text: qsTr("Move to Work Space Right")
        onTriggered: QmlHelper.shortcutManager.moveToNeighborWorkspace(1)
    }

    D.MenuItem {
        text: qsTr("Close")
        onTriggered: requestClose()
    }
}

