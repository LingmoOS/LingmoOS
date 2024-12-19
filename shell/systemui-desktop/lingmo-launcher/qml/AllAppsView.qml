/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     Reoin Wong <reion@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI

import Lingmo.Launcher 1.0

ListView {
    id: control

    property bool searchMode: false

    property var sourceModel: launcherModel
    property var modelCount: sourceModel.count

    property int iconSize: root.iconSize + LingmoUI.Units.largeSpacing * 2
    property int cellWidth: iconSize + calcExtraSpacing(iconSize, control.width)
    property int cellHeight: iconSize + calcExtraSpacing(iconSize, control.height)

    property int rows: control.width / control.cellWidth
    property int columns: control.height / control.cellHeight
    property int pageCount: control.rows * control.columns

    orientation: ListView.Horizontal
    snapMode: ListView.SnapOneItem
    model: Math.ceil(control.modelCount / control.pageCount)

    maximumFlickVelocity: 10000
    highlightMoveDuration: 300

    preferredHighlightBegin: 0
    preferredHighlightEnd: 0
    highlightRangeMode: ListView.StrictlyEnforceRange
    highlightFollowsCurrentItem: true

    cacheBuffer: control.width * control.count
    boundsBehavior: Flickable.DragOverBounds
    currentIndex: -1
    clip: true

    NumberAnimation on contentX {
        id: scrollAnim

        duration: 300
        easing.type: Easing.InOutQuad

        onStopped: {
            var index = control.indexAt(control.contentX + control.width / 2,
                                        control.contentY + control.height / 2)
            if (index === -1)
                control.currentIndex = 0
            else
                control.currentIndex = index
        }
    }

    DropArea {
        anchors.fill: parent
        z: -1
    }

    MouseArea {
        anchors.fill: parent
        z: -1

        onClicked: {
            launcher.hideWindow()
        }

        onWheel: {
            if (wheel.angleDelta.y > 0)
                scrollPreviousPage()
            else
                scrollNextPage()
        }
    }

    delegate: GridView {
        id: _page

        width: control.width
        height: control.height

        readonly property int pageIndex: index

        cellHeight: control.cellHeight
        cellWidth: control.cellWidth

        interactive: false

        moveDisplaced: Transition {
            NumberAnimation {
                properties: "x, y"
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        model: PageModel {
            id: _pageModel
            sourceModel: launcherModel
            startIndex: control.pageCount * _page.pageIndex
            limitCount: control.pageCount
        }

        delegate: GridItemDelegate {
            searchMode: control.searchMode
            pageIndex: _page.pageIndex
            pageCount: control.pageCount
            width: control.cellWidth
            height: control.cellHeight
        }
    }

    function calcExtraSpacing(cellSize, containerSize) {
        var availableColumns = Math.floor(containerSize / cellSize)
        var extraSpacing = 0
        if (availableColumns > 0) {
            var allColumnSize = availableColumns * cellSize
            var extraSpace = Math.max(containerSize - allColumnSize, 0)
            extraSpacing = extraSpace / availableColumns
        }
        return Math.floor(extraSpacing)
    }

    function scrollNextPage() {
        if (scrollAnim.running)
            return

        if (currentIndex < count - 1) {
            scrollAnim.to = control.currentItem.x + control.width
            scrollAnim.restart()
        }
    }

    function scrollPreviousPage() {
        if (scrollAnim.running)
            return

        if (currentIndex > 0) {
            scrollAnim.to = control.currentItem.x - control.width
            scrollAnim.restart()
        }
    }
}
