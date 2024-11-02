/*
 *
 *  Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQml.Models 2.15
import QtQuick.Layouts 1.12
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.panel.taskManager 1.0 as LingmoUITaskManager

Item {
    id: root
    property bool isDragged: false
    property alias model: visualModel.model
    property alias orientation: normalView.orientation
    property alias spacing: normalView.spacing
    property int itemSize: 48

    signal toNormalFinished()
    signal toDraggingFinished()
    signal requestThumbnailView(bool show, TaskItemData data, Item item)

    function indexAt(x,  y) {
        let idx = normalView.normalViewItemIdx(x, y, normalView.spacing/2, true);
        if (idx === -1) {
            idx = normalView.normalViewItemIdx(x, y, normalView.spacing/2, false);
        }

        return idx;
    }

    function updateIconGeometry() {
        for (var i = 0; i < normalView.count; i++) {
            let item = normalView.itemAtIndex(i);
            if (item) {
                Platform.WindowManager.setMinimizedGeometry(model[i], item);
            }
        }
    }

    DelegateModel {
        id: visualModel
        delegate: Package {
            Item {
                id: draggingLayout
                width: childrenRect.width
                height: childrenRect.height
                Package.name: "dragging"
                z: index
            }
            MouseArea {
                id: normalLayout
                width: childrenRect.width
                height: childrenRect.height
                Package.name: "normal"
                z: index

                acceptedButtons: Qt.NoButton
                hoverEnabled: true

                TaskItemData {
                    id: singleItemData
                    windowIdList: [modelData]
                    windowIcons:  taskItemData.WindowIcons
                    windowTitles: taskItemData.WindowTitles
                }

                onContainsMouseChanged: {
                    requestThumbnailView(containsMouse, singleItemData, this);
                }

                onClicked: {
                    taskItemClicked(modelData);
                }
            }
            LingmoUIItems.StyleBackground {
                id: visualDelegate
                width: viewLayout.width
                height: root.itemSize

                useStyleTransparency: false
                paletteRole: normalLayout.containsMouse ? Platform.Theme.BrightText : Platform.Theme.Base
                alpha: normalLayout.containsMouse ? 0.05 : 0.65
                border.width: 1
                borderColor: Platform.Theme.BrightText
                borderAlpha: 0.15
                radius: Platform.Theme.normalRadius
                opacity: !root.isDragged
                state: "normal"

                states: [
                    State {
                        name: "normal"
                        ParentChange { target: visualDelegate; parent: normalLayout; x: 0; y: 0 }
                        PropertyChanges { target: visualDelegate; width: viewLayout.width }
                    }
                ]

                Rectangle {
                    id: demandsAttention
                    anchors.fill: parent
                    color: "#FF9100"
                    radius: 8
                    visible: taskItemData.DemandsAttentionWinIdList.includes(modelData)

                    NumberAnimation on opacity {
                        running: demandsAttention.visible
                        loops: 4
                        from: 0.25
                        to: 0.6
                        duration: 450
                    }
                }

                RowLayout {
                    id: viewLayout
                    height: parent.height
                    spacing: 0
                    onChildrenRectChanged: width = childrenRect.width
                    z: 1

                    LingmoUIItems.Icon {
                        id: themeicon
                        Layout.margins: 4
                        Layout.preferredWidth: height
                        Layout.maximumWidth: height
                        Layout.fillHeight: true

                        source: taskItemData.WindowIcons[modelData]
                        Behavior on scale {
                            NumberAnimation { duration: 200 }
                        }
                    }

                    LingmoUIItems.StyleText {
                        id: text
                        Layout.preferredWidth: implicitWidth
                        Layout.maximumWidth: 192
                        Layout.fillHeight: true
                        Layout.rightMargin: 8
                        visible: orientation === ListView.Horizontal

                        text: (taskItemData.WindowTitles[modelData] === undefined) ? "" : taskItemData.WindowTitles[modelData]
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }

    ListView {
        id: normalView
        width: contentWidth
        height: contentHeight
        orientation: root.orientation
        model: visualModel.parts.normal

        interactive: false

        function normalViewItemIdx(x, y, offset, forward) {
            if (normalView.orientation === ListView.Horizontal) {
                return normalView.indexAt(forward ? x - offset : x + offset, y);
            } else {
                return normalView.indexAt(x, forward ? y - offset : y + offset);
            }
        }
    }

    Item {
        width: childrenRect.width
        height: childrenRect.height
        Repeater {
            model: visualModel.parts.dragging
        }
    }
}
