// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15

import org.deepin.ds.dock 1.0

Item {
    id: root
    required property list<string> windows
    required property int displayMode
    required property bool useColumnLayout
    required property AppItemPalette palette
    required property int dotWidth
    required property int dotHeight

    property int borderWidth: (displayMode === Dock.Fashion) ? 1 : 0

    width: indicatorLoader.width
    height: indicatorLoader.height

    Loader {
        id: indicatorLoader
        anchors.centerIn: parent
        sourceComponent: (windows.length > 1) ? multipleWindows : dot
    }

    Component {
        id: dot
        Rectangle {
            border.width: borderWidth
            border.color: palette.dotIndicatorBorder
            width: ((root.displayMode === Dock.Fashion || useColumnLayout) ? dotWidth : dotWidth / 2 - 1) + 2 * borderWidth
            height: ((root.displayMode === Dock.Fashion || !useColumnLayout) ? dotHeight : dotHeight / 2 - 1) +  2 * borderWidth
            color: palette.dotIndicator
            radius: (root.displayMode === Dock.Fashion || useColumnLayout) ? width / 2 : height / 2
        }
    }

    Component {
        id: multipleWindows
        Grid {
            columns: useColumnLayout ?  1 : 2
            rows: useColumnLayout ? 2 : 1
            flow: useColumnLayout ? GridLayout.LeftToRight : GridLayout.TopToBottom
            spacing: 2
            Loader { sourceComponent: dot }
            Loader { sourceComponent: dot }
        }
    }

}
