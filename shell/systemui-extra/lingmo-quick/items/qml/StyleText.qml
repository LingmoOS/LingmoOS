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

Text {
    //颜色枚举值，从调色板中取得
    property int paletteRole: Theme.ButtonText;
    property int pointSizeOffset: 0
    property real alpha: 1.0;

    font {
        pointSize: {
            if ((Theme.fontSize + pointSizeOffset) < 0 ) {
                return Theme.fontSize;
            } else {
                return Theme.fontSize + pointSizeOffset;
            }
        }
        family: Theme.fontFamily
    }

    function updateColor() {
        color = Theme.colorWithCustomTransparency(paletteRole, Theme.Active, alpha);
    }

    onPaletteRoleChanged: {
        updateColor();
    }

    onAlphaChanged: {
        updateColor();
    }

    Theme.onPaletteChanged: updateColor();

    Component.onCompleted: {
        updateColor();
    }
}
