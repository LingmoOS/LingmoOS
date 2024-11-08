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

import QtQuick 2.15
import QtQml.Models 2.1
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import org.lingmo.menu.core 1.0
import org.lingmo.menu.extension 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import AppControls2 1.0 as AppControls2

GridView {
    id: favoriteView
    cellWidth: width / column
    cellHeight: cellWidth + 12
    signal openFolderSignal(string folderId, string folderName, int x, int y)
    signal contentShowFinished()
    property bool isContentShow

    property int spacing: 4
    property int column: 5
    property alias viewModel: visualModel

    property string mergeToAppId: ""
    property bool isMergeToFolder: false
    property bool dragTypeIsMerge: false
    property int exchangedStartIndex: 0

    state: isContentShow ? "contentShow" : "contentHidden"
    states: [
        State {
            name: "contentHidden"
            PropertyChanges { target: favoriteView; opacity: 0; scale: 0.95 }
        },
        State {
            name: "contentShow"
            PropertyChanges { target: favoriteView; opacity: 1; scale: 1 }
        }
    ]

    transitions: [
        Transition {
            to:"contentHidden"
            SequentialAnimation {
                PropertyAnimation { properties: "opacity, scale"; duration: 300; easing.type: Easing.InOutCubic }
                ScriptAction { script: favoriteView.visible = false }
            }
        },
        Transition {
            to: "contentShow"
            SequentialAnimation {
                ScriptAction { script: favoriteView.visible = true }
                PropertyAnimation { properties: "opacity, scale"; duration: 300; easing.type: Easing.InOutCubic }
            }
        }
    ]
    // 按键导航处理（左右键可以循环）
    focus: true
    onActiveFocusChanged: currentIndex = 0
    onCountChanged: currentIndex = 0
    Keys.onRightPressed: {
        if(currentIndex === count - 1) {
            currentIndex = 0;
            return;
        }
        currentIndex++;
    }
    Keys.onLeftPressed: {
        if(currentIndex === 0) {
            currentIndex = count - 1;
            return;
        }
        currentIndex--;
    }
    Keys.onDownPressed: {
        if(currentIndex > count - 1 - column) {
            return;
        }
        currentIndex = currentIndex + column;
    }
    Keys.onUpPressed: {
        if(currentIndex < column) {
            return;
        }
        currentIndex = currentIndex - column;
    }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad; duration: 200 }
    }

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
        delegate: Item {
            id: container
            focus: true
            width: favoriteView.cellWidth
            height: favoriteView.cellHeight
            property var id: model.id

            states: State {
                when: activeFocus
                PropertyChanges {
                    target: iconItem
                    alpha: 0.6
                }
            }
            Keys.onReturnPressed: {
                var data = {"id": model.id};
                send(data);
            }

            FavoriteDelegate {
                id: iconItem
                anchors.fill: parent
                anchors.margins: 2
                isFullScreen: false

                visualIndex: container.DelegateModel.itemsIndex
                mergePrompt.anchors.topMargin: 6
                mergePrompt.width: 52
                mergePrompt.radius: 14
                delegateLayout.anchors.leftMargin: 2
                delegateLayout.anchors.rightMargin: 2

                Component.onCompleted: favoriteView.contentShowFinished.connect(resetOpacity)
                Component.onDestruction: favoriteView.contentShowFinished.disconnect(resetOpacity)
            }

            //编辑模式标志
            EditModeFlag {
                isFavorited: true
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.rightMargin: 10
                onClicked: {
                    visualModel.model.removeAppFromFavorites(id);
                }
            }
        }
    }
}
