/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import AppControls2 1.0 as AppControls2
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.menu.core 1.0

MouseArea {
    id: root

    readonly property alias view: listView
    property alias model: listView.model
    property alias delegate: listView.delegate
    property alias listFocus: listView.focus
    property int itemHeight: 40
    hoverEnabled: true
    clip: true

    LingmoUIItems.StyleBackground {
        anchors.top: parent.top
        width: parent.width
        height: 1
        useStyleTransparency: false
        alpha: 0.15
        paletteRole: Platform.Theme.Text
        visible: listView.contentY > 0
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        anchors.leftMargin: 4

        ListView {
            id: listView
            cacheBuffer: count * root.itemHeight
            spacing: 4
            Layout.fillHeight: true
            Layout.fillWidth: true
            highlightMoveDuration: 0
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: listViewScrollBar
            keyNavigationWraps: true

            // 焦点切换后，listView按键导航重新开始
            onCountChanged: resetCurrentIndex()
            function resetCurrentIndex() { currentIndex = 0 }
            Component.onCompleted: mainWindow.visibleChanged.connect(resetCurrentIndex)
            Component.onDestruction: mainWindow.visibleChanged.disconnect(resetCurrentIndex)
        }

        AppControls2.ScrollBar {
            id: listViewScrollBar
            Layout.fillHeight: true
            Layout.preferredWidth: 14
            visual: root.containsMouse
        }
    }
}
