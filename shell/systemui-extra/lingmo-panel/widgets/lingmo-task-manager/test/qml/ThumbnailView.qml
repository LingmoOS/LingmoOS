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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *          qiqi49 <qiqi@kylinos.cn>
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.lingmo.windowThumbnail 1.0
import org.lingmo.quick.items 1.0

MouseArea {
    width: childrenRect.width
    height: childrenRect.height
    hoverEnabled: true

    onEntered: {
        thumbnailView.viewShow = true;
    }

    onExited: {
        thumbnailView.viewShow = false;
    }

    ListView {
        width: childrenRect.width
        height: 200
        interactive: false
        orientation: ListView.Horizontal
        spacing: 20

        onChildrenRectChanged: {
            thumbnailView.width = childrenRect.width
        }

        model: thumbnailView.viewModel
        delegate: MouseArea {
            width: 300
            height: 200
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: {
                if (mouse.button === Qt.RightButton) {
                    thumbnailView.openMenu(modelData);
                } else {
                    taskModel.activateWindow(modelData);
                }
            }

            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 32
                    Layout.maximumHeight: 32
                    visible: windowThumbnail.thumbnailMprisModel.count <= 0

                    ThemeIcon {
                        Layout.preferredHeight: 32
                        Layout.preferredWidth: 32
                        source: taskModel.windowIcon(modelData)
                    }
                    Text {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        text: taskModel.windowTitle(modelData)
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                WindowThumbnail {
                    id: windowThumbnail
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    winId: Number.parseInt(modelData)
                }
            }
        }
    }
}
