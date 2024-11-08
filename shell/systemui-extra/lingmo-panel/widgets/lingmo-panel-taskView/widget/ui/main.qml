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
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.lingmo.quick.widgets 1.0
import org.lingmo.panel.taskView 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? childrenRect.width : width
    Layout.preferredHeight: isHorizontal ? height : childrenRect.height

    StyleBackground {
        id: backGround
        anchors.centerIn: parent
        width: root.isHorizontal? icon.width  + 8 : parent.width
        height: root.isHorizontal? parent.height : icon.height + 8
        radius: 8
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: mouseArea.containsPress ? 0.10 : mouseArea.containsMouse ? 0.05 : 0
        Icon {
            id: icon
            anchors.centerIn: parent
            width: root.isHorizontal? height : parent.width - 8
            height: root.isHorizontal? parent.height - 8 : width
            source: "lingmo-taskview-black-symbolic"
            mode: Icon.AutoHighlight
            scale: mouseArea.containsPress ? 0.9 : 1.0
        }
    }
    MouseArea {
        id: mouseArea
        anchors.fill: backGround
        hoverEnabled: true

        // TaskViewButton {
        //     id: taskviewButton
        // }

        onClicked: {
            // taskviewButton.showTaskView();
            AppLauncher.runCommand("lingmo-window-switch --show-workspace");
        }

        Tooltip {
            id: tooltip
            anchors.fill: parent
            mainText: Widget.tooltip
            posFollowCursor: false
            location: {
                switch(Widget.container.position) {
                    case Types.Bottom:
                        return Dialog.BottomEdge;
                    case Types.Left:
                        return Dialog.LeftEdge;
                    case Types.Top:
                        return Dialog.TopEdge;
                    case Types.Right:
                        return Dialog.RightEdge;
                }
            }
            margin: 6
        }
    }
}
