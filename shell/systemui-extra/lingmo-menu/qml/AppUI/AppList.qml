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

import QtQuick 2.15
import QtQml 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import AppControls2 1.0 as AppControls2
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.menu.core 1.0

AppListView {
    id: appListView
    signal labelItemClicked()

    function resetListFocus() {
        listFocus = true;
    }

    // 在listview区域，鼠标进出和移动事件都会清空原有的按键焦点
    // 鼠标不移动，原有的鼠标悬浮三态会保留
    onContainsMouseChanged: { listFocus = false }
    onPositionChanged: { listFocus = false }

    delegate: Component {
        Loader {
            focus: true
            width: ListView.view ? ListView.view.width : 0
            height: appListView.itemHeight

            property int index: model.index
            property int type: model.type
            property string id: model.id
            property string name: model.name
            property string icon: model.icon
            property string comment: model.comment // label tooltip
            property bool favorite: model.favorite

            sourceComponent: Component {
                Item {
                    id: appItemBase
                    AppControls2.AppItem {
                        id: appItem
                        focus: true
                        width: appListView.view ? appListView.view.width : 0
                        height: appListView.itemHeight
                        anchors.centerIn: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        isRecentInstalled: model.recentInstall

                        Drag.hotSpot.x: 48 / 2
                        Drag.hotSpot.y: 48 / 2
                        Drag.supportedActions: Qt.CopyAction
                        Drag.dragType: Drag.Automatic
                        Drag.mimeData: {"id": id, "favorite": favorite}
                        Drag.onDragFinished: function (dropAction) {
                            appItem.Drag.active = false;
                        }

                        onClicked: {
                            if (mouse.button === Qt.RightButton) {
                                appListView.model.openMenu(index, MenuInfo.AppList);
                                return;
                            }
                            if (mouse.button === Qt.LeftButton) {
                                appManager.launchApp(id);
                                return;
                            }
                        }

                        onPressed: {
                            normalUI.focus = false;
                            // 在启动拖拽前进行截图，避免开始拖拽后没有图标
                            grabImage();
                        }

                        onPressAndHold: {
                            // 长按时启动拖拽
                            Drag.active =  true;
                        }
                    }

                    function grabImage() {
                        var icon = mouseGrabImage.createObject(appItem, {width: 48, height: 48, source: model.icon})
                        if (icon !== null) {
                            icon.grabToImage(function(result) {
                                appItem.Drag.imageSource = result.url
                                icon.destroy();
                            });
                            icon.visible = false;
                        }
                    }

                    Component {
                        id: mouseGrabImage
                        LingmoUIItems.Icon { }
                    }


                    Keys.onPressed: {
                        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            appManager.launchApp(id);
                        }
                    }
                }
            }
        }
    }

    view.section.property: "group"
    view.section.delegate: Component {
        AppControls2.LabelItem {
            width: ListView.view.width
            height: appListView.itemHeight
            focus: true
            displayName: section
            onClicked: labelItemClicked();
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    labelItemClicked();
                }
            }
        }
    }
}
