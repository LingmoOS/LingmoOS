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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *          hxf <hewenfei@kylinos.cn>
 *
 */
import QtQml 2.15
import QtQuick 2.12
import QtQml.Models 2.12
import QtQuick.Layouts 1.12

import org.lingmo.quick.widgets 1.0
import org.lingmo.panel.taskManager 1.0 as LingmoUITaskManager
import org.lingmo.quick.items 1.0 as Items
import org.lingmo.quick.platform 1.0 as Platform

// taskManager全部界面显示（包括预览图）
TaskManager {
    id: taskManager
    // 任务栏是否激活： 预览图是否显示、是否有高亮显示、是否有拖拽事件、是否打开右键菜单
    Widget.active: taskManagerActived || thumbnailView.visible || isDragging

    function menuAboutToHide() {
        viewContent.menuVisible = false;
        taskManagerActived = false;
        isTaskMenuShow = false;
    }

    Component.onCompleted: {
        taskManager.viewModel.menuAboutToHide.connect(menuAboutToHide)
    }
    Component.onDestruction: {
        taskManager.viewModel.menuAboutToHide.disconnect(menuAboutToHide)
    }

    state: ""
    states: [
        State {
            name: "aboutToShowThumbnail"
        },
        State {
            name: "aboutToHideThumbanil"
        }
    ]

    transitions: [
        Transition {
            to: "aboutToShowThumbnail"
            SequentialAnimation {
                PauseAnimation { duration: 400 }
                ScriptAction {
                    script: {
                        if (!thumbnailView.visible && !isTaskMenuShow) {
                            thumbnailView.updatePosition();
                            thumbnailView.show();
                            updateBlurRegionTimer.restart();
                        }
                    }
                }
            }
        },
        Transition {
            to: "aboutToHideThumbanil"
            SequentialAnimation {
                PauseAnimation { duration: 400 }
                ScriptAction {
                    script: {
                        if (thumbnailView.visible && (!taskItemData.mouse.containsMouse || isTaskMenuShow)) {
                            LingmoUITaskManager.TaskManager.activateWindowView([]);
                            thumbnailView.hide();
                        }
                    }
                }
            }
        }
    ]

    Timer {
        id: changedThumbnailData
        interval: 400
        onTriggered: {
            if (taskItemData.mouse.containsMouse) {
                viewContent.thumbnailData = taskItemData.dataToBeChanged;
            }
        }
    }
    onRequestThumbnailView: (show, data, mouse) => {
        taskItemData.mouse = mouse;
        if (show) {
            state = "aboutToShowThumbnail";
            if (thumbnailView.visible) {
                // timer
                taskItemData.dataToBeChanged = data;
                changedThumbnailData.restart();
            } else {
                viewContent.thumbnailData = data;
            }

        } else {
            state = "aboutToHideThumbanil";
        }
    }

    QtObject {
        id: taskItemData
        // 当前预览图对应的taskItem，通过containsMouse判断是否数据更新 / 解决X下预览图尺寸变化会触发隐藏的bug
        property Item mouse
        property var dataToBeChanged
    }

    Items.Dialog {
        id: thumbnailView
        mainItem: viewContent
        type: Platform.Settings.platformName === "wayland" ? Platform.WindowType.Switcher : Platform.WindowType.PopupMenu
        flags: Qt.WindowDoesNotAcceptFocus | Qt.FramelessWindowHint

        function updatePosition() {
            switch (Widget.container.position) {
                case Items.Types.Left:
                    thumbnailView.location = Items.Types.LeftEdge
                    break;
                case Items.Types.Top:
                    thumbnailView.location = Items.Types.TopEdge
                    break;
                case Items.Types.Right:
                    thumbnailView.location = Items.Types.RightEdge
                    break;
                case Items.Types.Bottom:
                    thumbnailView.location = Items.Types.BottomEdge
                    break;
                default:
                    break;
            }
            thumbnailView.margin = 10;
        }

        Items.WindowBlurBehind {
            id: blurRegions
            window: thumbnailView
            enable: true
            // 整个窗口的毛玻璃区域（为list模式时）
            Items.RectRegion {
                id: windowRegion
                x: 0
                y: 0
                width: 240
                height: 368
                radius: Platform.Theme.windowRadius
            }
        }

        ThumbnailWindow {
            id: viewContent

            property LingmoUITaskManager.TaskItemData thumbnailData: null
            property var currentWinIdList: thumbnailData ? thumbnailData.windowIdList : []
            property var windowTitles: thumbnailData ? thumbnailData.windowTitles : []
            property var windowIcons: thumbnailData ? thumbnailData.windowIcons : []
            property bool isHorizontal: taskManager.view.orientation === ListView.Horizontal
            property bool hasFocus: viewContent.containsMouse || viewContent.menuVisible
            property bool windowVisible: thumbnailView.visible

            onHasFocusChanged: {
                if (hasFocus) {
                    taskManager.state = "aboutToShowThumbnail";
                } else {
                    taskManager.state = "aboutToHideThumbanil";
                }
            }

            Timer {
                id: updateBlurRegionTimer
                interval: 50
                onTriggered: {
                    if (!viewContent.isList) {
                        blurRegions.clearRectRegion();
                        for (var i = 0; i < thumbnailModel.count; i ++) {
                            let item = viewContent.thumbnailLayout.itemAt(i);
                            if (item) {
                                blurRegions.addRectRegion(item.itemRegion);
                            }
                        }
                    } else {
                        blurRegions.clearRectRegion();
                        blurRegions.addRectRegion(windowRegion);
                    }
                }
            }

            // 更新预览图model数据 && 更新毛玻璃区域
            onCurrentWinIdListChanged: {
                updateBlurRegionTimer.stop();
                updateThumbnailModel();
                updateBlurRegionTimer.restart();
                viewContent.determineListViewSize();

                // 先设置mainItem的宽高 再切换visualParent,解决预览图闪烁问题
                if (viewContent.thumbnailData) {
                    if (viewContent.thumbnailData.currentTaskItem !== null) {
                        thumbnailView.visualParent = viewContent.thumbnailData.currentTaskItem;
                    }
                } else {
                    thumbnailView.visualParent = null;
                }
            }
            property var modelList: []
            ListModel {
                id: thumbnailModel
            }
            function updateThumbnailModel() {
                // 这里不能直接用=（js中表示引用），需要浅拷贝
                if (!currentWinIdList) {
                    return;
                }
                var listFromModel = modelList.slice(0);
                var listFromCurrent = currentWinIdList.slice(0);
                var indexOfRemoveList = [];

                for (var i = 0; i < listFromModel.length; i++) {
                   if (listFromCurrent.includes(listFromModel[i])) {
                       listFromCurrent.splice(listFromCurrent.indexOf(listFromModel[i]), 1);
                   } else {
                       indexOfRemoveList.push(i);
                   }
                }
                for (var i = indexOfRemoveList.length -1; i > -1; i--) {
                    listFromModel.splice(indexOfRemoveList[i], 1);
                    thumbnailModel.remove(indexOfRemoveList[i]);
                }

                for (var i = 0; i < listFromCurrent.length; i ++) {
                    listFromModel.push(listFromCurrent[i]);
                    thumbnailModel.append({"winId": listFromCurrent[i]});
                }

                modelList = listFromModel;
            }
        }
    }
}
