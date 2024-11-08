/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

MouseArea {
    readonly property alias icon: iconBase
    readonly property alias background: backgroundBase

    hoverEnabled: true
    activeFocusOnTab: true

    StyleBackground {
        id: backgroundBase
        anchors.fill: parent

        border.width: parent.activeFocus ? 2 : 0
        paletteRole: Theme.Button
        borderColor: Theme.Highlight
        useStyleTransparency: false
        alpha: parent.containsPress ? 0.30 : parent.containsMouse ? 0.15 : 0.0

        ThemeIcon {
            id: iconBase
            anchors.centerIn: parent
            width: parent.width / 2
            height: parent.height / 2
        }
    }
}
