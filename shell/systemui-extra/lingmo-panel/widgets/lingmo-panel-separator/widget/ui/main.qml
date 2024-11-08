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

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.lingmo.quick.widgets 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? 1 : width
    Layout.preferredHeight: isHorizontal ? height : 1

    StyleBackground {
        useStyleTransparency: false
        alpha: 0.3
        paletteRole: Theme.ButtonText

        anchors.centerIn: parent
        // anchors.fill: parent

        width: root.isHorizontal ? parent.width : parent.width * 0.5
        height: root.isHorizontal ? parent.height * 0.5 : parent.height

        // anchors.leftMargin: root.isHorizontal ? 0 : 8
        // anchors.rightMargin: root.isHorizontal ? 0 : 8
        //
        // anchors.topMargin: root.isHorizontal ? 8 : 0
        // anchors.bottomMargin: root.isHorizontal ? 8 : 0
    }
}
