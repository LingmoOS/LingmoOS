/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQml.Models 2.12
import QtQuick.Layouts 1.12

import org.lingmo.menu.core 1.0
import org.lingmo.menu.extension 1.0
import "../extensions" as Extension
import AppControls2 1.0 as AppControls2

import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

ListView {
    id: root
    signal openFolderSignal(string folderId, string folderName, int x, int y)
    signal contentShowFinished()
    property bool isContentShow

    property int itemHeight: 40
    property alias sourceModel: appGroupModel.sourceModel
    property int cellWidth: 180
    property int cellHeight: 180
    property int column: Math.floor(width / cellWidth)

    spacing: 5
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    model: AppGroupModel {
        id: appGroupModel
    }

    delegate: Item {
        width: ListView.view.width
        height: childrenRect.height

        Column {
            width: parent.width
            height: childrenRect.height
            spacing: 0

            AppControls2.LabelItem {
                width: parent.width
                height: root.itemHeight
                displayName: model.name
                visible: displayName !== ""
            }

            GridView {
                anchors.horizontalCenter: parent.horizontalCenter
                width: root.cellWidth * root.column
                height: contentHeight
                cellWidth: root.cellWidth
                cellHeight: root.cellHeight
                interactive: false

                model: DelegateModel {
                    model: appGroupModel

                    Component.onCompleted: {
                        // Warning: rootIndex只需要设置一次，多次设置会导致对应关系错误，
                        // delegateModel使用persistedIndex保存rootIndex，会自动进行修正
                        rootIndex = modelIndex(index);
                    }

                    delegate: Item {
                        width: GridView.view.cellWidth
                        height: GridView.view.cellHeight

                        FullScreenAppItem {
                            id: appItem
                            anchors.fill: parent
                            anchors.margins: 12

                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            text.text: model.name
                            icon.source: model.icon
                            isRecentInstalled: model.recentInstall === undefined ? false : model.recentInstall

                            onClicked: (event) => {
                                if (event.button === Qt.LeftButton) {
                                    // openApplication
                                    appManager.launchApp(id);
                                } else if (event.button === Qt.RightButton) {
                                    menuManager.showMenu(model.id, MenuInfo.FullScreen);
                                }
                            }

                            drag.target: appItem
                            onPressed: {
                                fullScreenUI.focus = false;
                                grabToImage(function(result) {
                                    Drag.imageSource = result.url
                                })
                            }
                            Drag.supportedActions: Qt.CopyAction
                            Drag.dragType: Drag.Automatic
                            Drag.active: appItem.drag.active
                            Drag.hotSpot.x: width / 2
                            Drag.hotSpot.y: height / 2
                            Drag.mimeData: {"id": model.id, "favorite": model.favorite > 0}
                        }

                        // 收藏按钮
                        Extension.EditModeFlag {
                            isFavorited: model.favorite > 0
                            anchors.top: parent.top
                            anchors.topMargin: 10
                            anchors.right: parent.right
                            anchors.rightMargin: 28
                            onClicked: {
                                appManager.changeFavoriteState(id, !isFavorited);
                            }
                        }
                    }
                }
            }
        }
    }

    header: MouseArea {
        width: root.width
        height: childrenRect.height
        acceptedButtons: Qt.RightButton

        property var widgets: []
        property var widgetInfos: []
        property int widgetCount: 1

        Component.onCompleted: {
            widgetInfos.push({label: "favorite", display: "non-starred-symbolic", type: WidgetMetadata.Widget});
            widgets.push("favorite");
        }

        onClicked: {
            if (mouse.button === Qt.RightButton) {
                menu.open();
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
                    onClicked: favoriteModel.clearFavorites();
                }
            ]
        }

        Column {
            width: parent.width
            height: childrenRect.height + spacing
            spacing: root.spacing

            AppControls2.LabelItem {
                width: parent.width
                height: root.itemHeight
                displayName: qsTr("Favorite")
            }

            Item {
                width: parent.width
                height: favoriteView.height

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
                            visualModel.items.insert(addedIndex, {"id":"", "icon":"", "name": ""});

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

                            visualModel.items.move(addedIndex, dragIndex);
                            addedIndex = dragIndex;
                        }
                    }

                    onDropped: {
                        if (drop.getDataAsString("favorite") === "false") {
                            reset();
                            favoriteModel.addAppToFavorites(drop.getDataAsString("id"), addedIndex);
                        }
                    }
                    onExited: reset()
                }

                GridView {
                    id: favoriteView
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: root.cellWidth * root.column
                    height: contentHeight
                    property string mergeToAppId: ""
                    property bool isMergeToFolder: false
                    property bool dragTypeIsMerge: false
                    property int exchangedStartIndex: 0
                    property alias viewModel: visualModel

                    cellWidth: root.cellWidth
                    cellHeight: root.cellHeight
                    interactive: false

                    function indexAtNullItem() {
                        var nullItemIndex;
                        for (var i = 0; i < favoriteView.count; i ++) {
                            if (favoriteView.itemAtIndex(i).id === "") {
                                nullItemIndex = i;
                                return nullItemIndex;
                            }
                        }
                        return -1;
                    }

                    model: DelegateModel {
                        id: visualModel
                        model: favoriteModel
                        delegate: Item {
                            id: container
                            width: favoriteView.cellWidth
                            height: favoriteView.cellHeight
                            property var id: model.id
                            Extension.FavoriteDelegate {
                                anchors.fill: parent
                                anchors.margins: 12
                                isFullScreen: true

                                visualIndex: container.DelegateModel.itemsIndex
                                delegateLayout.anchors.topMargin: 16
                                delegateLayout.anchors.bottomMargin: 16
                                delegateLayout.spacing: 8
                                mergePrompt.anchors.topMargin: 10
                                mergePrompt.width: width*0.675
    //                            mergePrompt.width: 108
                                mergePrompt.radius: 32

                                Component.onCompleted: contentShowFinished.connect(resetOpacity)
                                Component.onDestruction: contentShowFinished.disconnect(resetOpacity)
                            }

                            //编辑模式标志
                            Extension.EditModeFlag {
                                id: tag
                                isFavorited: true
                                anchors.top: parent.top
                                anchors.topMargin: 10
                                anchors.right: parent.right
                                anchors.rightMargin: 28
                                onClicked: {
                                    visualModel.model.removeAppFromFavorites(id);
                                }
                            }
                        }
                    }

                    displaced: Transition {
                        NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad; duration: 200 }
                    }
                }
            }
        }
    }
}
