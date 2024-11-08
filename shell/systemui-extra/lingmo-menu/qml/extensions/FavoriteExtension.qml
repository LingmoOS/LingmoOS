/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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
import QtQml.Models 2.1
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import org.lingmo.menu.extension 1.0
import AppControls2 1.0 as AppControls2
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import AppUI 1.0 as AppUI

LingmoUIMenuExtension {
    MouseArea {
        id: viewMouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (mouse.button === Qt.RightButton) {
                normalUI.focus = false;
                menu.open();
            } else {
                if (mainWindow.editMode) {
                    mainWindow.editMode = false;
                } else {
                    folderLoader.isFolderOpened = false;
                }
            }
        }

        LingmoUIItems.Menu {
            id: menu
            content: [
                LingmoUIItems.MenuItem {
                    text: qsTr("Enable editing mode")
                    onClicked: mainWindow.editMode = true;
                },
                LingmoUIItems.MenuItem {
                    text: qsTr("Remove all favorite apps")
                    onClicked: extensionData.favoriteAppsModel.clearFavorites();
                }
            ]
        }
        LingmoUIItems.StyleBackground {
            anchors.top: parent.top
            width: parent.width; height: 1
            useStyleTransparency: false
            alpha: 0.15
            paletteRole: Platform.Theme.Text
            visible: favoriteView.contentY > 0
            z: 1
        }

        FolderGridView {
            id: folderLoader
            anchors.fill: parent

            isFullScreen: false
            folderModel: extensionData.folderModel
            Component.onCompleted: favoriteView.openFolderSignal.connect(initFolder)
            Component.onDestruction: favoriteView.openFolderSignal.disconnect(initFolder)
        }

        Item {
            anchors.fill: parent
            anchors.bottomMargin: 8
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.topMargin: 8

            // 拖动到文件到空白区域 添加到收藏
            DropArea {
                anchors.fill: parent
                property int addedIndex: -1
                property string addedId: ""

                function reset() {
                    addedId = "";
                    var nullIndex = favoriteView.indexAtNullItem();
                    if (nullIndex > -1 && nullIndex < favoriteView.count) {
                        favoriteView.viewModel.items.remove(nullIndex);
                    }
                }

                onEntered: {
                    if (drag.getDataAsString("favorite") === "true") {
                        drag.accepted = false;

                    } else if (drag.getDataAsString("favorite") === "false") {
                        // 从左侧列表添加到收藏
                        // 已经触发从左侧拖动到收藏区域的事件，不需要再次添加空白item
                        if (addedId === drag.getDataAsString("id")) {
                            return;
                        }

                        addedId = drag.getDataAsString("id")
                        addedIndex = favoriteView.count;
                        favoriteView.viewModel.items.insert(addedIndex, {"id":"", "icon":"", "name": ""});

                    } else {
                        // 收藏插件内拖拽
                        favoriteView.dragTypeIsMerge = false;
                    }
                }

                onPositionChanged: {
                    if (drag.getDataAsString("favorite") === "false"  && addedIndex > -1) {
                        var dragIndex = favoriteView.indexAt(drag.x, drag.y);

                        if (dragIndex < 0) {
                            return;
                        }

                        favoriteView.viewModel.items.move(addedIndex, dragIndex);
                        addedIndex = dragIndex;
                    }
                }

                onDropped: {
                    if (drop.getDataAsString("favorite") === "false") {
                        reset();
                        extensionData.favoriteAppsModel.addAppToFavorites(drop.getDataAsString("id"), addedIndex);
                    }
                }

                onExited: reset()
            }

            FavoriteGridView {
                id: favoriteView
                width: parent.width
                height: (contentHeight > parent.height) ? parent.height : contentHeight
                interactive: contentHeight > parent.height
                isContentShow: !folderLoader.isFolderOpened

                Component.onCompleted: {
                    favoriteView.viewModel.model = extensionData.favoriteAppsModel
                    folderLoader.turnPageFinished.connect(contentShowFinished)
                }
                Component.onDestruction: folderLoader.turnPageFinished.disconnect(contentShowFinished)
             }
        }
    }
}
