/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0

import Lingmo.FileManager 1.0 as FM
import LingmoUI 1.0 as LingmoUI
import "../"

Item {
    id: rootItem

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    GlobalSettings {
        id: globalSettings
    }

    Wallpaper {
        anchors.fill: parent
    }

    FM.FolderModel {
        id: dirModel
        url: desktopPath()
        isDesktop: true
        sortMode: globalSettings.sortMode
        viewAdapter: viewAdapter

        onCurrentIndexChanged: {
            _folderView.currentIndex = dirModel.currentIndex
        }

        onChangeIconSize: {
            _folderView.iconSize = size
            globalSettings.desktopIconSize = size
        }

        onChangeSortMode: {
            dirModel.sortMode = sortmode
            globalSettings.sortMode = sortmode
        }
    }

    FM.ItemViewAdapter {
        id: viewAdapter
        adapterView: _folderView
        adapterModel: dirModel
        adapterIconSize: 40
        adapterVisibleArea: Qt.rect(_folderView.contentX, _folderView.contentY,
                                    _folderView.contentWidth, _folderView.contentHeight)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: _folderView.forceActiveFocus()
    }

    FolderGridView {
        id: _folderView
        anchors.fill: parent

        isDesktopView: true
        iconSize: globalSettings.desktopIconSize
        maximumIconSize: globalSettings.maximumIconSize
        minimumIconSize: 22
        focus: true
        model: dirModel

        cellWidth: iconSize + 32
        cellHeight: iconSize + 48

        ScrollBar.vertical.policy: ScrollBar.AlwaysOff

        // Handle for topbar
        topMargin: 28

        // From dock
        leftMargin: Dock.leftMargin
        rightMargin: Dock.rightMargin
        bottomMargin: Dock.bottomMargin

        flow: GridView.FlowTopToBottom

        delegate: FolderGridItem {}

        onActiveFocusChanged: {
            if (!activeFocus) {
                _folderView.cancelRename()
                dirModel.clearSelection()
            }
        }

        Component.onCompleted: {
            dirModel.requestRename.connect(rename)
        }

        Connections {
            target: dirModel
            function onChangeIconSize(size) {
                var newSize = Math.min(Math.max(size, minimumIconSize), maximumIconSize)
                iconSize = newSize
                cellWidth = newSize + 32
                cellHeight = newSize + 48
            }
            function onChangeSortMode(sortmode) {
                switch (sortmode) {
                    case sortModeName:
                        // 按文件名排序
                        console.log("Sorting by name")
                        break;
                    case sortModeType:
                        // 按文件类型排序
                        console.log("Sorting by type")
                        break;
                    case sortModeModified:
                        // 按修改时间排序
                        console.log("Sorting by modified time")
                        break;
                    case sortModeSize:
                        // 按文件大小排序
                        console.log("Sorting by size")
                        break;
                    default:
                        // 默认排序
                        console.log("Using default sorting")
                        break;
                }
                sortMode = sortmode
            }
        }
    }

    FM.ShortCut {
        id: shortCut

        Component.onCompleted: {
            shortCut.install(_folderView)
        }

        onOpen: {
            dirModel.openSelected()
        }
        onCopy: {
            dirModel.copy()
        }
        onCut: {
            dirModel.cut()
        }
        onPaste: {
            dirModel.paste()
        }
        onRename: {
            dirModel.requestRename()
        }
        onOpenPathEditor: {
            folderPage.requestPathEditor()
        }
        onSelectAll: {
            dirModel.selectAll()
        }
        onDeleteFile: {
            dirModel.keyDeletePress()
        }
        onDeleteFileForever: {
            dirModel.keyDeleteForever()
        }
        onKeyPressed: {
            dirModel.keyboardSearch(text)
        }
        onShowHidden: {
            dirModel.showHiddenFiles = !dirModel.showHiddenFiles
        }
        onUndo: {
            dirModel.undo()
        }
    }

    Component {
        id: rubberBandObject

        FM.RubberBand {
            id: rubberBand

            width: 0
            height: 0
            z: 99999
            color: LingmoUI.Theme.highlightColor

            function close() {
                opacityAnimation.restart()
            }

            OpacityAnimator {
                id: opacityAnimation
                target: rubberBand
                to: 0
                from: 1
                duration: 150

                easing {
                    bezierCurve: [0.4, 0.0, 1, 1]
                    type: Easing.Bezier
                }

                onFinished: {
                    rubberBand.visible = false
                    rubberBand.enabled = false
                    rubberBand.destroy()
                }
            }
        }
    }
}
