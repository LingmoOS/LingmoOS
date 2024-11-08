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
import QtQml 2.15
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

Rectangle {
    property real borderAlpha: 1.0
    property int  borderColor: Theme.Base

    // colorMixer
    property alias colorMixer: mixer
    property alias mixMode: mixer.mixMode

    // 背景色
    property alias backColorRole: mixer.backColorRole
    property alias backColorGroup: mixer.backColorGroup
    property alias backColorAlpha: mixer.backColorAlpha

    // 前景色
    property alias foreColorRole: mixer.foreColorRole
    property alias foreColorGroup: mixer.foreColorGroup
    property alias foreColorAlpha: mixer.foreColorAlpha

    color: mixer.color
    ColorMix {
        id: mixer
    }
    border.width: 0
    border.color: {
        return Theme.colorWithCustomTransparency(borderColor, Theme.Active, borderAlpha);
    }

    //监听系统主题变化
    Theme.onPaletteChanged: {
        borderColorChanged();
    }
}
