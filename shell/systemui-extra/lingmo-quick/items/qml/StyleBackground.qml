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

Rectangle {
    //是否跟随系统主题颜色的透明度
    property bool useStyleTransparency: true;
    //背景颜色枚举值，从调色板中取得
    property int paletteRole: Theme.Window;
    property int paletteGroup: Theme.Active;
    property real alpha: 1.0;
    property real borderAlpha: 1.0;
    property int borderColor: Theme.Base;

    border.width: 0;
    clip: true;

    function updateColor() {
        if (useStyleTransparency) {
            color = Theme.colorWithThemeTransparency(paletteRole, paletteGroup);
        } else {
            color = Theme.colorWithCustomTransparency(paletteRole, paletteGroup, alpha);
        }
    }

    function updateBorderColor() {
        border.color = Theme.colorWithCustomTransparency(borderColor, Theme.Active, borderAlpha);
    }

    //监听系统主题变化
    Theme.onPaletteChanged: {
        updateColor();
        updateBorderColor();
    }
    Theme.onThemeTransparencyChanged: updateColor();

    onPaletteGroupChanged: {
        updateColor();
    }

    onPaletteRoleChanged: {
        updateColor();
    }

    onAlphaChanged: {
        updateColor();
    }

    onBorderAlphaChanged: {
        updateBorderColor();
    }

    onBorderColorChanged: {
        updateBorderColor();
    }

    Component.onCompleted: {
        updateColor();
        updateBorderColor();
    }
}
