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
 *          iaom <zhangpengfei@kylinos.cn>
 */

import QtQuick 2.15
import QtQml.Models 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.lingmo.quick.items 1.0

Item {
    id: root
    width: childrenRect.width
    height: childrenRect.height

    ListView {
        id: baseView
        property int dragStartIndex: 0
        property bool isBusy: false
        property bool dragIsContain: true

        width: childrenRect.width
        height: 100

        interactive: false
        orientation: ListView.Horizontal

        model: DelegateModel {
            id: visualModel
            model: proxyModel
            delegate: DropArea {
                id: dropArea
                property int order: DelegateModel.itemsIndex
                width: 100
                height: 100
                onEntered: {
                    if (drag.y > 95 || drag.y < 5) {
                        visualModel.items.move(baseView.count - 1, dropArea.order);
                    } else {
                        visualModel.items.move(drag.source.draggableOrder, dropArea.order);
                    }
                }
                onExited: {
                    if (drag.y > 95 || drag.y < 5) {
                        visualModel.items.move(drag.source.draggableOrder, baseView.count - 1);
                    }
                }

                Rectangle {
                    id: rect
                    width: 100
                    height: 100
                    ToolTip.text: model.Name
                    ToolTip.visible: mouseArea.containsMouse
                    ToolTip.delay: 300
                    color: "transparent"

                    Item {
                        id: draggableItem
                        property int draggableOrder: dropArea.order
                        width: 100
                        height: 100

                        Drag.supportedActions: Qt.CopyAction
                        Drag.dragType: Drag.Automatic
                        Drag.active: mouseArea.drag.active
                        Drag.hotSpot.x: icon.width / 2
                        Drag.hotSpot.y: icon.height / 2
                        Drag.onActiveChanged: {
                            if (Drag.active) {
                                draggableItem.parent = baseView;
                                draggableItem.opacity = 0;
                            }
                        }

                        function dragFinished() {
                            draggableItem.opacity = 1;
                            draggableItem.parent = rect;
                            draggableItem.x = 0;
                            draggableItem.y = 0;
                            proxyModel.setOrder(visualModel.model.index(baseView.dragStartIndex, 0), dropArea.order);
                        }

                        function grabImage() {
                            draggableItem.grabToImage(function(result) {
                                draggableItem.Drag.imageSource = result.url
                                return result;
                            })
                        }

                        ThemeIcon {
                            id: icon
                            width: 96
                            height: 96
                            source: model.Icon
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: mouseArea
                            property point mousePressPoint: Qt.point(0, 0)
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            drag.target: baseView.isBusy ? draggableItem : null

                            onPressed: {
                                draggableItem.grabImage();
                                baseView.isBusy = true;
                                baseView.dragStartIndex = draggableItem.draggableOrder;
                            }

                            onReleased: {
                                baseView.isBusy = false;
                                draggableItem.dragFinished();
                            }

                            onClicked: {
                                if (mouse.button === Qt.RightButton) {
                                    taskView.openMenu(model.Actions, index);
                                } else {
                                    if (CurrentDesktopWinIDList.length === 1) {
                                        taskModel.activateWindow(CurrentDesktopWinIDList[0]);
                                    }
                                }
                            }

                            onEntered: {
                                viewShowTimer.start();
                                viewHideTimer.stop();
                            }
                            onExited: {
                                viewHideTimer.start();
                                viewShowTimer.stop();
                            }
                        }
                    }
                    // *********Animation*********
                    PauseAnimation {
                        id: viewShowTimer
                        duration: 500
                        onFinished: {
                            if (CurrentDesktopWinIDList.length === 0) {
                                return;
                            }
                            if (baseView.isBusy) {
                                return;
                            }

                            var thumbnailPoint = mapToGlobal(width / 2, -8);
                            thumbnailView.viewPoint = thumbnailPoint;
                            thumbnailView.viewModel = CurrentDesktopWinIDList;
                            thumbnailView.show();
                        }
                    }

                    PauseAnimation {
                        id: viewHideTimer
                        duration: 300
                        onFinished: {
                            if (!thumbnailView.viewShow) {
                                thumbnailView.hide();
                            }
                        }
                    }
                }
            }
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                easing.type: Easing.OutQuad
                duration: 200
            }
        }

        add: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 400 }
            NumberAnimation { property: "scale"; from: 0; to: 1.0; duration: 400 }
        }
    }
}
