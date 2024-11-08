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

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.lingmo.quick.widgets 1.0
import org.lingmo.panel.showDesktop 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredHeight: isHorizontal ? height : 15
    Layout.preferredWidth: isHorizontal ? 15 : width

    StyleBackground {
        anchors.left: parent.left
        anchors.top: parent.top
        width: isHorizontal ? 1 : parent.width
        height: isHorizontal ? parent.height : 1

        alpha: 0.3
        paletteRole: Theme.Light
        useStyleTransparency: false
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        StyleBackground {
            anchors.fill: parent
            useStyleTransparency: false
            paletteRole: Theme.BrightText
            alpha: parent.containsPress ? 0.15 : parent.containsMouse ? 0.1 : 0.05
        }

        ShowDesktop {
            id: showDesktop
        }

        onClicked: {
            showDesktop.showDesktop();
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
