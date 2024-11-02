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
import QtQuick.Controls 2.12
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

LingmoUIItems.StyleBackground {
    id: root
    property alias count: actionsRepeater.count
    property alias actions: actionsRepeater.model
    property alias spacing: mainLayout.spacing
    property int maring: 2

    width: count > 0 ? (childrenRect.width + maring*2) : 0

    radius: height/2
    alpha: 0.03
    useStyleTransparency: false
    paletteRole: Platform.Theme.WindowText

    border.width: 1
    borderAlpha: 0.06
    borderColor: Platform.Theme.WindowText

    Row {
        id: mainLayout
        width: childrenRect.width
        height: parent.height - maring*2
        x: root.maring; y: root.maring
        spacing: 2

        Repeater {
            id: actionsRepeater
            delegate: Component {
                LingmoUIItems.Button {
                    width: height
                    height: parent.height

                    LingmoUIItems.Tooltip {
                        anchors.fill: parent
                        mainText: modelData.toolTip
                        posFollowCursor: true
                        margin: 6
                        visible: modelData.toolTip !== ""
                    }

                    background.radius: width / 2
                    background.paletteRole: Platform.Theme.Highlight
                    background.alpha: modelData.checked ? 1 : containsMouse ? 0.3 : 0

                    icon.source: modelData.icon
                    icon.mode: modelData.checked ? LingmoUIItems.Icon.Highlight : LingmoUIItems.Icon.AutoHighlight

                    onClicked: {
                        // 执行action
                        modelData.trigger();
                    }
                    Keys.onPressed: {
                        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            modelData.trigger();
                        }
                    }
                }
            }
        }
    }
}
