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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0

LingmoUIItems.StyleBackground {
    id: root
    width: childrenRect.width + 20
    height: childrenRect.height + 20

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 5

        CalendarHeader {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 368
            Layout.preferredHeight: 28
        }

        LingmoUIItems.StyleBackground {
            Layout.alignment: Qt.AlignCenter
            useStyleTransparency: false
            width: 368
            height: 1
            paletteRole: Theme.BrightText
            alpha: 0.1
        }

        CalendarCenterArea {
            id: centarArea
            Layout.alignment: Qt.AlignCenter
        }
    }
    function calendarStateChange(type) {
        if ((centarArea.stateType === type) && (centarArea.stateType != "normal")) {
            centarArea.stateType = "normal";
        } else {
            centarArea.stateType = type;
        }
    }
}
