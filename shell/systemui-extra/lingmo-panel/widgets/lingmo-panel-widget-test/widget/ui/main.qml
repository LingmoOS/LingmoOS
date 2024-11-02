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
import org.lingmo.menu.starter 1.0
import org.lingmo.quick.items 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal

    Layout.preferredWidth: isHorizontal ? 200 : 50
    Layout.preferredHeight: !isHorizontal ? 200 : 50

    // BlurImageTest {
    //     anchors.fill: parent
    // }
    ToolTip.text: Widget.tooltip

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            root.ToolTip.show(root.mapToGlobal(root.x, root.y));
        }

        onExited: {
            root.ToolTip.hide();
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#407D52BF"

        DropArea {
            anchors.fill: parent
            onDropped: (drop) => {
                console.log("drop:", drop.urls, drop.text);
            }
        }
    }
}
