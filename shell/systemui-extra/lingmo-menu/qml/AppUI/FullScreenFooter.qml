/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5

import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.menu.core 1.0
import org.lingmo.menu.utils 1.0

Row {
    Layout.fillWidth: true
    Layout.preferredHeight: 48
    layoutDirection: Qt.RightToLeft

    LingmoUIItems.StyleBackground {
        width: 48; height: width
        useStyleTransparency: false
        paletteRole: Platform.Theme.Light
        alpha: powerButtonArea.containsPress ? 0.25 : powerButtonArea.containsMouse ? 0.12 : 0
        radius: height / 2
        borderColor: Platform.Theme.Highlight
        border.width: powerButtonArea.activeFocus ? 2 : 0

        PowerButton {
            id: powerButtonBase
        }

        LingmoUIItems.Icon {
            anchors.centerIn: parent
            width: Math.floor(parent.width / 2)
            height: width
            mode: LingmoUIItems.Icon.Highlight
            source: powerButtonBase.icon
        }

        MouseArea {
            id: powerButtonArea
            anchors.fill: parent
            hoverEnabled: true
            ToolTip.delay: 500
            ToolTip.visible: containsMouse
            ToolTip.text: powerButtonBase.toolTip
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            property int spacingFromMenu: 8
            activeFocusOnTab: true

            onClicked: {
                var buttonPosition = powerButtonArea.mapToGlobal(width, height);
                powerButtonBase.clicked(mouse.button === Qt.LeftButton, buttonPosition.x - width - spacingFromMenu, buttonPosition.y + spacingFromMenu, mainWindow.isFullScreen);
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    powerButtonBase.clicked(true, 0, 0, mainWindow.isFullScreen);
                }
            }
        }
    }
}
