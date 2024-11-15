/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.Screen 1.0 as CS
import "../"

IconCheckBox {
    id: control

    property int value
    property var rot

    checked: rotationLayout.current_rot === rot

    onClicked: {
        if (element.rotation === rot) {
            return;
        }

        element.rotation = rot;
        rotationLayout.current_rot = rot;
        // 下面不需要了
        // screen.resetTotalSize()
        // screen.save()
    }

    Component.onCompleted: {
        switch(value) {
        case 90:
            rot = CS.Output.Left
            control.source = "qrc:/images/rot90.svg";
            break;
        case 180:
            control.source = "qrc:/images/rot180.svg";
            rot = CS.Output.Inverted;
            break;
        case 270:
            control.source = "qrc:/images/rot270.svg";
            rot = CS.Output.Right;
            break;
        case 0:
        default:
            control.source = "qrc:/images/rotnormal.svg";
            rot = CS.Output.None;
        }
    }
}
