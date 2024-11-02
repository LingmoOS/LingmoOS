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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.lingmo.quick.widgets 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0
import org.lingmo.panel.taskManager 1.0 as LingmoUITaskManager

WidgetItem {
    Layout.objectName: "layout"
    Layout.fillHeight: true
    Layout.fillWidth: true

    onWidgetWindowChanged: {
        if (widgetWindow) {
            widgetWindow.onVisibleChanged.connect(updateWindowVisible);
        }
    }

    function updateWindowVisible() {
        taskManagerView.view.windowVisible = widgetWindow.visible;
    }

    GridLayout {
        id: gridLayout
        anchors.fill: parent
        columnSpacing: 5
        rowSpacing: 5
        columns: taskManagerView.view.isHorizontal ? 2 : 1
        rows: taskManagerView.view.isHorizontal ? 1 : 2

        // taskManager
        MouseArea {
            id: wheelArea
            Layout.fillHeight: true
            Layout.fillWidth: true
            enabled: {
                if (taskManagerView.view.isHorizontal) {
                    taskManagerView.view.contentWidth > taskManagerView.view.width
                } else {
                    taskManagerView.view.contentHeight > taskManagerView.view.height
                }
            }
            onWheel: (wheel) => {
                if (taskManagerView.view.isHorizontal) {
                    taskManagerView.view.flick(wheel.angleDelta.y * 5, 0)
                } else {
                    taskManagerView.view.flick(0, wheel.angleDelta.y * 5)
                }
            }

            LingmoUITaskManager.TaskManagerView {
                id: taskManagerView
                anchors.fill: parent
            }
        }

        // 超长滚动按钮
        StyleBackground {
            id: buttonBase
            Layout.preferredHeight: taskManagerView.view.isHorizontal ? 40 : 80
            Layout.preferredWidth: taskManagerView.view.isHorizontal ? 80 : 40 // TODO
            Layout.alignment: taskManagerView.view.isHorizontal ? Qt.AlignVCenter : Qt.AlignHCenter
            Layout.margins: 2

            paletteRole: Theme.Text
            useStyleTransparency: false
            alpha: 0.1
            radius: 12
            visible: {
                if (taskManagerView.view.isHorizontal) {
                    taskManagerView.view.contentWidth > taskManagerView.view.width
                } else {
                    taskManagerView.view.contentHeight > taskManagerView.view.height
                }
            }

            RowLayout {
                spacing: 0
                width: 80
                height: 40
                rotation: taskManagerView.view.isHorizontal ? 0 : 90
                anchors.centerIn: parent

                // 上一页
                MouseArea {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        taskManagerView.view.positionViewAtBeginning();
                    }
                    Icon {
                        width: 16; height: width
                        anchors.centerIn: parent
                        mode: {
                            if (taskManagerView.view.isHorizontal) {
                                return (taskManagerView.view.atXBeginning) ? Icon.Disabled : Icon.AutoHighlight;
                            } else {
                                return (taskManagerView.view.atYBeginning) ? Icon.Disabled : Icon.AutoHighlight;
                            }
                        }
                        source: "lingmo-start-symbolic" // TODO
                    }
                }

                StyleBackground {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    paletteRole: Theme.Text
                    useStyleTransparency: false
                    alpha: 0.15
                }

                // 下一页
                MouseArea {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        // taskManagerView.view.positionViewAtEnd()存在显示不全的问题
                        taskManagerView.view.positionViewAtIndex(taskManagerView.view.count - 1, ListView.End);
                    }
                    Icon {
                        width: 16; height: width
                        anchors.centerIn: parent
                        mode: {
                            if (taskManagerView.view.isHorizontal) {
                                return (taskManagerView.view.atXEnd) ? Icon.Disabled : Icon.AutoHighlight;
                            } else {
                                return (taskManagerView.view.atYEnd) ? Icon.Disabled : Icon.AutoHighlight;
                            }
                        }
                        source: "lingmo-end-symbolic"
                    }
                }
            }
        }
    }
}
